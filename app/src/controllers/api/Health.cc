#include "controllers/api/V1.h"

#include <drogon/HttpResponse.h>
#include <drogon/utils/Utilities.h>

#include "controllers/models/response/BaseResponse.h"

void api::v1::health(const drogon::HttpRequestPtr&,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(
        SuccessResponse{
            .message = "OK",
            .traceId = drogon::utils::getUuid(),
            .details = Json::Value(Json::objectValue),
        });
    response->setStatusCode(drogon::k200OK);
    callback(response);
}
