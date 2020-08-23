#!/bin/bash

set -e

echo "Setting up routes..."
# By default, docker containers don't compute UDP / TCP checksums.
# When packets run through ns3 however, the receiving endpoint requires valid checksums.
# This command makes sure that the endpoints set the checksum on outgoing packets.
# ethtool -K eth0 tx off

IPS=(`hostname -I`)
IP4=${IPS[0]}
IP6=${IPS[1]}
echo "eth0 IPv4: $IP4"
echo "eth0 IPv6: $IP6"

GATEWAY_V4="${IP4%.*}.2"
echo "Gateway v4: $GATEWAY_V4"
route add -net 193.167.0.0 netmask 255.255.0.0 gw $GATEWAY_V4

GATEWAY_V6="${IP6%:*}:2"
echo "Gateway v6: $GATEWAY_V6"
route -A inet6 add fd00::/96 gw $GATEWAY_V6 dev eth0

# delete unused route
UNNEEDED_ROUTE_V4="${IP4%.*}.0"
echo "Unneeded route V4: $UNNEEDED_ROUTE_V4"
route del -net $UNNEEDED_ROUTE_V4 netmask 255.255.255.0

# create the /logs and the /logs/qlog directory
mkdir -p /logs/qlog
