package endpoints

import (
	"clustermanager/src/config"
	"clustermanager/src/logging"
	"github.com/golang-jwt/jwt/v4"
	"github.com/labstack/echo/v4"
	"net/http"
	"time"
)

type LoginController struct {
	Configuration *config.Configuartion
	Logger        *logging.Logger
}

func CreateLoginController(configuration *config.Configuartion, logger *logging.Logger) *LoginController {
	return &LoginController{
		Configuration: configuration,
		Logger:        logger,
	}
}

type LoginRequest struct {
	AuthKey string `json:"authKey"`
}

func (controller *LoginController) Login(c echo.Context) error {
	loginRequest := new(LoginRequest)
	if err := c.Bind(loginRequest); err != nil {
		return err
	}

	if loginRequest.AuthKey != controller.Configuration.Get(config.AUTH_API_USER_KEY) {
		return c.JSON(http.StatusForbidden, "Invalid auth key")
	}

	unsingedToken := jwt.NewWithClaims(jwt.SigningMethodHS256, &jwt.RegisteredClaims{
		ExpiresAt: jwt.NewNumericDate(time.Now().Add(time.Minute * 5)),
	})

	signedToken, err := unsingedToken.SignedString(
		[]byte(controller.Configuration.Get(config.API_JWT_SECRET_KEY)))

	if err != nil {
		return err
	}

	controller.Logger.Info("Executed LoginController for authkey")

	return c.JSON(http.StatusOK, echo.Map{
		"token": signedToken,
	})
}
