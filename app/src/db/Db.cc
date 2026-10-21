#include "db/Db.h"

#include <drogon/orm/Result.h>
#include <exception>
#include <trantor/utils/Logger.h>

namespace db
{
    drogon::Task<bool> ping(drogon::orm::DbClientPtr client, std::string traceId)
    {
        if (!client)
        {
            LOG_WARN << "[" << traceId << "] DB ping failed: no DB client configured";
            co_return false;
        }

        try
        {
            LOG_DEBUG << "[" << traceId << "] DB ping";
            drogon::orm::Result result = co_await client->execSqlCoro("SELECT 1");
            co_return !result.empty();
        }
        catch (const std::exception& error)
        {
            LOG_WARN << "[" << traceId << "] DB ping failed: " << error.what();
            co_return false;
        }
    }
}
