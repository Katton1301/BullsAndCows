#pragma once
#include <events/event_processor.hpp>
#if defined(KAFKA_SERVER)
    #include <librdkafka/rdkafkacpp.h>
#else
    #include <boost/asio.hpp>
#endif
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
    void init();
    void setup();

    TEventManager& m_manager;
    std::vector<std::thread> m_threads;
    std::mutex m_thread_mutex;
    std::atomic<bool> m_is_running;

#if defined(KAFKA_SERVER)
    void setupKafkaConsumer();
    void consumeMessages(int thread_id);
    void handleMessage(RdKafka::Message* message, int thread_id);

    // Kafka-related members
    std::shared_ptr<RdKafka::Conf> m_kafka_conf;
    std::vector<RdKafka::KafkaConsumer*> m_kafka_consumers;
    std::string m_kafka_brokers;
    std::string m_kafka_topic;
    std::string m_kafka_group_id;
#else
    void handleMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::io_service m_io_service;
    boost::asio::ip::tcp::acceptor m_acceptor;
    uint32_t m_threadCount;
#endif
};
