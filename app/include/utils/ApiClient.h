#pragma once

#include <drogon/drogon.h>
#include <drogon/utils/coroutine.h>
#include <json/json.h>
#include <string>

/**
 * @class ApiClient
 * @brief Singleton HTTP client for calling an upstream service with retry/backoff.
 *
 * Two ways to call it:
 *  - put() / get(): fire-and-forget. Retries happen in the background and the outcome is logged.
 *  - putCoro() / getCoro(): co_await them to get the final response back.
 *
 * Only idempotent methods (PUT, GET) are exposed, so retrying a request that may already
 * have reached the server can't create duplicates.
 */
class ApiClient
{
public:
    /// @brief Returns the global ApiClient instance.
    static ApiClient& instance();

    ApiClient(const ApiClient&)            = delete;
    ApiClient(ApiClient&&)                 = delete;
    ApiClient& operator=(const ApiClient&) = delete;
    ApiClient& operator=(ApiClient&&)      = delete;

    /// @brief Fire-and-forget JSON PUT. Returns immediately; the result is only logged.
    void put(std::string path, Json::Value body, std::string traceId);

    /// @brief Fire-and-forget GET. Returns immediately; the result is only logged.
    void get(std::string path, std::string traceId);

    /**
     * @brief JSON PUT that resolves to the final response (after any retries).
     *
     * Resolves to the last response, including a 4xx or a final 5xx, so the caller can
     * inspect the status. Throws drogon::HttpException if the request never got a
     * response (timeout or network failure on every attempt).
     */
    drogon::Task<drogon::HttpResponsePtr> putCoro(std::string path,
                                                  Json::Value body,
                                                  std::string traceId);

    /// @brief GET that resolves to the final response. Same semantics as putCoro().
    drogon::Task<drogon::HttpResponsePtr> getCoro(std::string path, std::string traceId);

private:
    ApiClient();

    /// @brief Sends the request, retrying timeouts, network failures and 5xx responses
    ///        with exponential backoff and jitter.
    drogon::Task<drogon::HttpResponsePtr> sendWithRetry(drogon::HttpRequestPtr request,
                                                        std::string traceId);

    /// @brief Runs a request in the background and logs how it ended.
    void sendInBackground(drogon::HttpRequestPtr request, std::string traceId);

    std::shared_ptr<drogon::HttpClient> client_; ///< Underlying Drogon HTTP client.
};
