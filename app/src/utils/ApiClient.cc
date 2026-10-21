#include "utils/ApiClient.h"

#include <chrono>
#include <random>

#include "core/Config.h"

namespace
{
    bool isRetryable(drogon::ReqResult result)
    {
        return result == drogon::ReqResult::Timeout ||
               result == drogon::ReqResult::NetworkFailure;
    }

    /// Exponential backoff with "equal jitter": half the delay is fixed, half is random,
    /// so many clients failing at once don't all retry at the same instant.
    std::chrono::milliseconds backoffDelay(int attempt, int baseMs)
    {
        thread_local std::mt19937 rng{std::random_device{}()};
        int expMs = baseMs * (1 << attempt);
        std::uniform_int_distribution<int> jitter(0, expMs / 2);
        return std::chrono::milliseconds(expMs / 2 + jitter(rng));
    }
}

ApiClient& ApiClient::instance()
{
    static ApiClient instance;
    return instance;
}

ApiClient::ApiClient()
{
    ConfigData config = Configuration::instance().getCopy();
    LOG_INFO << "[ApiClient] Starting with base URL: " << config.EXTERNAL_API.BASEURL;
    client_ = drogon::HttpClient::newHttpClient(config.EXTERNAL_API.BASEURL);
}

namespace
{
    drogon::HttpRequestPtr makePut(const std::string& path, const Json::Value& body)
    {
        drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpJsonRequest(body);
        request->setMethod(drogon::Put);
        request->setPath(path);
        return request;
    }

    drogon::HttpRequestPtr makeGet(const std::string& path)
    {
        drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
        request->setMethod(drogon::Get);
        request->setPath(path);
        return request;
    }
}

void ApiClient::put(std::string path, Json::Value body, std::string traceId)
{
    LOG_INFO << "[" << traceId << "] [ApiClient] PUT " << path;
    sendInBackground(makePut(path, body), std::move(traceId));
}

void ApiClient::get(std::string path, std::string traceId)
{
    LOG_INFO << "[" << traceId << "] [ApiClient] GET " << path;
    sendInBackground(makeGet(path), std::move(traceId));
}

drogon::Task<drogon::HttpResponsePtr> ApiClient::putCoro(std::string path,
                                                         Json::Value body,
                                                         std::string traceId)
{
    LOG_INFO << "[" << traceId << "] [ApiClient] PUT " << path;
    co_return co_await sendWithRetry(makePut(path, body), std::move(traceId));
}

drogon::Task<drogon::HttpResponsePtr> ApiClient::getCoro(std::string path, std::string traceId)
{
    LOG_INFO << "[" << traceId << "] [ApiClient] GET " << path;
    co_return co_await sendWithRetry(makeGet(path), std::move(traceId));
}

void ApiClient::sendInBackground(drogon::HttpRequestPtr request, std::string traceId)
{
    // async_run keeps the lambda (and its captures) alive until the coroutine finishes.
    drogon::async_run(
        [this, request = std::move(request), traceId = std::move(traceId)]() -> drogon::Task<>
        {
            try
            {
                drogon::HttpResponsePtr response = co_await sendWithRetry(request, traceId);
                if (response->getStatusCode() < 300)
                {
                    LOG_INFO << "[" << traceId << "] [ApiClient] Success for "
                             << request->getPath();
                }
                else
                {
                    LOG_WARN << "[" << traceId << "] [ApiClient] HTTP "
                             << response->getStatusCode() << " for " << request->getPath();
                }
            }
            catch (const std::exception& error)
            {
                LOG_ERROR << "[" << traceId << "] [ApiClient] Request failed for "
                          << request->getPath() << ": " << error.what();
            }
        });
}

drogon::Task<drogon::HttpResponsePtr> ApiClient::sendWithRetry(drogon::HttpRequestPtr request,
                                                               std::string traceId)
{
    const ConfigData config = Configuration::instance().getCopy();
    const int maxRetries    = config.EXTERNAL_API.MAX_RETRIES;

    for (int attempt = 0;; ++attempt)
    {
        std::string failure;
        try
        {
            drogon::HttpResponsePtr response =
                co_await client_->sendRequestCoro(request, config.EXTERNAL_API.TIMEOUT_SECS);

            // 2xx/3xx, or a 4xx that retrying won't fix: hand it back to the caller.
            // After the last retry, a 5xx is handed back too.
            if (response->getStatusCode() < 500 || attempt >= maxRetries)
            {
                co_return response;
            }
            failure = "HTTP " + std::to_string(response->getStatusCode());
        }
        catch (const drogon::HttpException& error)
        {
            if (!isRetryable(error.code()) || attempt >= maxRetries)
            {
                LOG_ERROR << "[" << traceId << "] [ApiClient] Giving up on "
                          << request->getPath() << " after " << attempt
                          << " retries: " << error.what();
                throw;
            }
            failure = error.what();
        }

        // co_await isn't allowed inside a catch block, so the wait happens out here.
        std::chrono::milliseconds delay = backoffDelay(attempt, config.EXTERNAL_API.BASE_BACKOFF_MS);
        LOG_WARN << "[" << traceId << "] [ApiClient] " << failure << " for "
                 << request->getPath() << "; retry " << (attempt + 1) << "/" << maxRetries
                 << " in " << delay.count() << " ms";
        co_await drogon::sleepCoro(client_->getLoop(), delay);
    }
}
