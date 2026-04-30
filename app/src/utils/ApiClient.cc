#include "utils/ApiClient.h"

#include "core/Config.h"

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

void ApiClient::post(const std::string& path,
                     const Json::Value& body,
                     const std::string& traceId)
{
    drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpJsonRequest(body);
    request->setMethod(drogon::Post);
    request->setPath(path);

    LOG_INFO << "[" << traceId << "] [ApiClient] POST " << path;

    sendWithRetry(request, 0, traceId);
}

void ApiClient::get(const std::string& path, const std::string& traceId)
{
    drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);
    request->setPath(path);

    LOG_INFO << "[" << traceId << "] [ApiClient] GET " << path;

    sendWithRetry(request, 0, traceId);
}

void ApiClient::sendWithRetry(const drogon::HttpRequestPtr& request,
                              int attempt,
                              const std::string& traceId)
{
    ConfigData config = Configuration::instance().getCopy();

    client_->sendRequest(
        request,
        [this, request, attempt, traceId]
        (drogon::ReqResult result, const drogon::HttpResponsePtr& response)
        {
            if (result == drogon::ReqResult::Ok &&
                response &&
                response->getStatusCode() == drogon::k200OK)
            {
                LOG_INFO << "[" << traceId << "] [ApiClient] Success for "
                         << request->getPath();
                return;
            }

            ConfigData config = Configuration::instance().getCopy();
            bool isRetryable = false;

            if (result == drogon::ReqResult::Timeout ||
                result == drogon::ReqResult::NetworkFailure)
            {
                isRetryable = true;
            }
            else if (result == drogon::ReqResult::Ok && response)
            {
                if (response->getStatusCode() >= 500)
                {
                    isRetryable = true;
                }
                else
                {
                    LOG_WARN << "[" << traceId << "] [ApiClient] Non-retryable HTTP "
                             << response->getStatusCode() << " for " << request->getPath();
                    return;
                }
            }

            if (!isRetryable)
            {
                LOG_ERROR << "[" << traceId << "] [ApiClient] Non-retryable failure for "
                          << request->getPath()
                          << ", result=" << static_cast<int>(result);
                return;
            }

            if (attempt >= config.EXTERNAL_API.MAX_RETRIES)
            {
                LOG_ERROR << "[" << traceId << "] [ApiClient] Final failure for "
                          << request->getPath()
                          << " after " << attempt << " retries.";
                return;
            }

            int backoffMs = config.EXTERNAL_API.BASE_BACKOFF_MS * (1 << attempt);

            LOG_WARN << "[" << traceId << "] [ApiClient] Retry " << (attempt + 1)
                     << " for " << request->getPath()
                     << " in " << backoffMs << " ms";

            drogon::app().getLoop()->runAfter(
                std::chrono::milliseconds(backoffMs),
                [this, request, attempt, traceId]()
                {
                    sendWithRetry(request, attempt + 1, traceId);
                });
        },
        config.EXTERNAL_API.TIMEOUT_SECS);
}
