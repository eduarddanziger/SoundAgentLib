#pragma once

#include <nlohmann/json.hpp>

#include <rmqa_rabbitcontext.h>
#include <rmqa_vhost.h>
#include <rmqa_topology.h>
#include <rmqa_producer.h>

#include <rmqt_message.h>
#include <rmqt_simpleendpoint.h>
#include <rmqt_plaincredentials.h>
#include <rmqt_vhostinfo.h>

#include <bsl_memory.h>
#include <bsl_string.h>
#include <iostream>
#include <stdexcept>

using Json = nlohmann::json;

using namespace BloombergLP;

class MetricsPublisher
{
public:
    MetricsPublisher(const std::string& host,
                     const std::string& vhost,
                     const std::string& user,
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

    void Publish(const nlohmann::json& payload, const std::string& httpRequest, const std::string& urlSuffix) const
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


        nlohmann::json payloadExtended(payload);
        payloadExtended["httpRequest"] = httpRequest;
        payloadExtended["urlSuffix"] = urlSuffix;

        std::string msgStr = payloadExtended.dump();
        auto vecPtr = bsl::make_shared<bsl::vector<uint8_t>>(msgStr.begin(), msgStr.end());
        rmqt::Message message(vecPtr, "", {});

        producer->send(
            message,
            "metrics-capture",
            [msgStr](const rmqt::Message&,
                     const bsl::string& routingKey,
                     const rmqt::ConfirmResponse& confirm)
            {
                if (confirm.status() != rmqt::ConfirmResponse::Status::ACK)
                {
                    std::cerr << "Message NACKed (" << routingKey << "): " << msgStr << "\n";
                }
                else
                {
                    std::cout << "Message ACKed (" << routingKey << "): " << msgStr << "\n";
                }
            }
        );
    }

private:
    rmqa::RabbitContext context_;
    bsl::shared_ptr<rmqa::VHost> vhostSharedPtr_;
};
