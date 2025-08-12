#pragma once

#include "common/ClassDefHelper.h"

#include "HttpRequestDispatcherInterface.h"

class RequestPublisher;

class RabbitMqHttpRequestDispatcher final : public HttpRequestDispatcherInterface
{
public:
    RabbitMqHttpRequestDispatcher();

    DISALLOW_COPY_MOVE(RabbitMqHttpRequestDispatcher);

    ~RabbitMqHttpRequestDispatcher() override;

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
