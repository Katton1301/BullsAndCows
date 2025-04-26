#pragma once
#include <events/event_processor.hpp>
#include <boost/asio.hpp>
class TTcpServer
{
public:
    enum
    {
        MAX_THREADS = 4
    };
    TTcpServer(boost::asio::io_service& _io_service, short port, TEventManager& manager);
    ~TTcpServer();

private:
    void stop();

    void startAccept();
    void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void setupKafkaProducer();

    std::vector<std::thread> m_threads;
    boost::asio::ip::tcp::acceptor m_acceptor;
    TEventManager& m_manager;
    uint32_t m_threadCount;
    std::mutex m_threadMutex;
    std::atomic<bool> m_isRunning;
    //std::shared_ptr<RdKafka::Conf> m_kafka_conf;
    //RdKafka::Producer* m_kafka_producer = nullptr;
};
