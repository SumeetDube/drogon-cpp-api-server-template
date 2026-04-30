#include "core/Config.h"

#include <cstdlib>
#include <drogon/drogon.h>
#include <fstream>
#include <stdexcept>

Configuration& Configuration::instance()
{
    static Configuration instance;
    return instance;
}

namespace
{
    std::string mustGetEnv(const char* key)
    {
        const char* value = std::getenv(key);
        if (!value)
        {
            throw std::runtime_error(std::string("Missing required environment variable: ") + key);
        }
        return std::string(value);
    }
}

void Configuration::loadFromYAML()
{
    const char* configPath = std::getenv("MAIN_CONFIG");
    if (configPath == nullptr)
    {
        throw std::runtime_error("Environment variable MAIN_CONFIG is not set.");
    }

    LOG_INFO << "Loading configuration from: " << configPath;
    std::unique_lock lock(mutex_);

    YAML::Node root = YAML::LoadFile(configPath);
    if (!root)
    {
        throw std::runtime_error("Failed to load configuration: " + std::string(configPath));
    }

    YAML::Node serviceConfig = root["SERVICE"];
    if (!serviceConfig)
    {
        throw std::runtime_error("SERVICE section missing in YAML");
    }

    if (YAML::Node externalApi = serviceConfig["EXTERNAL_API"]; externalApi)
    {
        config_.EXTERNAL_API.BASEURL         = externalApi["BASEURL"].as<std::string>("");
        config_.EXTERNAL_API.MAX_RETRIES     = externalApi["MAX_RETRIES"].as<int>(3);
        config_.EXTERNAL_API.BASE_BACKOFF_MS = externalApi["BASE_BACKOFF_MS"].as<int>(200);
        config_.EXTERNAL_API.TIMEOUT_SECS    = externalApi["TIMEOUT_SECS"].as<int>(5);
    }

    YAML::Node drogonConfig = serviceConfig["DROGON_CONFIG"];
    if (!drogonConfig)
    {
        throw std::runtime_error("DROGON_CONFIG section missing in YAML");
    }

    YAML::Node dbClients = drogonConfig["db_clients"];
    if (dbClients && dbClients.IsSequence())
    {
        for (std::size_t i = 0; i < dbClients.size(); ++i)
        {
            YAML::Node clientNode = dbClients[i];
            if (clientNode["name"] && clientNode["name"].as<std::string>() == "default")
            {
                clientNode["host"]   = mustGetEnv("POSTGRESQL_HOST");
                clientNode["port"]   = std::stoi(mustGetEnv("POSTGRESQL_PORT"));
                clientNode["dbname"] = mustGetEnv("POSTGRESQL_DB_NAME");
                clientNode["user"]   = mustGetEnv("POSTGRESQL_USER");
                clientNode["passwd"] = mustGetEnv("POSTGRESQL_PASSWORD");
                break;
            }
        }
    }

    YAML::Emitter out;
    out << drogonConfig;

    std::ofstream fout("/tmp/drogon_config.yaml");
    if (!fout)
    {
        throw std::runtime_error("Failed to write /tmp/drogon_config.yaml");
    }
    fout << out.c_str();
    fout.close();

    LOG_INFO << "Configuration successfully loaded";
}

ConfigData Configuration::getCopy()
{
    std::shared_lock lock(mutex_);
    return config_;
}

void Configuration::update(const ConfigData& data)
{
    std::unique_lock lock(mutex_);
    config_ = data;
}

void Configuration::modify(std::function<void(ConfigData&)> fn)
{
    std::unique_lock lock(mutex_);
    fn(config_);
}
