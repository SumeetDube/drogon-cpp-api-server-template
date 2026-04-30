#include "core/Advice.h"

#include <chrono>
#include <drogon/drogon.h>
#include <string>

void advice::preRoutingAdvice(const drogon::HttpRequestPtr& request)
{
    auto startTime = std::chrono::steady_clock::now();
    request->attributes()->insert("start_time", startTime);

    LOG_INFO << "INCOMING REQUEST"
             << " | From: "   << request->peerAddr().toIpPort()
             << " | Method: " << request->getMethodString()
             << " | Path: "   << request->getPath()
             << " | Query: "  << request->getQuery();
}

void advice::postHandlingAdvice(const drogon::HttpRequestPtr& request,
                                const drogon::HttpResponsePtr& response)
{
    using namespace std::chrono;

    double durationMs = -1.0;
    try
    {
        auto startTime = request->attributes()->get<steady_clock::time_point>("start_time");
        uint64_t duration = duration_cast<microseconds>(steady_clock::now() - startTime).count();
        durationMs = duration / 1000.0;
        response->addHeader("X-Process-Time-ms", std::to_string(durationMs));
    }
    catch (const std::exception&)
    {
    }

    response->addHeader("X-Request-Path", request->getPath());
    response->addHeader("X-Client-IP", request->peerAddr().toIpPort());
    response->addHeader("X-Response-Size", std::to_string(response->getBody().size()));
    response->addHeader("X-Server-Timestamp",
                        std::to_string(trantor::Date::now().secondsSinceEpoch()));

    LOG_INFO << "OUTGOING RESPONSE"
             << " | To: "       << request->peerAddr().toIpPort()
             << " | Method: "   << request->getMethodString()
             << " | Path: "     << request->getPath()
             << " | Status: "   << response->getStatusCode()
             << " | Size: "     << response->getBody().size() << " bytes"
             << " | Duration: " << (durationMs >= 0.0
                                        ? std::to_string(durationMs) + " ms"
                                        : "N/A");
}

void advice::beginningAdvice()
{
    for (const trantor::InetAddress& addr : drogon::app().getListeners())
    {
        LOG_INFO << "Server listening on: " << addr.toIpPort();
    }
}
