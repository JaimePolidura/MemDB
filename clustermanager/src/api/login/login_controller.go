package login

import (
	configuration "clustermanager/src/_shared/config"
	configuration_keys "clustermanager/src/_shared/config/keys"
	"github.com/golang-jwt/jwt/v4"
	"github.com/labstack/echo/v4"
	"net/http"
	"time"
)

type LoginController struct {
	Configuration *configuration.Configuartion
}

type LoginRequest struct {
	AuthKey string `json:"authKey"`
}

func (controller *LoginController) Login(c echo.Context) error {
	loginRequest := new(LoginRequest)
	if err := c.Bind(loginRequest); err != nil {
		return err
	}
	if loginRequest.AuthKey != controller.Configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_AUTH_CLUSTER_KEY) {
		return c.JSON(http.StatusForbidden, "Invalid auth key")
	}

	unsingedToken := jwt.NewWithClaims(jwt.SigningMethodHS256, &jwt.RegisteredClaims{
		ExpiresAt: jwt.NewNumericDate(time.Now().Add(time.Minute * 5)),
	})
	
	signedToken, err := unsingedToken.SignedString(
		[]byte(controller.Configuration.Get(configuration_keys.MEMDB_CLUSTERMANAGER_API_SECRET_KEY)))

	if err != nil {
		return err
	}

	return c.JSON(http.StatusOK, echo.Map{
		"token": signedToken,
	})
}
