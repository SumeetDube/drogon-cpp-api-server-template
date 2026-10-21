#include <gtest/gtest.h>

#include <drogon/drogon.h>
#include <stdexcept>

#include "core/ExceptionHandler.h"
#include "custom_exceptions/ControllerError.h"

namespace
{
    /// Runs the global exception handler on an exception and returns the response it produced.
    drogon::HttpResponsePtr handle(const std::exception& error)
    {
        drogon::HttpResponsePtr captured;
        exceptionHandler(error,
                         drogon::HttpRequest::newHttpRequest(),
                         [&](const drogon::HttpResponsePtr& response) { captured = response; });
        return captured;
    }
}

TEST(ExceptionHandler, ControllerErrorMapsToItsStatusAndEnvelope)
{
    ControllerError error("INVALID_NAME", "ERROR", "name must not be empty", "trace-123",
                          drogon::k422UnprocessableEntity);

    drogon::HttpResponsePtr response = handle(error);
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k422UnprocessableEntity);

    auto json = response->getJsonObject();
    ASSERT_NE(json, nullptr);
    EXPECT_FALSE((*json)["status"].asBool());
    EXPECT_EQ((*json)["trace_id"].asString(), "trace-123");
    EXPECT_EQ((*json)["details"]["error_code"].asString(), "INVALID_NAME");
    EXPECT_EQ((*json)["details"]["error_message"].asString(), "name must not be empty");
}

TEST(ExceptionHandler, UnexpectedErrorReturns500WithoutLeakingDetails)
{
    std::runtime_error error("connection to 10.0.0.5:5432 failed: password=hunter2");

    drogon::HttpResponsePtr response = handle(error);
    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->getStatusCode(), drogon::k500InternalServerError);

    auto json = response->getJsonObject();
    ASSERT_NE(json, nullptr);
    EXPECT_EQ((*json)["details"]["error_code"].asString(), "INTERNAL_SERVER_ERROR");
    EXPECT_FALSE((*json)["trace_id"].asString().empty());

    std::string body(response->getBody());
    EXPECT_EQ(body.find("hunter2"), std::string::npos);
    EXPECT_EQ(body.find("10.0.0.5"), std::string::npos);
}
