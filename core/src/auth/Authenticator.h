#pragma once

#include "shared.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "auth/AuthenticationType.h"
#include "utils/std/Result.h"

class Authenticator {
    configuration_t configuration;

public:
    Authenticator() = default;

    explicit Authenticator(configuration_t configuartion): configuration(configuartion) {}

    bool authenticate(const std::string& authKey);
    std::result<AuthenticationType> getAuthenticationType(const std::string& authKey);
    bool isKeyFromNode(const std::string& authKey);
    bool isKeyUser(const std::string& authKey);

private:
    std::result<AuthenticationType> getAuthenticationTypeFromConfiguration(const std::string& authKey);
};