#pragma once

#include <drogon/drogon.h>

namespace advice
{
    /**
     * @brief Runs before any route matching. Useful for global pre-checks and request logging.
     */
    void preRoutingAdvice(const drogon::HttpRequestPtr& request);

    /**
     * @brief Runs after the handler produces a response, before it's sent to the client.
     */
    void postHandlingAdvice(const drogon::HttpRequestPtr& request,
                            const drogon::HttpResponsePtr& response);

    /**
     * @brief Runs once when the Drogon event loop starts.
     */
    void beginningAdvice();
}
