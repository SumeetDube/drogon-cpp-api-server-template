#include <drogon/drogon.h>

#include "core/Advice.h"
#include "core/Config.h"
#include "core/ExceptionHandler.h"
#include "utils/SignalHandler.h"

const std::string SOFTWARE_VERSION = "v0.1.0";

int main()
{
    signalutils::registerSignals();

    LOG_INFO << "Initializing API Service " << SOFTWARE_VERSION;

    Configuration::instance().loadFromYAML();

    drogon::app()
        .loadConfigFile("/tmp/drogon_config.yaml")
        .setExceptionHandler(exceptionHandler)
        .registerPreRoutingAdvice(advice::preRoutingAdvice)
        .registerPostHandlingAdvice(advice::postHandlingAdvice)
        .registerBeginningAdvice(advice::beginningAdvice);

    signalutils::attachDrogonSignalWatcher();

    drogon::app().run();

    LOG_INFO << "Service shutdown complete";
    return 0;
}
