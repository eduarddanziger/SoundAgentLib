#pragma once

#include <rmqa_rabbitcontext.h>
#include <nlohmann/json_fwd.hpp>

class RequestPublisher
{
public:
    RequestPublisher(
        const std::string& host,
        const std::string& vhost,
        const std::string& user,
        const std::string& pass);

    void Publish(
        const nlohmann::json& payload,
        const std::string& httpRequest,
        const std::string& urlSuffix) const;

private:
    static constexpr auto RQM_EXCHANGE_NAME = "sdr_exchange";
    static constexpr auto RQM_QUEUE_NAME = "sdr_queue";
    static constexpr auto RQM_ROUTING_KEY = "sdr_bind";

    static constexpr int CONNECTION_THRESHOLD_IN_SECONDS = 20;
    static constexpr int MAX_RECONNECTION_ATTEMPTS = 8;
    static constexpr int DELAY_BETWEEN_RECONNECTION_ATTEMPTS_IN_MILLISECONDS = 2000;
    static constexpr int MAX_DELAY_BETWEEN_RECONNECTION_ATTEMPTS_IN_MILLISECONDS = 30000;


    bsl::shared_ptr<BloombergLP::rmqa::RabbitContextOptions> contextOptionsSmartPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::RabbitContext> contextSmartPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::VHost> vhostSharedPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::Producer> producer_;
};
