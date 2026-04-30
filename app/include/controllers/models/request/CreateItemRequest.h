#pragma once

#include <string>

/**
 * @struct CreateItemRequest
 * @brief Example request DTO parsed from JSON via reflect-cpp.
 *
 * Field names below map 1:1 to JSON keys. Reflect-cpp reads aggregate
 * structs reflectively, so no macros or annotations are needed.
 */
struct CreateItemRequest
{
    std::string name;     ///< Human-readable item name.
    int quantity;         ///< Number of units; must be >= 1.
    std::string trace_id; ///< Caller-supplied trace ID for log correlation; required, non-empty.
};
