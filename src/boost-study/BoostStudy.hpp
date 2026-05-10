#pragma once

#include <sdbusplus/bus/match.hpp>
#include <variant>
#include <vector>
#include <type_traits>
#include <concepts>

constexpr auto busName = "xyz.openbmc_project.boost.study";
constexpr auto objectPath = "/xyz/openbmc_project/boost/study";
constexpr auto interfaceName = "xyz.openbmc_project.boost.study.Interface";
constexpr auto interfaceHeader = "xyz.openbmc_project.boost.study.";

#ifdef MANAGER_MANUAL
constexpr auto managerObjectPath = "/xyz/openbmc_project/boost/manager";
#else
constexpr auto managerObjectPath = "/";
#endif

namespace sbmr_t = sdbusplus::bus::match::rules;

using BasicVariantType =
    std::variant<std::vector<std::string>, std::vector<uint8_t>, std::vector<std::uint64_t>, std::string,
                 int64_t, uint64_t, double, int32_t, uint32_t, int16_t, uint16_t, uint8_t, bool>;

using boostDateType = std::vector<std::pair<std::string, std::map<std::string, BasicVariantType>>>;

