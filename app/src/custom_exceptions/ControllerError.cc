#include "custom_exceptions/ControllerError.h"

#include <drogon/drogon.h>

#include "utils/Time.h"

ControllerError::ControllerError(const std::string& errorCode,
                                 const std::string& errorLevel,
                                 const std::string& errorMessage,
                                 const std::string& traceId,
                                 const drogon::HttpStatusCode& statusCode)
    : std::runtime_error(errorMessage),
      errorCode(errorCode),
      errorLevel(errorLevel),
      errorMessage(errorMessage),
      traceId(traceId),
      statusCode(statusCode)
{
    this->errorTimestampMs = timeutils::getCurrentTimestampMs();
}

const char* ControllerError::what() const noexcept
{
    return errorMessage.c_str();
}
