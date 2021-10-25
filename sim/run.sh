#!/bin/bash

set -e

# We are using eth0 and eth1 as EmuFdNetDevices in ns3.
# Use promiscuous mode to allow ns3 to capture all packets.
ifconfig eth0 promisc
ifconfig eth1 promisc

# A packet arriving at eth0 destined to 10.100.0.0/16 could be routed directly to eth1,
# and a packet arriving at eth1 destined to 10.0.0.0/16 directly to eth0.
# This would allow packets to skip the ns3 simulator altogether.
# Drop those to make sure they actually take the path through ns3.
if iptables -L 2> /dev/null | grep FORWARD > /dev/null; then
    echo "Using iptables"
    iptables -A FORWARD -i eth0 -o eth1 -j DROP
    iptables -A FORWARD -i eth1 -o eth0 -j DROP
    ip6tables -A FORWARD -i eth0 -o eth1 -j DROP
    ip6tables -A FORWARD -i eth1 -o eth0 -j DROP
elif grep nf_tables /proc/modules > /dev/null ; then
    echo "Using nftables"
    nft add table ip filter
    nft add chain ip filter FORWARD
    nft add rule ip filter FORWARD iifname "eth0" oifname "eth1" counter drop
    nft add rule ip filter FORWARD iifname "eth1" oifname "eth0" counter drop
else
    echo "Neither ip_tables nor nf_tables module is loaded. Skipping firewall configuration."
fi

if [[ -n "$WAITFORSERVER" ]]; then
    wait-for-it-quic -t 10s "${WAITFORSERVER}"
fi

echo "Using scenario: ${SCENARIO}"

eval ./scratch/"$SCENARIO &"

PID=$(jobs -p)
trap "kill -SIGINT ${PID}" INT
trap "kill -SIGTERM ${PID}" TERM
trap "kill -SIGKILL ${PID}" KILL
wait
