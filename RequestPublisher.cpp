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

#include <future>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using namespace BloombergLP;


RequestPublisher::RequestPublisher(const std::string& host, const std::string& vhost, const std::string& user,
                                   const std::string& pass)
{
    const auto vhostSharedPtr = context_.createVHostConnection("metrics-publisher",
                                                               bsl::make_shared<
                                                                   rmqt::SimpleEndpoint>(host, vhost, 5672),
                                                               bsl::make_shared<rmqt::PlainCredentials>(user, pass));
    if (!vhostSharedPtr)
    {
        throw std::runtime_error("VHost connection failed");
    }
    vhostSharedPtr_ = vhostSharedPtr;

    rmqa::Topology topology;
    const auto exchange = topology.addExchange("sdr_exchange");
    const auto queue = topology.addQueue("sdr_queue");
    topology.bind(exchange, queue, "sdr_bind");

    constexpr unsigned short maxUnconfirmed = 10;
    const auto prodRes = vhostSharedPtr_->createProducer(topology, exchange, maxUnconfirmed);
    if (!prodRes)
    {
        const auto errorString = fmt::format(
            "Producer creation failed: {}. Host: {}, VHost: {}, User: {}",
            prodRes.error(), host, vhost, user);
        spdlog::error(errorString);
        throw std::runtime_error(errorString);
    }
    producer_ = prodRes.value();
}

void RequestPublisher::Publish(const nlohmann::json& payload, const std::string& httpRequest,
                               const std::string& urlSuffix) const
{
    // Prepare message
    nlohmann::json payloadExtended(payload);
    payloadExtended["httpRequest"] = httpRequest;
    payloadExtended["urlSuffix"] = urlSuffix;

    const std::string msgStr = payloadExtended.dump();
    const auto vecPtr = bsl::make_shared<bsl::vector<uint8_t>>(msgStr.begin(), msgStr.end());
    const rmqt::Message message(vecPtr);

    const rmqp::Producer::SendStatus sendResult =
        producer_->send(
            message,
            "metrics-capture",
            [msgStr](const rmqt::Message&,
                     const bsl::string& routingKey,
                     const rmqt::ConfirmResponse& confirm)
            {
                if (confirm.status() == rmqt::ConfirmResponse::Status::ACK)
                {
                    spdlog::info("Message ACKed ({}): {}", routingKey, msgStr);
                }
                else
                {
                    //REJECT / RETURN indicate problem with the send request. Bad routing key?
                    spdlog::error("Message NOT ACKed ({}): {}", routingKey, msgStr);
                }
            }
        );
    if (sendResult != rmqp::Producer::SENDING)
    {
        spdlog::error("Unable to enqueue message {}.", msgStr);
    }
    else
    {
        spdlog::debug("Message enqueued: {}.", msgStr);
    }
}

//TODO: why these 3 are unresolved and I need this ugly code?
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


