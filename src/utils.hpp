#pragma once
#include <type_traits>
#include <variant>
#include <vector>
#include <string>
#include <functional>
#include <sdbusplus/asio/connection.hpp>
#include <boost/container/flat_set.hpp>

using BasicVariantType =
    std::variant<std::vector<std::string>, std::vector<uint8_t>, std::vector<std::uint64_t>, std::string,
                 int64_t, uint64_t, double, int32_t, uint32_t, int16_t, uint16_t, uint8_t, bool>;

using getObjectType = std::vector<
    std::pair<std::string, std::vector<std::string>>>;

using getSubTreeType = std::vector<
    std::pair<std::string, std::vector<std::pair<std::string, std::vector<std::string>>>>>;

namespace utils
{
    namespace objectMapper
    {
        constexpr auto service = "xyz.openbmc_project.ObjectMapper";
        constexpr auto object = "/xyz/openbmc_project/object_mapper";
        constexpr auto interface = "xyz.openbmc_project.ObjectMapper";

        namespace details
        {
            /** @brief Asynchronous query for information about a specific object. Uses the default timeout value.
             *
             *  @param[in] handler - A function object that will be invoked as a continuation of an asynchronous dbus method call. The arguments to be resolved upon return are inferred from the handler's signature and then passed along with the error code and dbus lookup information.
             *  @param[in] systemBus - system bus.
             *  @param[in] path - Search path.
             *  @param[in] interfaces - Search interfaces.
             *
             */
            void getObject(std::move_only_function<void(boost::system::error_code &ec, const getObjectType &interfaceObject)> &&callback,
                           sdbusplus::asio::connection &systemBus, const std::string &path, boost::container::flat_set<std::string> &&interfaces = {})
            {
                systemBus.async_method_call(std::move(callback), service, object, interface, "GetObject", path, interfaces);
            }

            /** @brief Asynchronously retrieves detailed information for all objects in the subtree. Uses the default timeout value.
             *
             *  @param[in] handler - A function object that will be invoked as a continuation of an asynchronous dbus method call. The arguments to be resolved upon return are inferred from the handler's signature and then passed along with the error code and dbus lookup information.
             *  @param[in] systemBus - system bus.
             *  @param[in] path - Search path.
             *  @param[in] depth - Search depth.
             *  @param[in] interfaces - Search interfaces.
             *
             */
            void getSubTree(std::move_only_function<void(boost::system::error_code &ec, const getSubTreeType &interfaceSubtree)> &&callback,
                            sdbusplus::asio::connection &systemBus, const std::string &path, const int32_t depth, boost::container::flat_set<std::string> &&interfaces = {})
            {
                systemBus.async_method_call(std::move(callback), service, object, interface, "GetSubTree", path, depth, interfaces);
            }

        }
    }
}

namespace objectMapper_t = utils::objectMapper::details;