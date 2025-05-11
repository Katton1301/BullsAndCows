#pragma once
#include <events/event_processor.hpp>
#include <librdkafka/rdkafkacpp.h>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

class TServer
{
public:
    enum
    {
        MAX_THREADS = 4
    };
    
    TServer(TEventManager& manager);
    ~TServer();

    void start();
    void stop();

private:
    void setupKafkaConsumer();
    void startConsumers();
    void consumeMessages(int thread_id);
    void handleKafkaMessage(RdKafka::Message* message, int thread_id);

    TEventManager& m_manager;
    std::vector<std::thread> m_consumer_threads;
    std::mutex m_thread_mutex;
    std::atomic<bool> m_is_running;
    
    // Kafka-related members
    std::shared_ptr<RdKafka::Conf> m_kafka_conf;
    std::vector<RdKafka::KafkaConsumer*> m_kafka_consumers;
    std::string m_kafka_brokers;
    std::string m_kafka_topic;
    std::string m_kafka_group_id;
};
