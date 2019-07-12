#!/bin/bash

echo "Building NS3 container..."
docker build sim/ -t qns

echo "Configuring networks..."
docker create -it --cap-add=NET_ADMIN --net rightnet --ip 192.168.100.2 --name sim qns sh
docker network connect leftnet sim --ip 192.168.0.2

echo "Starting the simulator"
docker start -i sim
