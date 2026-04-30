#include "controllers/api/V1.h"

#include <drogon/HttpResponse.h>
#include <drogon/utils/Utilities.h>

#include "controllers/models/response/BaseResponse.h"
#include "custom_exceptions/ControllerError.h"

void api::v1::createItem(const drogon::HttpRequestPtr&,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                         CreateItemRequest&& payload)
{
    std::string traceId = payload.trace_id;

    if (traceId.empty())
    {
        throw ControllerError("INVALID_TRACE_ID", "ERROR",
                              "trace_id must not be empty",
                              drogon::utils::getUuid(),
                              drogon::HttpStatusCode::k422UnprocessableEntity);
    }
    if (payload.name.empty())
    {
        throw ControllerError("INVALID_NAME", "ERROR",
                              "name must not be empty",
                              traceId,
                              drogon::HttpStatusCode::k422UnprocessableEntity);
    }
    if (payload.quantity < 1)
    {
        throw ControllerError("INVALID_QUANTITY", "ERROR",
                              "quantity must be >= 1",
                              traceId,
                              drogon::HttpStatusCode::k422UnprocessableEntity);
    }

    Json::Value details;
    details["id"]       = drogon::utils::getUuid();
    details["name"]     = payload.name;
    details["quantity"] = payload.quantity;

    LOG_INFO << "[" << traceId << "] Item created: " << details["id"].asString()
             << " name=" << payload.name
             << " quantity=" << payload.quantity;

    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(
        SuccessResponse{
            .message = "Item created",
            .traceId = traceId,
            .details = details,
        });
    response->setStatusCode(drogon::k201Created);
    callback(response);
}
