#include "utils/Time.h"

#include <chrono>

uint64_t timeutils::getCurrentTimestampMs()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
