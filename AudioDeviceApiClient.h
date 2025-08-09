#pragma once

#include <functional>
#include <memory>

#include "public/SoundAgentInterface.h"


class HttpRequestProcessorInterface;
class HttpStandaloneProcessor;
class SoundDeviceInterface;


class AudioDeviceApiClient {
public:
    AudioDeviceApiClient(HttpRequestProcessorInterface &processor,
                         std::function<std::string()> getHostNameCallback,
                         std::function<std::string()> getOperationSystemNameCallback
    );

    void PostDeviceToApi(SoundDeviceEventType eventType, const SoundDeviceInterface* device,
                         const std::string& hintPrefix) const;
    void PutVolumeChangeToApi(const std::string& pnpId, bool renderOrCapture, uint16_t volume,
                              const std::string& hintPrefix) const;

private:
    HttpRequestProcessorInterface& requestProcessor_;
    std::function<std::string()> getHostNameCallback_;
    std::function<std::string()> getOperationSystemNameCallback_;
};
