#pragma once
#include <thread>
#include <iostream>
#if defined(KAFKA_SERVER)
    #include <librdkafka/rdkafkacpp.h>
#endif
#include <events/event_manager.hpp>

class TEventProcessor
{
public:
    TEventProcessor() = delete;
    TEventProcessor(
        TEventManager& manager
#if defined(KAFKA_SERVER)
        ,RdKafka::Producer* kafka_producer, std::string const & producer_topic
#endif
    );

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

    TEventManager& m_manager;

#if defined(KAFKA_SERVER)
    void sendToKafka(const std::string& message);
    RdKafka::Producer* m_kafka_producer;
    std::string m_producer_topic;
#endif

    std::vector<std::thread> m_workers;
    SERVER_COMPONENTS::TServerState m_serverState;
    uint32_t m_serverId = 0;
    std::atomic<bool> m_running;
};
