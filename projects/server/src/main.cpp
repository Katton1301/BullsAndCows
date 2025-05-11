#include <connection/server.hpp>
#include <components/commands.hpp>
#include <librdkafka/rdkafkacpp.h>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void signal_handler([[maybe_unused]]int signal)
{
    running = false;
}

RdKafka::Producer* CreateKafkaProducer() 
{
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    
    const char* brokers = std::getenv("KAFKA_BOOTSTRAP_SERVERS");
    if (!brokers) brokers = "localhost:9092";
    
    if (conf->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set bootstrap.servers: " << errstr << std::endl;
        delete conf;
        return nullptr;
    }

    if (conf->set("message.timeout.ms", "5000", errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set message.timeout.ms: " << errstr << std::endl;
    }

    RdKafka::Producer* producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
    }

    delete conf;
    return producer;
}

int main() 
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try 
    {
        RdKafka::Producer* kafka_producer = CreateKafkaProducer();
        if (!kafka_producer) 
        {
            std::cerr << "Failed to initialize Kafka producer" << std::endl;
            return 1;
        }
        TEventManager manager;
        
        std::string producer_topic = SERVER_COMPONENTS::getEnvVar("KAFKA_TOPIC_PRODUCER", "game_bot");
        TEventProcessor processor(manager, kafka_producer, producer_topic);
        processor.start();

        TServer server(manager);
        server.start();

        std::cout << "Server started successfully. Press Ctrl+C to stop..." << std::endl;

        while (running) 
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            if (kafka_producer->outq_len() > 0) 
            {
                kafka_producer->poll(0);
            }
        }

        // Грациозное завершение
        std::cout << "Shutting down server..." << std::endl;
        processor.stop();
        server.stop();

        while (kafka_producer->outq_len() > 0) 
        {
            std::cout << "Waiting for " << kafka_producer->outq_len()
                      << " pending messages..." << std::endl;
            kafka_producer->poll(1000);
        }

        delete kafka_producer;
        std::cout << "Server stopped successfully" << std::endl;
    }
    catch (std::exception const & e) 
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
