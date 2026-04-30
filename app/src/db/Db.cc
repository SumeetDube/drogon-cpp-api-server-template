#include "db/Db.h"

#include <drogon/orm/DbClient.h>
#include <drogon/orm/Result.h>
#include <exception>
#include <trantor/utils/Logger.h>

namespace db
{
    bool ping(const std::shared_ptr<drogon::orm::Transaction>& transaction,
              const std::string& traceId)
    {
        try
        {
            LOG_DEBUG << "[" << traceId << "] DB ping via transaction";
            drogon::orm::Result result = transaction->execSqlSync("SELECT 1");
            return !result.empty();
        }
        catch (const std::exception& error)
        {
            LOG_WARN << "[" << traceId << "] DB ping failed (transaction): "
                     << error.what();
            return false;
        }
    }

    bool ping(const std::shared_ptr<drogon::orm::DbClient>& client,
              const std::string& traceId)
    {
        try
        {
            LOG_DEBUG << "[" << traceId << "] DB ping via DbClient";
            drogon::orm::Result result = client->execSqlSync("SELECT 1");
            return !result.empty();
        }
        catch (const std::exception& error)
        {
            LOG_WARN << "[" << traceId << "] DB ping failed (client): "
                     << error.what();
            return false;
        }
    }
}
