#include <connection/server.hpp>
#include <components/commands.hpp>
#include <iostream>

TServer::TServer(TEventManager& manager)
    : m_manager(manager)
    , m_is_running(false)
#if defined(KAFKA_SERVER)
    , m_kafka_conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL))
    , m_kafka_brokers(SERVER_COMPONENTS::getEnvVar("KAFKA_BOOTSTRAP_SERVERS", "kafka:9092"))
    , m_kafka_topic(SERVER_COMPONENTS::getEnvVar("KAFKA_TOPIC_FROM_BOT_TO_SERVER", "bot_game"))
    , m_kafka_group_id(SERVER_COMPONENTS::getEnvVar("KAFKA_SERVER_GROUP_ID", "server_group"))
#else
    , m_io_service()
    , m_acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 9092))
#endif

{
}

TServer::~TServer()
{
    stop();
}

void TServer::start()
{
    if (m_is_running) return;
    m_is_running = true;
    setup();
}

#if defined(KAFKA_SERVER)
void TServer::stop()
{
    if (!m_is_running) return;

    m_is_running = false;

    for (auto& thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_threads.clear();

    for (auto consumer : m_kafka_consumers)
    {
        consumer->close();
        delete consumer;
    }
    m_kafka_consumers.clear();
}

void TServer::setup()
{
    std::string errstr;

    if (m_kafka_conf->set("session.timeout.ms", "30000", errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set session.timeout.ms: " + errstr);
    }

    if (m_kafka_conf->set("max.poll.interval.ms", "300000", errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set max.poll.interval.ms: " + errstr);
    }

    if (m_kafka_conf->set("request.timeout.ms", "30000", errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set request.timeout.ms: " + errstr);
    }

    if (m_kafka_conf->set("heartbeat.interval.ms", "3000", errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set heartbeat.interval.ms: " + errstr);
    }

    if (m_kafka_conf->set("log_level", "3", errstr) != RdKafka::Conf::CONF_OK)
    {  // 3 = DEBUG
        throw std::runtime_error("Failed to set log level: " + errstr);
    }

    if (m_kafka_conf->set("bootstrap.servers", m_kafka_brokers, errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set bootstrap.servers: " + errstr);
    }

    if (m_kafka_conf->set("group.id", m_kafka_group_id, errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set group.id: " + errstr);
    }

    if (m_kafka_conf->set("enable.auto.commit", "false", errstr) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set enable.auto.commit: " + errstr);
    }

    if (m_kafka_conf->set("partition.assignment.strategy", "roundrobin", errstr) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set partition.assignment.strategy: " + errstr);
    }

    init();
}

void TServer::init()
{
    std::string errstr;
    for (int i = 0; i < MAX_THREADS; ++i) 
    {
        RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(m_kafka_conf.get(), errstr);
        if (!consumer) 
        {
            throw std::runtime_error("Failed to create Kafka consumer: " + errstr);
        }

        std::vector<std::string> topics = {m_kafka_topic};
        RdKafka::ErrorCode err = consumer->subscribe(topics);
        if (err) 
        {
            delete consumer;
            throw std::runtime_error("Failed to subscribe to topic: " + RdKafka::err2str(err));
        }

        m_kafka_consumers.push_back(consumer);
        
        m_threads.emplace_back([this, i]()
            {
                consumeMessages(i);
            }
        );
    }
}

void TServer::handleMessage(RdKafka::Message* message, int thread_id)
{
    try
    {
        const std::string payload(static_cast<const char*>(message->payload()), message->len());
        std::cout << "Thread " << thread_id << " received message: " << payload << std::endl;
        int event_id = m_manager.addEvent(payload);

        std::pair<bool, std::string> response = m_manager.getEventResponse(event_id);
        if(response.first)
        {
            std::cout << "Thread " << thread_id << " processed message, response: " << response.second << std::endl;
        }

        m_manager.removeEvent(event_id);

        if (message->err() == RdKafka::ERR_NO_ERROR)
        {
            m_kafka_consumers[thread_id]->commitAsync(message);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Thread " << thread_id << " exception: " << e.what() << std::endl;
    }

    delete message;
}

void TServer::consumeMessages(int thread_id)
{
    RdKafka::KafkaConsumer* consumer = m_kafka_consumers[thread_id];

    while (m_is_running)
    {
        try
        {
            RdKafka::Message* msg = consumer->consume(1000);
            switch (msg->err())
            {
            case RdKafka::ERR__TIMED_OUT:
                delete msg;
                continue;

            case RdKafka::ERR_NO_ERROR:
                handleMessage(msg, thread_id);
                break;

            case RdKafka::ERR__PARTITION_EOF:
                delete msg;
                continue;

            case RdKafka::ERR__UNKNOWN_TOPIC:
            case RdKafka::ERR__UNKNOWN_PARTITION:
                std::cerr << "Thread " << thread_id << ": Topic/partition not found" << std::endl;
                delete msg;
                continue;

            default:
                std::cerr << "Thread " << thread_id << ": Consumer error: " << msg->errstr() << std::endl;
                delete msg;
                continue;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Consumer thread " << thread_id
                      << " error: " << e.what() << std::endl;
            if (m_is_running)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
}
#else
void TServer::stop()
{
    m_is_running = false;
    m_acceptor.close();

    std::lock_guard<std::mutex> lock(m_thread_mutex);
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void TServer::setup()
{
    init();
    m_io_service.run();
}

void TServer::init()
{
    if (!m_is_running) return;

    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_acceptor.get_executor());
    m_acceptor.async_accept(
        *socket,
        [this, socket](const boost::system::error_code& error)
        {
            if (error)
            {
                if (error != boost::asio::error::operation_aborted)
                {
                    std::cerr << "Accept error: " << error.message() << "\n";
                }
                return;
            }

            if (m_threadCount < MAX_THREADS)
            {
                std::lock_guard<std::mutex> lock(m_thread_mutex);
                m_threads.emplace_back(
                    [this, socket]()
                    {
                        handleMessage(socket);
                        std::lock_guard<std::mutex> lock(m_thread_mutex);
                        m_threadCount--;
                    }
                    );
                m_threadCount++;
            }
            else
            {
                boost::system::error_code ec;
                socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                socket->close(ec);
            }

            if (m_is_running)
            {
                init();
            }
        }
    );
}

void TServer::handleMessage(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    try
    {
        boost::asio::streambuf buf;
        read_until(*socket, buf, '\n');
        std::istream is(&buf);
        std::string message;
        std::getline(is, message);

        int event_id = m_manager.addEvent(message);

        auto response = m_manager.getEventResponse(event_id);
        boost::asio::write(*socket, boost::asio::buffer(response.second + "\n"));

        m_manager.removeEvent(event_id);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception in thread: " << e.what() << std::endl;
    }
}
#endif
