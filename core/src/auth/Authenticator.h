#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "auth/AuthenticationType.h"

class Authenticator {
private:
    configuration_t configuartion;

public:
    Authenticator() = default;

    Authenticator(configuration_t configuartion): configuartion(configuartion) {}

    bool authenticate(const std::string& authKey) {
        return this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY).compare(authKey) == 0 ||
                this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY).compare(authKey) == 0 ||
                this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY).compare(authKey);
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
        if(this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY).compare(authKey) == 0) {
            return AuthenticationType::API;
        }else if (this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY).compare(authKey) == 0) {
            return AuthenticationType::MAINTENANCE;
        }else if (this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY).compare(authKey) == 0) {
            return AuthenticationType::NODE;
        }

        throw std::runtime_error("Invalid authentication");
    }
};