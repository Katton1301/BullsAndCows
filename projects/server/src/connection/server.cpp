#include <connection/server.hpp>
#include <iostream>

TServer::TServer(TEventManager& manager)
    : m_manager(manager)
    , m_is_running(false)
    , m_kafka_conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL))
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
    std::string brokers = "kafka:9092";
    std::string topic_name = "game_command";
    std::string group_id = "server_group";

    if (m_kafka_conf->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK) 
    {
        throw std::runtime_error("Failed to set bootstrap.servers: " + errstr);
    }

    if (m_kafka_conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) 
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
    std::string topic_name = "game_command";
    
    for (int i = 0; i < MAX_THREADS; ++i) 
    {
        RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create(m_kafka_conf.get(), errstr);
        if (!consumer) 
        {
            throw std::runtime_error("Failed to create Kafka consumer: " + errstr);
        }

        std::vector<std::string> topics = {topic_name};
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
    
    while (m_is_running) {
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
