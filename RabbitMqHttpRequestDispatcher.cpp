#include "os-dependencies.h"

#include "RabbitMqHttpRequestDispatcher.h"

#include <spdlog/spdlog.h>
#include <cpprest/http_client.h>

#include "RequestPublisher.h"
#include <nlohmann/json.hpp>

using namespace BloombergLP;


RabbitMqHttpRequestDispatcher::RabbitMqHttpRequestDispatcher()
    : requestPublisher_(std::make_unique<RequestPublisher>("localhost", "/", "guest", "guest"))
{
}

RabbitMqHttpRequestDispatcher::~RabbitMqHttpRequestDispatcher() = default;

void RabbitMqHttpRequestDispatcher::EnqueueRequest(bool postOrPut, const std::chrono::system_clock::time_point & time, const std::string & urlSuffix,
                                          const std::string & payload, const std::unordered_map<std::string, std::string> & header, const std::string & hint)
{
    spdlog::info("Publishing to the RabbitMQ queue: {}...", hint);
    const nlohmann::json jsonPayload = nlohmann::json::parse(payload);
    requestPublisher_->Publish(jsonPayload, postOrPut ? "POST" : "PUT", urlSuffix);
}
