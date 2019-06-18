#!/bin/bash

route add -net 10.0.0.0 netmask 255.0.0.0 gw $GATEWAY
route del -net 10.0.0.0 netmask 255.255.0.0

sleep 3600
