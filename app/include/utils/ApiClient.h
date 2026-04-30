#pragma once

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

/**
 * @class ApiClient
 * @brief Singleton HTTP client for calling an upstream service with retry/backoff.
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

    /// @brief Sends a JSON POST request to the upstream service.
    void post(const std::string& path,
              const Json::Value& body,
              const std::string& traceId);

    /// @brief Sends a GET request to the upstream service.
    void get(const std::string& path,
             const std::string& traceId);

private:
    ApiClient();

    /// @brief Sends the request and retries on transient failures with exponential backoff.
    void sendWithRetry(const drogon::HttpRequestPtr& request,
                       int attempt,
                       const std::string& traceId);

    std::shared_ptr<drogon::HttpClient> client_; ///< Underlying Drogon HTTP client.
};
