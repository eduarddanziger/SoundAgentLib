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
    bsl::shared_ptr<BloombergLP::rmqa::RabbitContextOptions> contextOptionsSmartPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::RabbitContext> contextSmartPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::VHost> vhostSharedPtr_;
    bsl::shared_ptr<BloombergLP::rmqa::Producer> producer_;
};
