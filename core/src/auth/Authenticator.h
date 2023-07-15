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

    bool authenticate(const std::string& authKey);
    AuthenticationType getAuthenticationType(const std::string& authKey);
    bool isKeyFromMaintenance(const std::string& authKey);
    bool isKeyFromNode(const std::string& authKey);
    bool isKeyApi(const std::string& authKey);

private:
    AuthenticationType getAuthenticationTypeFromConfiguration(const std::string& authKey);
};