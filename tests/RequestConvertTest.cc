#include <gtest/gtest.h>

#include <drogon/drogon.h>
#include <typeinfo>

#include "controllers/models/request/CreateItemRequest.h"
#include "controllers/serialization/RequestConvert.h"
#include "custom_exceptions/ControllerError.h"

namespace
{
    drogon::HttpRequestPtr requestWithBody(const std::string& body)
    {
        drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
        request->setMethod(drogon::Post);
        request->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        request->setBody(body);
        return request;
    }

    /// Converts the body and returns the ControllerError it throws (fails the test if none).
    ControllerError conversionError(const std::string& body)
    {
        try
        {
            drogon::convertAndValidate<CreateItemRequest>(*requestWithBody(body));
        }
        catch (const ControllerError& error)
        {
            return error;
        }
        ADD_FAILURE() << "expected a ControllerError for body: " << body;
        return ControllerError("NONE", "NONE", "", "");
    }
}

TEST(RequestConvert, ValidBodyBindsToStruct)
{
    CreateItemRequest item = drogon::convertAndValidate<CreateItemRequest>(
        *requestWithBody(R"({"name":"Widget","quantity":3,"trace_id":"req-001"})"));

    EXPECT_EQ(item.name, "Widget");
    EXPECT_EQ(item.quantity, 3);
    EXPECT_EQ(item.trace_id, "req-001");
}

TEST(RequestConvert, MissingFieldIs422AndNamesTheField)
{
    ControllerError error = conversionError(R"({"name":"Widget","trace_id":"req-001"})");

    EXPECT_EQ(error.statusCode, drogon::k422UnprocessableEntity);
    EXPECT_EQ(error.errorCode, "VALIDATION_FAILED");
    EXPECT_NE(error.errorMessage.find("quantity"), std::string::npos) << error.errorMessage;
}

TEST(RequestConvert, MalformedJsonIs422)
{
    ControllerError error = conversionError("{not json");

    EXPECT_EQ(error.statusCode, drogon::k422UnprocessableEntity);
}

TEST(RequestConvert, ErrorDoesNotExposeCppTypeName)
{
    ControllerError error = conversionError("{}");

    EXPECT_EQ(error.errorMessage.find(typeid(CreateItemRequest).name()), std::string::npos)
        << error.errorMessage;
}
