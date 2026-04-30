#pragma once

#include <drogon/drogon.h>
#include <drogon/HttpRequest.h>
#include <rfl/json.hpp>
#include <string>

#include "controllers/models/request/CreateItemRequest.h"
#include "custom_exceptions/ControllerError.h"

namespace drogon
{
    /**
     * @brief Parse the request body into a strongly-typed struct using reflect-cpp.
     *
     * Throws a ControllerError(422) on validation failure.
     */
    template <typename T>
    inline T convertAndValidate(const HttpRequest& request)
    {
        std::string jsonStr = std::string(request.body());
        auto result = rfl::json::read<T>(jsonStr);

        if (result)
        {
            return result.value();
        }

        std::string errorMsg = "JSON Validation Failed for " +
                               std::string(typeid(T).name()) + ": " +
                               result.error().what();

        LOG_ERROR << errorMsg;
        throw ControllerError("VALIDATION_FAILED",
                              "FATAL",
                              errorMsg,
                              drogon::utils::getUuid(),
                              drogon::HttpStatusCode::k422UnprocessableEntity);
    }

    /**
     * @brief Drogon auto-binding hook.
     *
     * When a controller method takes `CreateItemRequest&&` as its third
     * argument, Drogon calls this specialization to materialize it from
     * the request body. Add a similar specialization for every DTO you
     * want auto-bound.
     */
    template <>
    inline CreateItemRequest fromRequest(const HttpRequest& request)
    {
        return convertAndValidate<CreateItemRequest>(request);
    }
}
