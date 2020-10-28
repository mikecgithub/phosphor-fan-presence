/**
 * Copyright © 2020 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "sdbusplus.hpp"

#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>
#include <sdeventplus/source/signal.hpp>

#include <filesystem>
#include <fstream>

namespace phosphor::fan
{

namespace fs = std::filesystem;
using json = nlohmann::json;
using namespace phosphor::logging;

constexpr auto confOverridePath = "/etc/phosphor-fan-presence";
constexpr auto confBasePath = "/usr/share/phosphor-fan-presence";
constexpr auto confCompatIntf =
    "xyz.openbmc_project.Configuration.IBMCompatibleSystem";
constexpr auto confCompatProp = "Names";

class JsonConfig
{
  public:
    /**
     * Get the json configuration file. The first location found to contain
     * the json config file for the given fan application is used from the
     * following locations in order.
     * 1.) From the confOverridePath location
     * 2.) From config file found using an entry from a list obtained from an
     * interface's property as a relative path extension on the base path where:
     *     interface = Interface set in confCompatIntf with the property
     *     property = Property set in confCompatProp containing a list of
     *                subdirectories in priority order to find a config
     * 3.) *DEFAULT* - From the confBasePath location
     *
     * @brief Get the configuration file to be used
     *
     * @param[in] bus - The dbus bus object
     * @param[in] appName - The phosphor-fan-presence application name
     * @param[in] fileName - Application's configuration file's name
     * @param[in] isOptional - Config file is optional, default to 'false'
     *
     * @return filesystem path
     *     The filesystem path to the configuration file to use
     */
    static const fs::path getConfFile(sdbusplus::bus::bus& bus,
                                      const std::string& appName,
                                      const std::string& fileName,
                                      bool isOptional = false)
    {
        // Check override location
        fs::path confFile = fs::path{confOverridePath} / appName / fileName;
        if (fs::exists(confFile))
        {
            return confFile;
        }

        // Default base path used if no config file found at any locations
        // provided on dbus objects with the compatible interface
        confFile = fs::path{confBasePath} / appName / fileName;

        // Get all objects implementing the compatible interface
        auto objects =
            util::SDBusPlus::getSubTreePathsRaw(bus, "/", confCompatIntf, 0);
        for (auto& path : objects)
        {
            try
            {
                // Retrieve json config compatible relative path locations
                auto confCompatValue =
                    util::SDBusPlus::getProperty<std::vector<std::string>>(
                        bus, path, confCompatIntf, confCompatProp);
                // Look for a config file at each entry relative to the base
                // path and use the first one found
                auto it = std::find_if(
                    confCompatValue.begin(), confCompatValue.end(),
                    [&confFile, &appName, &fileName](auto const& entry) {
                        confFile =
                            fs::path{confBasePath} / appName / entry / fileName;
                        return fs::exists(confFile);
                    });
                if (it != confCompatValue.end())
                {
                    // Use the first config file found at a listed location
                    break;
                }
            }
            catch (const util::DBusError&)
            {
                // Property unavailable on object.
                // Set to default base path and continue to check next object
            }
            confFile = fs::path{confBasePath} / appName / fileName;
        }

        if (!fs::exists(confFile))
        {
            if (!isOptional)
            {
                log<level::ERR>(
                    fmt::format("No JSON config file found. Default file: {}",
                                confFile.string())
                        .c_str());
                throw std::runtime_error(
                    fmt::format("No JSON config file found. Default file: {}",
                                confFile.string())
                        .c_str());
            }
            else
            {
                confFile.clear();
            }
        }

        return confFile;
    }

    /**
     * @brief Load the JSON config file
     *
     * @param[in] confFile - File system path of the configuration file to load
     *
     * @return Parsed JSON object
     *     The parsed JSON configuration file object
     */
    static const json load(const fs::path& confFile)
    {
        std::ifstream file;
        json jsonConf;

        if (!confFile.empty() && fs::exists(confFile))
        {
            log<level::INFO>(
                fmt::format("Loading configuration from {}", confFile.string())
                    .c_str());
            file.open(confFile);
            try
            {
                jsonConf = json::parse(file);
            }
            catch (std::exception& e)
            {
                log<level::ERR>(
                    fmt::format(
                        "Failed to parse JSON config file: {}, error: {}",
                        confFile.string(), e.what())
                        .c_str());
                throw std::runtime_error(
                    fmt::format(
                        "Failed to parse JSON config file: {}, error: {}",
                        confFile.string(), e.what())
                        .c_str());
            }
        }
        else
        {
            log<level::ERR>(fmt::format("Unable to open JSON config file: {}",
                                        confFile.string())
                                .c_str());
            throw std::runtime_error(
                fmt::format("Unable to open JSON config file: {}",
                            confFile.string())
                    .c_str());
        }

        return jsonConf;
    }
};

} // namespace phosphor::fan
