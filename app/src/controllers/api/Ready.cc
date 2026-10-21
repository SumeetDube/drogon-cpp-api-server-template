#include "controllers/api/V1.h"

#include <drogon/HttpResponse.h>
#include <drogon/orm/DbClient.h>
#include <drogon/utils/Utilities.h>

#include "controllers/models/response/BaseResponse.h"
#include "custom_exceptions/ControllerError.h"
#include "db/Db.h"

drogon::Task<drogon::HttpResponsePtr> api::v1::ready(drogon::HttpRequestPtr)
{
    std::string traceId = drogon::utils::getUuid();

    drogon::orm::DbClientPtr client = drogon::app().getDbClient("default");

    // Suspends instead of blocking the IO thread; other requests keep being served.
    if (!co_await db::ping(client, traceId))
    {
        // Exceptions thrown from coroutine handlers still reach the global exception handler.
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
    co_return response;
}
