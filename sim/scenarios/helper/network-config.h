#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <string>
#include <cstdlib>
#include "ns3/core-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

class NetworkConfig {
public:
    // Singleton instance to ensure we read environment variables only once.
    static const NetworkConfig& Instance() {
        static NetworkConfig instance;
        return instance;
    }

    const std::string& GetClientV4Addr() const { return client_v4_addr_; }
    const std::string & GetServerV4Addr() const
    {
        return server_v4_addr_;
    }
    const std::string& GetClientV6Addr() const { return client_v6_addr_; }
    const std::string& GetServerV6Addr() const { return server_v6_addr_; }

    Ipv4Address GetClientV4Address() const {
        return Ipv4Address(client_v4_addr_.c_str());
    }

    Ipv4Address GetServerV4Address() const {
        return Ipv4Address(server_v4_addr_.c_str());
    }

    const std::string & GetClientGatewayV4Addr() const
    {
        return client_v4_gateway_;
    }

    const std::string & GetServerGatewayV4Addr() const
    {
        return server_v4_gateway_;
    }

    const std::string & GetClientGatewayV6Addr() const
    {
        return client_v6_gateway_;
    }

    const std::string & GetServerGatewayV6Addr() const
    {
        return server_v6_gateway_;
    }

    const std::string & GetLeftNetName() const
    {
        return leftnet_name_;
    }

    const std::string & GetRightNetName() const
    {
        return rightnet_name_;
    }

    std::string GetV4PointToPointNetwork() const {
        return subnet_v4_ + ".255.0";
    }

    std::string GetV4SubnetMask() const {
        return "255.255.255.0";
    }

    std::string GetV6PointToPointNetwork() const {
        return subnet_v6_ + ":255::";
    }

    int GetV6PrefixInt() const {
        return std::stoi(v6_prefix_);
    }

private:
    NetworkConfig() {
        // SUBNET_V4: Base IPv4 network prefix (e.g., "10.0")
        subnet_v4_ = GetEnvVar("SUBNET_V4");

        // CLIENT_V4_ADDR: Client IPv4 address (e.g., "10.0.10.10")
        client_v4_addr_ = GetEnvVar("CLIENT_V4_ADDR");

        // SERVER_V4_ADDR: Server IPv4 address (e.g., "10.0.222.222")
        server_v4_addr_ = GetEnvVar("SERVER_V4_ADDR");

        // CLIENT_V4_GATEWAY: Client gateway IPv4 address (e.g., "10.0.10.2")
        client_v4_gateway_ = GetEnvVar("CLIENT_V4_GATEWAY");

        // SERVER_V4_GATEWAY: Server gateway IPv4 address (e.g., "10.0.222.2")
        server_v4_gateway_ = GetEnvVar("SERVER_V4_GATEWAY");

        // SUBNET_V6: Base IPv6 network prefix (e.g., "fd00:cafe:0000")
        subnet_v6_ = GetEnvVar("SUBNET_V6");

        // V6_PREFIX: IPv6 prefix length (e.g., "64")
        v6_prefix_ = GetEnvVar("V6_PREFIX");

        // CLIENT_V6_ADDR: Client IPv6 address (e.g., "fd00:cafe:0000:10::10")
        client_v6_addr_ = GetEnvVar("CLIENT_V6_ADDR");

        // SERVER_V6_ADDR: Server IPv6 address (e.g., "fd00:cafe:0000:222::222")
        server_v6_addr_ = GetEnvVar("SERVER_V6_ADDR");

        // CLIENT_V6_GATEWAY: Client gateway IPv6 address (e.g.,
        // "fd00:cafe:0000:10::2")
        client_v6_gateway_ = GetEnvVar("CLIENT_V6_GATEWAY");

        // SERVER_V6_GATEWAY: Server gateway IPv6 address (e.g.,
        // "fd00:cafe:0000:222::2")
        server_v6_gateway_ = GetEnvVar("SERVER_V6_GATEWAY");

        // LEFTNET_NAME: Left network interface name (e.g., "eth0")
        leftnet_name_ = GetEnvVar("LEFTNET_NAME");

        // RIGHTNET_NAME: Right network interface name (e.g., "eth1")
        rightnet_name_ = GetEnvVar("RIGHTNET_NAME");

        // V4_PREFIX: IPv4 prefix length - must always be "24"
        std::string v4_prefix = GetEnvVar("V4_PREFIX");
        NS_ABORT_MSG_IF(v4_prefix != "24", "V4_PREFIX must be 24, but got: " << v4_prefix);
    }

    static std::string GetEnvVar(const std::string& var_name) {
        const char* value = std::getenv(var_name.c_str());
        NS_ABORT_MSG_IF(value == nullptr, "Environment variable " << var_name << " is not set.");
        return std::string(value);
    }

    std::string subnet_v4_;
    std::string client_v4_addr_;
    std::string server_v4_addr_;
    std::string client_v4_gateway_;
    std::string server_v4_gateway_;
    std::string subnet_v6_;
    std::string v6_prefix_;
    std::string client_v6_addr_;
    std::string server_v6_addr_;
    std::string client_v6_gateway_;
    std::string server_v6_gateway_;
    std::string leftnet_name_;
    std::string rightnet_name_;
};

} // namespace ns3

#endif // NETWORK_CONFIG_H
