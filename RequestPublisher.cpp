#include "os-dependencies.h"

#define USES_LIBRMQ_EXPERIMENTAL_FEATURES

#include "RequestPublisher.h"

#include "Contracts.h"

#include <rmqa_topology.h>
#include <rmqa_producer.h>

#include <rmqt_future.h>
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
                               bsls::TimeInterval(CONNECTION_THRESHOLD_IN_SECONDS, 0)) // 20 seconds
                           .setErrorCallback([this](const bsl::string& errorText, int errorCode)
                           {
                               HandleConnectionError(errorText, errorCode);
                           });

    std::chrono::milliseconds delay(DELAY_BETWEEN_RECONNECTION_ATTEMPTS_IN_MILLISECONDS);

    for (int attempt = 1; attempt <= MAX_RECONNECTION_ATTEMPTS; ++attempt)
    {
        try
        {
            contextSmartPtr_ = bsl::make_shared<rmqa::RabbitContext>(*contextOptionsSmartPtr_);

            vHostSmartPtr_ = contextSmartPtr_->createVHostConnection(
                "sdr-publisher",
                bsl::make_shared<rmqt::SimpleEndpoint>(host, vhost, 5672),
                bsl::make_shared<rmqt::PlainCredentials>(user, pass)
            );
            if (!vHostSmartPtr_)
            {
                throw std::runtime_error("VHost connection failed");
            }

            rmqa::Topology topology;
            const auto exchange = topology.addExchange(RQM_EXCHANGE_NAME);
            const auto queue = topology.addQueue(RQM_QUEUE_NAME);
            topology.bind(exchange, queue, RQM_ROUTING_KEY);

            constexpr unsigned short maxUnconfirmed = 10;
            spdlog::info("Initializing the RabbitMQ producer on attempt {}/{}.", attempt, MAX_RECONNECTION_ATTEMPTS);
            auto prodFuture = vHostSmartPtr_->createProducerAsync(topology, exchange, maxUnconfirmed);

            // Wait with a timeout so we can break out if host is unreachable
            spdlog::info("Waiting for RabbitMQ producer (up to {} seconds)...", CONNECTION_THRESHOLD_IN_SECONDS + 5);
            const auto prodRes = prodFuture.waitResult(
                bsls::TimeInterval(CONNECTION_THRESHOLD_IN_SECONDS + 5, 0));
            if (!prodRes)
            {
                const auto errorString = fmt::format(
                    "Producer creation failed: {}. Host: {}, VHost: {}, User: {}",
                    prodRes.error(), host, vhost, user);
                spdlog::error(errorString);
                throw std::runtime_error(errorString);
            }

            producer_ = prodRes.value();

            spdlog::info("RabbitMQ producer initialized on attempt {}.", attempt);
            break;
        }
        catch (const std::exception& ex)
        {
            if (attempt == MAX_RECONNECTION_ATTEMPTS)
            {
                spdlog::error("RabbitMQ initialization failed after {} attempts: {}", MAX_RECONNECTION_ATTEMPTS, ex.what());
                throw;
            }

            spdlog::warn(
                "RabbitMQ init attempt {}/{} failed: {}. Retrying in {} ms...",
                attempt, MAX_RECONNECTION_ATTEMPTS, ex.what(), delay.count());

            std::this_thread::sleep_for(delay);
            
            delay = std::min(delay * 2, std::chrono::milliseconds(MAX_DELAY_BETWEEN_RECONNECTION_ATTEMPTS_IN_MILLISECONDS)); // Exponential
        }
    }
}

void RequestPublisher::HandleConnectionError(const bsl::string& errorText, int errorCode)
{
    spdlog::error("RabbitMQ error {}: {}",
                  errorCode,
                  errorText);

    connectionErrorCV_.notify_all();
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

const char* BloombergLP::BSLS_LIBRARYFEATURES_LINKER_CHECK_NAME = "";


