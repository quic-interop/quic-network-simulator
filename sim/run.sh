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
iptables -A FORWARD -i eth0 -o eth1 -j DROP
iptables -A FORWARD -i eth1 -o eth0 -j DROP
ip6tables -A FORWARD -i eth0 -o eth1 -j DROP
ip6tables -A FORWARD -i eth1 -o eth0 -j DROP

if [[ -n "$WAITFORSERVER" ]]; then
  wait-for-it-quic -t 10s $WAITFORSERVER
fi

echo "Using scenario:" $SCENARIO

dumpcap -i eth0 -s 0 -w "/logs/trace_node_left.pcap" &
dumpcap -i eth1 -s 0 -w "/logs/trace_node_right.pcap" &
eval ./scratch/"$SCENARIO &"

PID=`jobs -p | tr '\n' ' '`

# On Linux, dumpcap captures through a TPACKET_V3 ring buffer. The kernel hands
# packets to dumpcap only in blocks, when a block is full or when its timeout
# (250 ms for dumpcap) expires. Packets in the block that is still open when
# dumpcap stops are never written to the pcap, so stopping the capture right
# away loses the last few hundred milliseconds of traffic. Wait for that block
# to be handed over before stopping the capture.
# Then wait for dumpcap to finish writing the pcaps: once this script (PID 1)
# exits, all processes left in the container are killed.
stop() {
  sleep 0.5
  kill -"$1" $PID || true
  wait
}
trap "stop SIGINT" INT
trap "stop SIGTERM" TERM
trap "kill -SIGKILL $PID" KILL
wait
