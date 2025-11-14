#!/bin/bash

set -e

# We are using eth0 and eth1 as EmuFdNetDevices in ns3.
# Use promiscuous mode to allow ns3 to capture all packets.
ifconfig eth0 promisc
ifconfig eth1 promisc

# A packet arriving at eth0 could be routed directly to eth1 and vice versa.
# This would allow packets to skip the ns3 simulator altogether.
# Drop all forwarding between these interfaces to ensure packets go through ns3.
iptables -A FORWARD -i eth0 -o eth1 -j DROP
iptables -A FORWARD -i eth1 -o eth0 -j DROP
ip6tables -A FORWARD -i eth0 -o eth1 -j DROP
ip6tables -A FORWARD -i eth1 -o eth0 -j DROP

if [[ -n "$WAITFORSERVER" ]]; then
  wait-for-it-quic -t 10s "$WAITFORSERVER"
fi

echo "Using scenario: $SCENARIO"

tcpdump -i eth0 -U -w "/logs/trace_node_left.pcap" &
tcpdump -i eth1 -U -w "/logs/trace_node_right.pcap" &
eval ./scratch/"$SCENARIO &"

PID=$(jobs -p | tr '\n' ' ')
trap "kill -SIGINT $PID" INT
trap "kill -SIGTERM $PID" TERM
wait
