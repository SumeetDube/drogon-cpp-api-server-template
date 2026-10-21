#pragma once

#include <drogon/drogon.h>

#include "controllers/models/request/CreateItemRequest.h"
#include "controllers/serialization/RequestConvert.h"

namespace api
{
  /**
   * @brief Example v1 HTTP controller. Add your own METHOD_ADD entries below.
   */
  class v1 : public drogon::HttpController<v1>
  {
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(v1::health, "/health", drogon::Get);
    METHOD_ADD(v1::ready, "/ready", drogon::Get);
    METHOD_ADD(v1::createItem, "/items", drogon::Post);
    METHOD_LIST_END

    /// Liveness probe — always 200. Use this for k8s livenessProbe.
    void health(const drogon::HttpRequestPtr& request,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    /// Readiness probe — pings the database asynchronously (coroutine). 503 if it's down.
    drogon::Task<drogon::HttpResponsePtr> ready(drogon::HttpRequestPtr request);

    /// Example POST endpoint. Demonstrates auto-bound JSON payload + validation.
    void createItem(const drogon::HttpRequestPtr& request,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    CreateItemRequest&& payload);
  };
}
