#/bin/bash
ifconfig eth0 hw ether 02:51:55:49:43:00
ifconfig eth0 promisc

ifconfig eth0 hw ether 02:51:55:49:43:01
ifconfig eth1 promisc

route del default

iptables -A FORWARD -i eth0 -o eth1 -j DROP
iptables -A FORWARD -i eth1 -o eth0 -j DROP

./waf --run sim
