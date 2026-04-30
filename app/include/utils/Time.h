#pragma once

#include <cstdint>

namespace timeutils
{
    /**
     * @brief Current wall-clock time in milliseconds since the Unix epoch.
     */
    uint64_t getCurrentTimestampMs();
}
