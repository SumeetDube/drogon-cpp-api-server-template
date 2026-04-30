#pragma once

#include <drogon/drogon.h>
#include <exception>
#include <functional>

/**
 * @brief Global exception handler.
 *
 * Catches anything thrown from controllers OR from `fromRequest` specializations
 * during argument binding, and converts it to a structured FailedResponse.
 * Register once at startup via drogon::app().setExceptionHandler(...).
 */
void exceptionHandler(const std::exception& error,
                      const drogon::HttpRequestPtr& request,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);
