#!/bin/bash

echo "Setting up routes..."
# By default, docker containers don't compute UDP / TCP checksums.
# When packets run through ns3 however, the receiving endpoint requires valid checksums.
# This command makes sure that the endpoints set the checksum on outgoing packets.
ethtool -K eth0 tx off

IP=`hostname -I`
GATEWAY="${IP%.*}.2"
UNNEEDED_ROUTE="${IP%.*}.0"

route add -net 193.167.0.0 netmask 255.255.0.0 gw $GATEWAY
# delete unused route
route del -net $UNNEEDED_ROUTE netmask 255.255.255.0

netstat -rn

# XXX this relies on the IPv6 address being second in the "hostname -I" output

IP=$(echo "$IP" | cut -f2 -d" ")
GATEWAY="${IP%:*}:2"
UNNEEDED_ROUTE="${IP%:*}:"

echo "IP $IP"
echo "GATEWAY $GATEWAY"
echo "UNNEEDED_ROUTE $UNNEEDED_ROUTE"

netstat -6rn

echo ip route add fd00:cafe:cafe::/48 via $GATEWAY
echo ip route del $UNNEEDED_ROUTE/64

ip route add fd00:cafe:cafe::/48 via $GATEWAY
# delete unused route
ip route del $UNNEEDED_ROUTE/64


netstat -6rn

# create the /logs and the /logs/qlog directory
mkdir -p /logs/qlog
