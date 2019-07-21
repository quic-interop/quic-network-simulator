#!/bin/bash
set -o pipefail
set -e

IMPL=$1
shift

docker rm server &> /dev/null || true
docker run --cap-add=NET_ADMIN --network rightnet --hostname server --name server --ip 192.168.100.100 -it $IMPL server "$@"
