#!/bin/bash
set -o pipefail
set -e

IMPL=$1
shift

docker rm client &> /dev/null || true
docker run --cap-add=NET_ADMIN -v `pwd`/qnslogs:/qnslogs --network leftnet --hostname client --name client --ip 192.168.0.100 -it $IMPL client "$@"
