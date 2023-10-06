#include "auth/Authenticator.h"

bool Authenticator::authenticate(const std::string& authKey) {
    return this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY) == authKey ||
               this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY) == authKey ||
               this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY) == authKey;
}

AuthenticationType Authenticator::getAuthenticationType(const std::string& authKey)  {
    return this->getAuthenticationTypeFromConfiguration(authKey);
}

bool Authenticator::isKeyFromMaintenance(const std::string& authKey) {
    return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::MAINTENANCE;  
}

bool Authenticator::isKeyFromNode(const std::string& authKey) {
    return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::NODE;
}

bool Authenticator::isKeyApi(const std::string& authKey) {
    return this->getAuthenticationTypeFromConfiguration(authKey) == AuthenticationType::API;
}

AuthenticationType Authenticator::getAuthenticationTypeFromConfiguration(const std::string& authKey) {
    if(this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY) == authKey) {
        return AuthenticationType::API;
    }else if (this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY) == authKey) {
        return AuthenticationType::MAINTENANCE;
    }else if (this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY) == authKey) {
        return AuthenticationType::NODE;
    }

    throw std::runtime_error("Invalid authentication");
}

