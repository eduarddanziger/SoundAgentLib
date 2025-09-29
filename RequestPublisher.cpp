#include "os-dependencies.h"

#include "RequestPublisher.h"
#include "Contracts.h"

#include <rmqa_topology.h>
#include <rmqa_producer.h>

#include <rmqt_message.h>
#include <rmqt_simpleendpoint.h>
#include <rmqt_plaincredentials.h>

#include <bsl_string.h>
#include <stdexcept>

#include <future>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <thread>
#include <chrono>
#include <algorithm>


using namespace BloombergLP;


RequestPublisher::RequestPublisher(const std::string& host, const std::string& vhost, const std::string& user,
    const std::string& pass) :
    contextOptionsSmartPtr_(bsl::make_shared<rmqa::RabbitContextOptions>())
{
    contextOptionsSmartPtr_->setConnectionErrorThreshold(
        bsls::TimeInterval(20, 0)) // 20 seconds
        .setErrorCallback([](const bsl::string& errorText, int errorCode)
            {
                spdlog::error("RabbitMQ error (code {}): {}", errorCode, errorText);
            });

    // Retry parameters
    constexpr int maxAttempts = 12;
    std::chrono::milliseconds delay(2000);
    constexpr std::chrono::milliseconds maxDelay(30000);

    for (int attempt = 1; attempt <= maxAttempts; ++attempt)
    {
        try
        {
            contextSmartPtr_ = bsl::make_shared<rmqa::RabbitContext>(*contextOptionsSmartPtr_);

            const auto vhostSharedPtr = contextSmartPtr_->createVHostConnection(
                "sdr-publisher",
                bsl::make_shared<rmqt::SimpleEndpoint>(host, vhost, 5672),
                bsl::make_shared<rmqt::PlainCredentials>(user, pass)
            );
            if (!vhostSharedPtr)
            {
                throw std::runtime_error("VHost connection failed");
            }

            rmqa::Topology topology;
            const auto exchange = topology.addExchange(RQM_EXCHANGE_NAME);
            const auto queue = topology.addQueue(RQM_QUEUE_NAME);
            topology.bind(exchange, queue, RQM_ROUTING_KEY);

            constexpr unsigned short maxUnconfirmed = 10;
            const auto prodRes = vhostSharedPtr->createProducer(topology, exchange, maxUnconfirmed);
            if (!prodRes)
            {
                const auto errorString = fmt::format(
                    "Producer creation failed: {}. Host: {}, VHost: {}, User: {}",
                    prodRes.error(), host, vhost, user);
                spdlog::error(errorString);
                throw std::runtime_error(errorString);
            }

            producer_ = prodRes.value();
            vhostSharedPtr_ = vhostSharedPtr;

            spdlog::info("RabbitMQ producer initialized on attempt {}.", attempt);
            break;
        }
        catch (const std::exception& ex)
        {
            if (attempt == maxAttempts)
            {
                spdlog::error("RabbitMQ initialization failed after {} attempts: {}", maxAttempts, ex.what());
                throw;
            }

            spdlog::warn(
                "RabbitMQ init attempt {}/{} failed: {}. Retrying in {} ms...",
                attempt, maxAttempts, ex.what(), delay.count());

            std::this_thread::sleep_for(delay);
            
            delay = std::min(delay * 2, maxDelay); // Exponential
        }
    }
}

void RequestPublisher::Publish(const nlohmann::json& payload, const std::string& httpRequest,
                               const std::string& urlSuffix) const
{
    // Prepare message
    nlohmann::json payloadExtended(payload);
    payloadExtended[std::string(contracts::message_fields::HTTP_REQUEST)] = httpRequest;
    payloadExtended[std::string(contracts::message_fields::URL_SUFFIX)] = urlSuffix;

    const std::string msgStr = payloadExtended.dump();
    const auto vecPtr = bsl::make_shared<bsl::vector<uint8_t>>(msgStr.begin(), msgStr.end());
    const rmqt::Message message(vecPtr);

    const rmqp::Producer::SendStatus sendResult =
        producer_->send(
            message,
            RQM_ROUTING_KEY,
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


