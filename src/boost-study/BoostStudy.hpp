#pragma once

#include <sdbusplus/bus/match.hpp>
#include <vector>
#include <concepts>

#include "utils.hpp"

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

using boostDateType = std::vector<std::pair<std::string, std::map<std::string, BasicVariantType>>>;

