#include "os-dependencies.h"

#include "AudioDeviceApiClient.h"
#include "MetricsPublisher.h"

#include "public/SoundAgentInterface.h"

#include "common/TimeUtil.h"

#include <spdlog/spdlog.h>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>

//TODO: why these 3 are unresolved?

rmqt::SimpleEndpoint::SimpleEndpoint(bsl::string_view address,
                                     bsl::string_view vhost,
                                     bsl::uint16_t port)
    : d_address(address)
    , d_vhost(vhost)
    , d_port(port)
{
}

rmqt::PlainCredentials::PlainCredentials(bsl::string_view username,
                                         bsl::string_view password)
    : d_username(username)
    , d_password(password)
{
}

auto BloombergLP::BSLS_LIBRARYFEATURES_LINKER_CHECK_NAME = "";




// ReSharper disable CppPassValueParameterByConstReference
AudioDeviceApiClient::AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor,
                                           std::function<std::string()> getHostNameCallback,
                                           std::function<std::string()> getOperationSystemNameCallback
)
    : requestProcessor_(processor)  // NOLINT(performance-unnecessary-value-param)
    , getHostNameCallback_(std::move(getHostNameCallback))
    , getOperationSystemNameCallback_(std::move(getOperationSystemNameCallback))
{
}
// ReSharper restore CppPassValueParameterByConstReference

void AudioDeviceApiClient::PostDeviceToApi(SoundDeviceEventType eventType, const SoundDeviceInterface* device, const std::string& hintPrefix) const
{
    if (!device)
    {
        spdlog::error("Cannot post device data: nullptr provided");
        return;
    }

    const std::string hostName = getHostNameCallback_();
    const std::string operationSystemName = getOperationSystemNameCallback_();

    const auto nowTime = std::chrono::system_clock::now();
    const auto timeAsUtcString = ed::TimePointToStringAsUtc(
        nowTime,
        true, // insertTBetweenDateAndTime
        true // addTimeZone
    );

    const nlohmann::json payload = {
        {"pnpId", device->GetPnpId()},
        {"hostName", hostName},
        {"name", device->GetName()},
        {"operationSystemName", operationSystemName},
        {"flowType", device->GetFlow()},
        {"renderVolume", device->GetCurrentRenderVolume()},
        {"captureVolume", device->GetCurrentCaptureVolume()},
        {"updateDate", timeAsUtcString},
        {"deviceMessageType", eventType}
    };

    // Convert nlohmann::json to string and to value
    const std::string payloadString = payload.dump();
    const auto hint = hintPrefix + "Post a device." + device->GetPnpId();

    spdlog::info("Enqueueing: {}...", hint);

    const MetricsPublisher publisher("localhost", "/", "guest", "guest");
    publisher.Publish(payload, "POST", "");
}

void AudioDeviceApiClient::PutVolumeChangeToApi(const std::string & pnpId, bool renderOrCapture, uint16_t volume, const std::string& hintPrefix) const
{
    const auto nowTime = std::chrono::system_clock::now();
    const auto timeAsUtcString = ed::TimePointToStringAsUtc(
        nowTime,
        true, // insertTBetweenDateAndTime
        true // addTimeZone
    );

    const nlohmann::json payload = {
        {"deviceMessageType", renderOrCapture ? SoundDeviceEventType::VolumeRenderChanged : SoundDeviceEventType::VolumeCaptureChanged},
        {"volume", volume},
        {"updateDate", timeAsUtcString }
    };
    const std::string payloadString = payload.dump();

    const auto hint = hintPrefix + "Volume change (PUT) for a device: " + pnpId;
    spdlog::info("Enqueueing: {}...", hint);
    // Instead of sending directly, enqueue the request in the processor

    const auto urlSuffix = std::format("/{}/{}", pnpId, getHostNameCallback_());

    const MetricsPublisher publisher("localhost", "/", "guest", "guest");
    publisher.Publish(payload, "PUT", urlSuffix);
}

