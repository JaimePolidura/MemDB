#include "auth/Authenticator.h"

bool Authenticator::authenticate(const std::string& authKey) {
    return this->configuration->get(ConfigurationKeys::AUTH_API_USER_KEY) == authKey ||
           this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY) == authKey;
}

std::result<AuthenticationType> Authenticator::getAuthenticationType(const std::string& authKey)  {
    return this->getAuthenticationTypeFromConfiguration(authKey);
}

bool Authenticator::isKeyFromNode(const std::string& authKey) {
    std::result<AuthenticationType> result = this->getAuthenticationTypeFromConfiguration(authKey);
    return result.is_success() && result.get() == AuthenticationType::NODE;
}

bool Authenticator::isKeyUser(const std::string& authKey) {
    std::result<AuthenticationType> result = this->getAuthenticationTypeFromConfiguration(authKey);
    return result.is_success() && result.get() == AuthenticationType::USER;
}

std::result<AuthenticationType> Authenticator::getAuthenticationTypeFromConfiguration(const std::string& authKey) {
    if(this->configuration->get(ConfigurationKeys::AUTH_API_USER_KEY) == authKey) {
        return std::ok(AuthenticationType::USER);
    }else if (this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY) == authKey) {
        return std::ok(AuthenticationType::NODE);
    }

    return std::error<AuthenticationType>();
}


