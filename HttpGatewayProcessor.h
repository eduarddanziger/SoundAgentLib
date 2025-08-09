#pragma once

#include "common/ClassDefHelper.h"

#include "HttpRequestProcessorInterface.h"

class RequestPublisher;

class HttpGatewayProcessor final : public HttpRequestProcessorInterface
{
public:
    HttpGatewayProcessor();

    DISALLOW_COPY_MOVE(HttpGatewayProcessor);

    ~HttpGatewayProcessor() override;

    void EnqueueRequest(
        bool postOrPut,
        const std::chrono::system_clock::time_point& time,
        const std::string& urlSuffix, const std::string& payload,
        const std::unordered_map<std::string, std::string>& header,
        const std::string& hint
    ) override;

private:
    std::unique_ptr<RequestPublisher> requestPublisher_;
};
