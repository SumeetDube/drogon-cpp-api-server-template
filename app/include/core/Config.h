#pragma once

#include <functional>
#include <shared_mutex>
#include <string>
#include <yaml-cpp/yaml.h>

/**
 * @struct ExternalApiConfig
 * @brief Example configuration for an upstream/external API the service calls.
 */
struct ExternalApiConfig
{
    std::string BASEURL;       ///< Base URL of the upstream service.
    int MAX_RETRIES = 3;       ///< Maximum retry attempts for transient failures.
    int BASE_BACKOFF_MS = 200; ///< Base delay (ms) for exponential backoff.
    int TIMEOUT_SECS = 5;      ///< Per-request timeout in seconds.
};

/**
 * @struct ConfigData
 * @brief Aggregate runtime configuration. Add your own sections here.
 */
struct ConfigData
{
    ExternalApiConfig EXTERNAL_API; ///< Upstream API config.
};

/**
 * @class Configuration
 * @brief Thread-safe singleton that loads YAML config and exposes a typed copy.
 */
class Configuration
{
public:
    /// @brief Returns the global Configuration instance.
    static Configuration& instance();

    /// @brief Loads configuration from the YAML file pointed to by $MAIN_CONFIG.
    void loadFromYAML();

    /// @brief Returns a thread-safe copy of the current configuration.
    ConfigData getCopy();

    /// @brief Replaces the entire configuration atomically.
    void update(const ConfigData& data);

    /// @brief Atomically mutates the configuration via a user-supplied function.
    void modify(std::function<void(ConfigData&)> fn);

private:
    Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    ConfigData config_;                 ///< Backing config data.
    mutable std::shared_mutex mutex_;   ///< Guards reads/writes; allows concurrent reads.
};
