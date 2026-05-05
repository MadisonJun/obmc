#include "BoostStudy.hpp"
#include <iostream>
#include <string_view>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <phosphor-logging/lg2.hpp>

int main()
{
    /* code */
    boost::asio::io_context io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    systemBus->request_name(busName); // Request a well-known name on the bus

#ifdef MANAGER_MANUAL
    sdbusplus::asio::object_server objServer(systemBus, true);
    objServer.add_manager(managerObjectPath); // Add the object manager interface
#else
    sdbusplus::asio::object_server objServer(systemBus);
#endif

    // Register interface
    auto iface = objServer.add_interface(objectPath, interfaceName);

    // Register method
    iface->register_method(
        "HelloWorld", []()
        { lg2::info("Hello, World!"); });

    // Register property
    iface->register_property(
        "RW", std::string("rw"),
        [](const std::string &value, std::string &currentValue)
        {
            lg2::info("Setting RWProperty to: {VALUE}", "VALUE", value);
            currentValue = value;
            return 0;
        });

    iface->register_property("RO", std::string_view("readonly"));

    // Register signal
    iface->register_signal<int, std::string>("Signal");

    iface->initialize();

    // Register signal matches
    std::vector<std::unique_ptr<sdbusplus::bus::match_t>> matches;

    // Match for OEM signal with arguments
    matches.emplace_back(std::make_unique<sdbusplus::bus::match_t>(
        *systemBus,
        sbmr_t::type::signal()
            .append(sbmr_t::interface(interfaceName))
            .append(sbmr_t::member("Signal")),
        [](sdbusplus::message_t &msg)
        {
            auto [intValue, strValue] = msg.unpack<int, std::string>();
            lg2::info("Received signal with int: {INT_VALUE} and string: {STR_VALUE}", "INT_VALUE", intValue, "STR_VALUE", strValue);
        }));

    // Matches the InterfacesAdded signal on the {managerObjectPath} object path.
    matches.emplace_back(std::make_unique<sdbusplus::bus::match_t>(
        *systemBus,
        sbmr_t::interfacesAdded(managerObjectPath),
        [](sdbusplus::message_t &msg)
        {
            // TODO : parse the message to get the interfaces added and their object paths
            lg2::info("Interfaces added at path: {PATH}, interfaces: {INTERFACES}", "PATH", msg.get_path(), "INTERFACES", msg.get_interface());
        }));

    // Matches the InterfacesRemoved signal on the {managerObjectPath} object path.
    matches.emplace_back(std::make_unique<sdbusplus::bus::match_t>(
        *systemBus,
        sbmr_t::interfacesRemoved(managerObjectPath),
        [](sdbusplus::message_t &msg)
        {
            // TODO : parse the message to get the interfaces removed and their object paths
            lg2::info("Interfaces removed at path: {PATH}, interfaces: {INTERFACES}", "PATH", msg.get_path(), "INTERFACES", msg.get_interface());
        }));

    // loop forever
    io.run();
    return 0;
}