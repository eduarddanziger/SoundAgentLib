#include "os-dependencies.h"

#include "RequestPublisher.h"

#include <rmqa_topology.h>
#include <rmqa_producer.h>

#include <rmqt_message.h>
#include <rmqt_simpleendpoint.h>
#include <rmqt_plaincredentials.h>
#include <rmqt_vhostinfo.h>

#include <bsl_string.h>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using namespace BloombergLP;


RequestPublisher::RequestPublisher(const std::string& host, const std::string& vhost, const std::string& user,
    const std::string& pass)
{
    const bsl::shared_ptr<rmqt::Endpoint> endpoint =
        bsl::make_shared<rmqt::SimpleEndpoint>(host, vhost);

    const bsl::shared_ptr<rmqt::Credentials> credentials =
        bsl::make_shared<rmqt::PlainCredentials>(user, pass);

    const rmqt::VHostInfo vhostInfo(endpoint, credentials);

    const auto vhostSharedPtr = context_.createVHostConnection("metrics-publisher", vhostInfo);
    if (!vhostSharedPtr)
    {
        throw std::runtime_error("VHost connection failed");
    }
    vhostSharedPtr_ = vhostSharedPtr;
}

void RequestPublisher::Publish(const nlohmann::json& payload, const std::string& httpRequest,
    const std::string& urlSuffix) const
{
    // Build topology
    rmqa::Topology topology;
    auto exchange = topology.addExchange("sdr_updates");
    auto queue = topology.addQueue("sdr_metrics");
    topology.bind(exchange, queue, "metrics-capture");

    constexpr unsigned short maxUnconfirmed = 10;
    const auto prodRes = vhostSharedPtr_->createProducer(topology, exchange, maxUnconfirmed);
    if (!prodRes)
    {
        throw std::runtime_error("Producer creation failed: " + prodRes.error());
    }
    const bsl::shared_ptr<rmqa::Producer>& producer = prodRes.value();

    // Prepare message
    nlohmann::json payloadExtended(payload);
    payloadExtended["httpRequest"] = httpRequest;
    payloadExtended["urlSuffix"] = urlSuffix;

    std::string msgStr = payloadExtended.dump();
    auto vecPtr = bsl::make_shared<bsl::vector<uint8_t>>(msgStr.begin(), msgStr.end());
    rmqt::Message message(vecPtr, "", {});

    // Send message
    producer->send(
        message,
        "metrics-capture",
        [msgStr](const rmqt::Message&,
                 const bsl::string& routingKey,
                 const rmqt::ConfirmResponse& confirm)
        {
            if (confirm.status() != rmqt::ConfirmResponse::Status::ACK)
            {
                spdlog::error("Message NACKed ({}): {}", routingKey, msgStr);
            }
            else
            {
                spdlog::info("Message ACKed ({}): {}", routingKey, msgStr);
            }
        }
    );
}

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


