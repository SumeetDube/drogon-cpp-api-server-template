#pragma once

#include <drogon/drogon.h>
#include <stdexcept>
#include <string>

/**
 * @class ControllerError
 * @brief Exception type carrying enough context for a structured JSON error response.
 */
class ControllerError : public std::runtime_error
{
public:
    uint64_t errorTimestampMs;          ///< Time the error was raised (ms since epoch).
    std::string errorCode;              ///< Machine-readable error code (e.g. "VALIDATION_FAILED").
    std::string errorLevel;             ///< Severity tag (e.g. "FATAL", "ERROR", "WARN").
    std::string errorMessage;           ///< Human-readable description.
    std::string traceId;                ///< Trace ID for correlating logs.
    drogon::HttpStatusCode statusCode;  ///< HTTP status code to return to the client.

    /// @brief Constructs a ControllerError. Captures timestamp automatically.
    ControllerError(const std::string& errorCode,
                    const std::string& errorLevel,
                    const std::string& errorMessage,
                    const std::string& traceId,
                    const drogon::HttpStatusCode& statusCode = drogon::HttpStatusCode::k400BadRequest);

    /// @brief Returns the error message (overrides std::runtime_error::what).
    const char* what() const noexcept override;
};
