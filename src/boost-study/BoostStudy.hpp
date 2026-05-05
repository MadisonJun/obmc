#pragma once

#include <sdbusplus/bus/match.hpp>

constexpr auto busName = "xyz.openbmc_project.boost.study";
constexpr auto objectPath = "/xyz/openbmc_project/boost/study";
constexpr auto interfaceName = "xyz.openbmc_project.boost.study.Interface";

#ifdef MANAGER_MANUAL
constexpr auto managerObjectPath = "/xyz/openbmc_project/boost/manager";
#else
constexpr auto managerObjectPath = "/";
#endif

namespace sbmr_t = sdbusplus::bus::match::rules;
