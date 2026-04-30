#pragma once

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

/**
 * @struct SuccessResponse
 * @brief JSON envelope for successful API responses.
 */
struct SuccessResponse
{
    const bool status = true; ///< Always true for successes.
    std::string message;      ///< Human-readable summary.
    std::string traceId;      ///< Trace ID for correlating logs.
    Json::Value details;      ///< Endpoint-specific payload.

    /// @brief Implicit conversion to Json::Value for use with Drogon's JSON response helper.
    operator Json::Value() const
    {
        Json::Value json;
        json["status"]   = status;
        json["message"]  = message;
        json["trace_id"] = traceId;
        json["details"]  = details;
        return json;
    }
};

/**
 * @struct Details
 * @brief Error details embedded inside a FailedResponse.
 */
struct Details
{
    std::string errorCode;    ///< Machine-readable error code.
    std::string errorMessage; ///< Human-readable description.
    uint64_t timestampMs;     ///< Time the error was raised (ms since epoch).
};

/**
 * @struct FailedResponse
 * @brief JSON envelope for failed API responses.
 */
struct FailedResponse
{
    const bool status = false; ///< Always false for failures.
    std::string message;       ///< Human-readable summary.
    std::string traceId;       ///< Trace ID for correlating logs.
    Details details;           ///< Structured error details.

    /// @brief Implicit conversion to Json::Value for use with Drogon's JSON response helper.
    operator Json::Value() const
    {
        Json::Value json;
        json["status"]   = status;
        json["message"]  = message;
        json["trace_id"] = traceId;

        Json::Value detailsJson;
        detailsJson["error_code"]    = details.errorCode;
        detailsJson["error_message"] = details.errorMessage;
        detailsJson["timestamp_ms"]  = (Json::UInt64) details.timestampMs;
        json["details"] = detailsJson;
        return json;
    }
};
