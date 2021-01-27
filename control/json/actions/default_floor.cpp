/**
 * Copyright © 2021 IBM Corporation
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
#include "default_floor.hpp"

#include "types.hpp"
#include "zone.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <tuple>

namespace phosphor::fan::control::json
{

using json = nlohmann::json;

DefaultFloor::DefaultFloor(const json&) : ActionBase(DefaultFloor::name)
{
    // There are no JSON configuration parameters for this action
}

void DefaultFloor::run(Zone& zone, const Group& group)
{
    // Set/update the services of the group
    zone.setServices(&group);
    auto services = zone.getGroupServices(&group);
    auto defFloor =
        std::any_of(services.begin(), services.end(),
                    [](const auto& s) { return !std::get<hasOwnerPos>(s); });
    if (defFloor)
    {
        zone.setFloor(zone.getDefFloor());
    }
    // Update fan control floor change allowed
    zone.setFloorChangeAllow(&group, !defFloor);
}

} // namespace phosphor::fan::control::json