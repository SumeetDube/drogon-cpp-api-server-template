#pragma once

#include <drogon/orm/DbClient.h>
#include <drogon/utils/coroutine.h>
#include <string>

namespace db
{
    /**
     * @brief Pings the database without blocking the event loop. Returns false on any error.
     *
     * Works for both a DbClient and a Transaction (a Transaction is a DbClient).
     * Parameters are taken by value: references could dangle across the co_await.
     */
    drogon::Task<bool> ping(drogon::orm::DbClientPtr client, std::string traceId);
}
