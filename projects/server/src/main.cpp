#include <connection/tcp_server.hpp>

int main()
{
    try
    {
        boost::asio::io_service _io_service;
        TEventManager manager;

        TEventProcessor processor(manager/*, nullptr*/);
        processor.start();

        TTcpServer server(_io_service, 9092, manager);

        _io_service.run();

        processor.stop();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
