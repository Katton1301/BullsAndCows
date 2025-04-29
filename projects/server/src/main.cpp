#include <connection/server.hpp>
#include <librdkafka/rdkafkacpp.h>

RdKafka::Producer* CreateKafkaProducer(const std::string& brokers) 
{
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    
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
    try 
    {
        const std::string brokers = "kafka:9092";
        RdKafka::Producer* kafka_producer = CreateKafkaProducer(brokers);
        if (!kafka_producer)
        {
            std::cerr << "Failed to initialize Kafka producer" << std::endl;
            return 1;
        }
        TEventManager manager;
        
        TEventProcessor processor(manager, kafka_producer);
        processor.start();

        TServer server(manager);
        server.start();

        processor.stop();
        
        while (kafka_producer->outq_len() > 0) {
            std::cerr << "Waiting for " << kafka_producer->outq_len() << std::endl;
            kafka_producer->poll(1000);
        }
        delete kafka_producer;
        
        server.stop();
    }
    catch (std::exception const & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
