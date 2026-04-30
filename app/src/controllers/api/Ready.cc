#include "controllers/api/V1.h"

#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include <drogon/utils/Utilities.h>

#include "controllers/models/response/BaseResponse.h"
#include "custom_exceptions/ControllerError.h"
#include "db/Db.h"

void api::v1::ready(const drogon::HttpRequestPtr&,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    std::string traceId = drogon::utils::getUuid();

    drogon::orm::DbClientPtr client = drogon::app().getDbClient("default");

    if (!db::ping(client, traceId))
    {
        throw ControllerError("DB_NOT_READY", "ERROR",
                              "Database is not reachable",
                              traceId,
                              drogon::HttpStatusCode::k503ServiceUnavailable);
    }

    drogon::HttpResponsePtr response = drogon::HttpResponse::newHttpJsonResponse(
        SuccessResponse{
            .message = "Ready",
            .traceId = traceId,
            .details = Json::Value(Json::objectValue),
        });
    response->setStatusCode(drogon::k200OK);
    callback(response);
}
