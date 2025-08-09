#pragma once

#include <rmqa_rabbitcontext.h>
#include <rmqa_vhost.h>
#include <nlohmann/json_fwd.hpp>

class RequestPublisher
{
public:
    RequestPublisher(const std::string& host,
                     const std::string& vhost,
                     const std::string& user,
                     const std::string& pass);

    void Publish(const nlohmann::json& payload, const std::string& httpRequest, const std::string& urlSuffix) const;

private:
    BloombergLP::rmqa::RabbitContext context_;
    bsl::shared_ptr<BloombergLP::rmqa::VHost> vhostSharedPtr_;
};
