#!/bin/bash

if [[ $# -eq 0 ]] ; then
    echo "expected scenario to run"
    exit 0
fi

# clean up the sim container (which might exist from a prior run)
docker rm sim &> /dev/null

echo "Building NS3 container..."
docker build sim/ -t qns

echo "Configuring networks..."
# Docker doesn't allow us to pass any arguments to docker start,
# as it would be possible using docker run.
# Modify the entrypoint to take take the arguments instead.
docker create --cap-add=NET_ADMIN --net rightnet --ip 192.168.100.2 --name sim -it --entrypoint "./run.sh" qns "$1"
docker network connect leftnet sim --ip 192.168.0.2

echo "Starting the simulator"
docker start -i sim
