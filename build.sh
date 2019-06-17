#!/bin/bash

echo "Cleaning up..."
docker kill sim client server
docker system prune --force

echo "Building NS3 container..."
cd sim
docker build . -t qns
cd ..

echo "Configuring networks..."
docker network create leftnet --subnet 10.0.0.0/16
docker network create rightnet --subnet 10.100.0.0/16
docker create -it --privileged --net rightnet --ip 10.100.0.2 --name sim qns sh
docker network connect leftnet sim --ip 10.0.0.2

cd endpoint
echo "Building endpoint container..."
docker build . -t endpoint
echo "Running server and client container..."
docker run -d --cap-add=NET_ADMIN --network leftnet --ip 10.0.0.100 -e GATEWAY="10.0.0.2" --name client endpoint
docker run -d --cap-add=NET_ADMIN --network rightnet --ip 10.100.0.100 -e GATEWAY="10.100.0.2" --name server endpoint
cd ..

echo "Starting NS3 container"
docker start -i sim
