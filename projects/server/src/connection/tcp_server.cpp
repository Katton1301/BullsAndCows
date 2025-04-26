#include <connection/tcp_server.hpp>


TTcpServer::TTcpServer(boost::asio::io_service& _io_service, short port, TEventManager& manager)
    : m_acceptor(_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
    , m_manager(manager)
    , m_isRunning(true)
//, m_kafka_conf(RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL))
{
    m_threadCount = 0;
    startAccept();
    setupKafkaProducer();
}

TTcpServer::~TTcpServer()
{
    /*
        if (m_kafka_producer)
        {
            m_kafka_producer->flush(5000);
            delete m_kafka_producer;
        }
        */
    stop();
}

void TTcpServer::stop()
{
    m_isRunning = false;
    m_acceptor.close();

    std::lock_guard<std::mutex> lock(m_threadMutex);
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void TTcpServer::startAccept()
{
    if (!m_isRunning) return;

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
                std::lock_guard<std::mutex> lock(m_threadMutex);
                m_threads.emplace_back(
                    [this, socket]()
                    {
                        handleClient(socket);
                        std::lock_guard<std::mutex> lock(m_threadMutex);
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

            if (m_isRunning)
            {
                startAccept();
            }
        }
    );
}

void TTcpServer::handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
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
        boost::asio::write(*socket, boost::asio::buffer(response + "\n"));

        m_manager.removeEvent(event_id);
    }
    catch (std::exception& e)
    {
        std::cout << "Exception in thread: " << e.what() << std::endl;
    }
}

void TTcpServer::setupKafkaProducer()
{

    /*
    std::string errstr;

    if (m_kafka_conf->set("bootstrap.servers", "localhost:9092", errstr) != RdKafka::Conf::CONF_OK)
    {
        std::cout << "Kafka config error: " << errstr << std::endl;
        return;
    }

    m_kafka_producer = RdKafka::Producer::create(m_kafka_conf.get(), errstr);
    if (!m_kafka_producer)
    {
        std::cout << "Failed to create Kafka producer: " << errstr << std::endl;
    }*/
}
