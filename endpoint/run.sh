#!/bin/bash

route del default
route add default gw $GATEWAY

sleep 3600
