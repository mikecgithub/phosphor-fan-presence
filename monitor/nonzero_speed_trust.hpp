#pragma once

#include "trust_group.hpp"

namespace phosphor
{
namespace fan
{
namespace trust
{

/**
 * @class NonzeroSpeed
 *
 * A trust group where the sensors in the group are trusted as long
 * as at least one of them has a nonzero speed.  If all sensors
 * have a speed of zero, then no sensor in the group is trusted.
 */
class NonzeroSpeed : public Group
{
    public:

        NonzeroSpeed() = delete;
        ~NonzeroSpeed() = default;
        NonzeroSpeed(const NonzeroSpeed&) = delete;
        NonzeroSpeed& operator=(const NonzeroSpeed&) = delete;
        NonzeroSpeed(NonzeroSpeed&&) = default;
        NonzeroSpeed& operator=(NonzeroSpeed&&) = default;

        /**
         * Constructor
         *
         * @param[in] names - the names of the sensors and its inclusion in
         * determining trust for the group
         */
        explicit NonzeroSpeed(const std::vector<GroupDefinition>& names) :
                Group(names)
        {
        }

    private:

        /**
         * Determines if the group is trusted by checking
         * if any sensor has a nonzero speed.  If all speeds
         * are zero, then no sensors in the group are trusted.
         *
         * @return bool - if group is trusted or not
         */
        bool checkGroupTrust() override
        {
            return std::any_of(
                    _sensors.begin(),
                    _sensors.end(),
                    [](const auto& s)
                    {
                        return std::get<0>(s)->getInput() != 0;
                    });
        }
};

}
}
}
