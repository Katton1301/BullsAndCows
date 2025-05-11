#pragma once
#include <thread>
#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <events/event_manager.hpp>

class TEventProcessor
{
public:
    TEventProcessor() = delete;
    TEventProcessor(TEventManager& manager, RdKafka::Producer* kafka_producer, std::string const & producer_topic);

    void start(int thread_count = 1);
    void stop();

private:
    void processEvents();
    SERVER_COMPONENTS::TServerState ServerState() const;
    uint32_t ServerId() const;
    SERVER_COMPONENTS::TResultData registerServer(SERVER_COMPONENTS::TRequestData const& request);
    SERVER_COMPONENTS::TResultData handleRequest(SERVER_COMPONENTS::TRequestData const& request);
    SERVER_COMPONENTS::TResultData handleServerCommand(SERVER_COMPONENTS::TRequestData const& request);
    SERVER_COMPONENTS::TResultData handleGameCommand(SERVER_COMPONENTS::TRequestData const& request);

    void sendToKafka(const std::string& message);

    TEventManager& m_manager;
    RdKafka::Producer* m_kafka_producer;
    std::string m_producer_topic;
    std::vector<std::thread> m_workers;
    SERVER_COMPONENTS::TServerState m_serverState;
    uint32_t m_serverId = 0;
    std::atomic<bool> m_running;
};
