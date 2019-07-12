#!/bin/bash

echo "Setting up routes..."
# By default, docker containers don't compute UDP / TCP checksums.
# When packets run through ns3 however, the receiving endpoint requires valid checksums.
# This command makes sure that the endpoints set the checksum on outgoing packets.
ethtool -K eth0 tx off

IP=`hostname -I`
GATEWAY="${IP%.*}.2"

route add -net 192.168.0.0 netmask 255.255.0.0 gw $GATEWAY
# delete unused routes
route del -net 192.168.0.0 netmask 255.255.255.0
route del -net 192.168.100.0 netmask 255.255.255.0
