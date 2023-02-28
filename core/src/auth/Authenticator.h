#pragma once

#include <memory>
#include <map>
#include <stdexcept>

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "AuthenticationType.h"

class Authenticator {
private:
    std::map<std::string, AuthenticationType> cachedAuthenticationTypes;
    configuration_t configuartion;

public:
    Authenticator() = default;

    Authenticator(configuration_t configuartion): configuartion(configuartion) {}

    bool authenticate(const std::string& authKey) {
        return this->configuartion->get(ConfigurationKeys::AUTH_USER_KEY).compare(authKey) == 0 ||
                this->configuartion->get(ConfigurationKeys::AUTH_CLUSTER_KEY).compare(authKey) == 0;
    }

    AuthenticationType getAuthenticationType(const std::string& authKey) {
        bool containedInCache = this->cachedAuthenticationTypes.contains(authKey);
        if(containedInCache)
            return this->cachedAuthenticationTypes.at(authKey);

        AuthenticationType authTypeFromConfig = this->getAuthenticationTypeFromConfiguration(authKey);
        this->cachedAuthenticationTypes[authKey] = authTypeFromConfig;

        return authTypeFromConfig;
    }

    bool isAuthKeyFromCluster(const std::string& authKey) {
        return this->configuartion->get(ConfigurationKeys::AUTH_CLUSTER_KEY).compare(authKey) == 0;
    }

    bool isAuthKeyFromUser(const std::string& authKey) {
        return this->configuartion->get(ConfigurationKeys::AUTH_USER_KEY).compare(authKey) == 0;
    }

private:
    AuthenticationType getAuthenticationTypeFromConfiguration(const std::string& authKey) {
        if(this->configuartion->get(ConfigurationKeys::AUTH_USER_KEY).compare(authKey) == 0) {
            return AuthenticationType::USER;
        }else if (this->configuartion->get(ConfigurationKeys::AUTH_CLUSTER_KEY).compare(authKey) == 0) {
            return AuthenticationType::CLUSTER;
        }

        throw std::runtime_error("Invalid auth. Before calling this method, the auth key should be validated using authenticate() method");
    }
};