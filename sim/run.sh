#!/bin/bash

# We are using eth0 and eth1 as EmuFdNetDevices in ns3.
# ns3 usually uses MAC address spoofing to separate ns3 from other traffic,
# see https://www.nsnam.org/docs/models/html/fd-net-device.html#emufdnetdevicehelper.
# We actually want all traffic to flow through ns3, so we manually set the MAC
# addresses, and use the configure the same values in the ns3 script,
# thereby effectively disabling the spoofing.
ifconfig eth0 hw ether 02:51:55:49:43:00
ifconfig eth0 promisc
ifconfig eth1 hw ether 02:51:55:49:43:01
ifconfig eth1 promisc

# A packet arriving at eth0 destined to 10.100.0.0/16 could be routed directly to eth1,
# and a packet arriving at eth1 destined to 10.0.0.0/16 directly to eth0.
# This would allow packets to skip the ns3 simulator altogether.
# Drop those to make sure they actually take the path through ns3.
iptables -A FORWARD -i eth0 -o eth1 -j DROP
iptables -A FORWARD -i eth1 -o eth0 -j DROP

echo "Using scenario:" $SCENARIO

eval ./scratch/"$SCENARIO &"

PID=`jobs -p`
trap "kill -SIGINT $PID" INT
trap "kill -SIGTERM $PID" TERM
trap "kill -SIGKILL $PID" KILL
wait
