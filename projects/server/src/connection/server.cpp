#include <connection/server.hpp>
#include <iostream>

std::string TServer::getEnvVar(const std::string& name, const std::string& defaultValue) {
    const char* value = std::getenv(name.c_str());
    return value ? value : defaultValue;
}

TServer::TServer(TEventManager& manager)
    : m_manager(manager)
    , m_is_running(false)
    , m_kafka_conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL))
    , m_kafka_brokers(getEnvVar("KAFKA_BOOTSTRAP_SERVERS", "kafka:9092"))
    , m_kafka_topic(getEnvVar("KAFKA_TOPIC", "game_command"))
    , m_kafka_group_id(getEnvVar("KAFKA_GROUP_ID", "server_group"))
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
    setupKafkaConsumer();
    startConsumers();
}

void TServer::stop()
{
    if (!m_is_running) return;
    
    m_is_running = false;
    
    for (auto& thread : m_consumer_threads) 
    {
        if (thread.joinable()) 
        {
            thread.join();
        }
    }
    m_consumer_threads.clear();
    
    for (auto consumer : m_kafka_consumers) 
    {
        consumer->close();
        delete consumer;
    }
    m_kafka_consumers.clear();
}

void TServer::setupKafkaConsumer()
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
}

void TServer::startConsumers()
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
        
        m_consumer_threads.emplace_back([this, i]()
            {
                consumeMessages(i);
            }
        );
    }
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
                    handleKafkaMessage(msg, thread_id);
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

void TServer::handleKafkaMessage(RdKafka::Message* message, int thread_id)
{
    try 
    {
        const std::string payload(static_cast<const char*>(message->payload()), message->len());
        std::cout << "Thread " << thread_id << " received message: " << payload << std::endl;
        int event_id = m_manager.addEvent(payload);
        
        std::string response;
        while (m_is_running && response.empty()) 
        {
            response = m_manager.getEventResponse(event_id);
            if (response.empty()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (!response.empty()) 
        {
            std::cout << "Thread " << thread_id << " processed message, response: " << response << std::endl;
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
