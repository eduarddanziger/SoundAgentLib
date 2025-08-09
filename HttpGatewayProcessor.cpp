#include "os-dependencies.h"

#include "HttpGatewayProcessor.h"

#include <spdlog/spdlog.h>
#include <cpprest/http_client.h>

#include "MetricsPublisher.h"
#include <nlohmann/json.hpp>


using namespace BloombergLP;


HttpGatewayProcessor::HttpGatewayProcessor()
    : metricsPublisher_(std::make_unique<MetricsPublisher>("localhost", "/", "guest", "guest"))
{
}

HttpGatewayProcessor::~HttpGatewayProcessor() = default;

void HttpGatewayProcessor::EnqueueRequest(bool postOrPut, const std::chrono::system_clock::time_point & time, const std::string & urlSuffix,
                                          const std::string & payload, const std::unordered_map<std::string, std::string> & header, const std::string & hint)
{
    nlohmann::json jsonPayload = nlohmann::json::parse(payload);
    metricsPublisher_->Publish(jsonPayload, postOrPut ? "POST" : "PUT", urlSuffix);
}
