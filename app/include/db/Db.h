#pragma once

#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include <memory>
#include <string>

namespace db
{
    /**
     * @brief Pings the database via a transaction. Returns false on any error.
     */
    bool ping(const std::shared_ptr<drogon::orm::Transaction>& transaction,
              const std::string& traceId);

    /**
     * @brief Pings the database via a DbClient. Returns false on any error.
     */
    bool ping(const std::shared_ptr<drogon::orm::DbClient>& client,
              const std::string& traceId);
}
