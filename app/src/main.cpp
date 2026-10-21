#include <drogon/drogon.h>

#include "core/Advice.h"
#include "core/Config.h"
#include "core/ExceptionHandler.h"

const std::string SOFTWARE_VERSION = "v0.1.0";

int main()
{
    LOG_INFO << "Initializing API Service " << SOFTWARE_VERSION;

    Configuration::instance().loadFromYAML();

    // Drogon installs its own SIGINT/SIGTERM handlers: on either signal it stops
    // accepting connections, finishes in-flight requests and returns from run().
    drogon::app()
        .loadConfigFile("/tmp/drogon_config.yaml")
        .setExceptionHandler(exceptionHandler)
        .registerPreRoutingAdvice(advice::preRoutingAdvice)
        .registerPostHandlingAdvice(advice::postHandlingAdvice)
        .registerBeginningAdvice(advice::beginningAdvice);

    drogon::app().run();

    LOG_INFO << "Service shutdown complete";
    return 0;
}
