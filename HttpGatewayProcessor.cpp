#include "os-dependencies.h"

#include "HttpGatewayProcessor.h"

#include <spdlog/spdlog.h>
#include <cpprest/http_client.h>

#include "RequestPublisher.h"
#include <nlohmann/json.hpp>

using namespace BloombergLP;


HttpGatewayProcessor::HttpGatewayProcessor()
    : requestPublisher_(std::make_unique<RequestPublisher>("localhost", "/", "guest", "guest"))
{
}

HttpGatewayProcessor::~HttpGatewayProcessor() = default;

void HttpGatewayProcessor::EnqueueRequest(bool postOrPut, const std::chrono::system_clock::time_point & time, const std::string & urlSuffix,
                                          const std::string & payload, const std::unordered_map<std::string, std::string> & header, const std::string & hint)
{
    const nlohmann::json jsonPayload = nlohmann::json::parse(payload);
    requestPublisher_->Publish(jsonPayload, postOrPut ? "POST" : "PUT", urlSuffix);
}
