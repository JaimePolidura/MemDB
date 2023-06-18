#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "auth/AuthenticationType.h"

class Authenticator {
private:
    configuration_t configuration;

public:
    Authenticator() = default;

    explicit Authenticator(configuration_t configuartion): configuration(configuartion) {}

    bool authenticate(const std::string& authKey) {
        return this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY) == authKey ||
               this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY) == authKey ||
               this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY) == authKey;
    }

    AuthenticationType getAuthenticationType(const std::string& authKey) {
        return this->getAuthenticationTypeFromConfiguration(authKey);
    }

    bool isKeyFromMaintenance(const std::string& authKey) {
        return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::MAINTENANCE;
    }

    bool isKeyFromNode(const std::string& authKey) {
        return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::NODE;
    }

    bool isKeyApi(const std::string& authKey) {
        return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::API;
    }

private:
    AuthenticationType getAuthenticationTypeFromConfiguration(const std::string& authKey) {
        if(this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY) == authKey) {
            return AuthenticationType::API;
        }else if (this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY) == authKey) {
            return AuthenticationType::MAINTENANCE;
        }else if (this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY) == authKey) {
            return AuthenticationType::NODE;
        }

        throw std::runtime_error("Invalid authentication");
    }
};