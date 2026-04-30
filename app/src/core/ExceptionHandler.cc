#include "core/ExceptionHandler.h"

#include <drogon/HttpResponse.h>
#include <drogon/utils/Utilities.h>

#include "controllers/models/response/BaseResponse.h"
#include "custom_exceptions/ControllerError.h"
#include "utils/Time.h"

void exceptionHandler(const std::exception& error,
                      const drogon::HttpRequestPtr&,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    if (const ControllerError* err = dynamic_cast<const ControllerError*>(&error))
    {
        LOG_WARN << "[" << err->traceId << "] " << err->errorCode
                 << ": " << err->errorMessage;

        drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(
            FailedResponse{
                .message = "Request failed",
                .traceId = err->traceId,
                .details = Details{
                    .errorCode    = err->errorCode,
                    .errorMessage = err->errorMessage,
                    .timestampMs  = err->errorTimestampMs,
                },
            });
        response->setStatusCode(err->statusCode);
        callback(response);
        return;
    }

    std::string traceId = drogon::utils::getUuid();
    LOG_ERROR << "[" << traceId << "] Unhandled exception: " << error.what();

    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(
        FailedResponse{
            .message = "An unexpected error occurred",
            .traceId = traceId,
            .details = Details{
                .errorCode    = "INTERNAL_SERVER_ERROR",
                .errorMessage = error.what(),
                .timestampMs  = timeutils::getCurrentTimestampMs(),
            },
        });
    response->setStatusCode(drogon::k500InternalServerError);
    callback(response);
}
