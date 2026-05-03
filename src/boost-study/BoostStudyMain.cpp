#include "BoostStudy.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>

int main()
{
    /* code */
    boost::asio::io_context io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name(busName); // Request a well-known name on the bus

#ifdef MANAGER_MANAUL
    sdbusplus::asio::object_server objServer(systemBus, true);
    objServer.add_manager("/xyz/openbmc_project/manager"); // Add the object manager interface
#else
    sdbusplus::asio::object_server objServer(systemBus);
#endif

    // Register interface
    auto iface = objServer.add_interface(objectPath, interfaceName);

    iface->initialize();

    // loop forever
    io.run();
    return 0;
}