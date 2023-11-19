package utils

import (
	"net"
	"regexp"
	"strconv"
)

func GetTCPAddress(address string) (*net.TCPAddr, error) {
	if IsName(address) {
		return SingleResolveWithPort(address)
	} else {
		ipStr, portStr, _ := net.SplitHostPort(address)
		ip := net.ParseIP(ipStr)
		port, _ := strconv.Atoi(portStr)

		return &net.TCPAddr{IP: ip, Port: port, Zone: ""}, nil
	}
}

func IsName(name string) bool {
	ipv4Regex := regexp.MustCompile(`^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$`)

	return !ipv4Regex.MatchString(name)
}

func SingleResolveWithPort(name string) (*net.TCPAddr, error) {
	return net.ResolveTCPAddr("tcp", name)
}
