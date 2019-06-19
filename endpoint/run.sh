#!/bin/bash

route add -net 10.0.0.0 netmask 255.0.0.0 gw $GATEWAY
# delete unused routes
route del -net 10.0.0.0 netmask 255.255.0.0
route del -net 10.100.0.0 netmask 255.255.0.0

sleep 3600
