#include "auth/Authenticator.h"
#include "gtest/gtest.h"

#define AUTH_CLUSTER_KEY_VALUE "abc"
#define AUTH_USER_KEY_VALUE "123"

configuration_t createConfiguration();

TEST(Authenticator, isAuthKeyFromClusterAndIsAuthKeyFromUser) {
    Authenticator authenticator{createConfiguration()};

    ASSERT_TRUE(authenticator.isAuthKeyFromCluster(AUTH_CLUSTER_KEY_VALUE));
    ASSERT_FALSE(authenticator.isAuthKeyFromCluster(AUTH_USER_KEY_VALUE));

    ASSERT_FALSE(authenticator.isAuthKeyFromUser(AUTH_CLUSTER_KEY_VALUE));
    ASSERT_TRUE(authenticator.isAuthKeyFromUser(AUTH_USER_KEY_VALUE));
}

TEST(Authenticator, getAuthenticationType) {
    Authenticator authenticator{createConfiguration()};

    ASSERT_EQ(authenticator.getAuthenticationType(AUTH_CLUSTER_KEY_VALUE), AuthenticationType::CLUSTER);
    ASSERT_EQ(authenticator.getAuthenticationType(AUTH_USER_KEY_VALUE), AuthenticationType::USER);
    ASSERT_EQ(authenticator.getAuthenticationType(AUTH_CLUSTER_KEY_VALUE), AuthenticationType::CLUSTER); //Two times, it gets cached
    ASSERT_EQ(authenticator.getAuthenticationType(AUTH_USER_KEY_VALUE), AuthenticationType::USER);
}

TEST(Authenticator, authenticate) {
    Authenticator authenticator{createConfiguration()};

    ASSERT_TRUE(authenticator.authenticate(AUTH_CLUSTER_KEY_VALUE));
    ASSERT_TRUE(authenticator.authenticate(AUTH_USER_KEY_VALUE));
    ASSERT_FALSE(authenticator.authenticate("caca"));
}

configuration_t createConfiguration() {
    std::map<std::string, std::string> configValues = {
            {ConfigurationKeys::AUTH_CLUSTER_KEY, AUTH_CLUSTER_KEY_VALUE},
            {ConfigurationKeys::AUTH_USER_KEY, AUTH_USER_KEY_VALUE},
    };

    return std::make_shared<Configuration>(configValues, configValues);
}