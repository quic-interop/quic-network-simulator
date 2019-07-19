# Network Simulator for QUIC benchmarking

The build script builds the network simulator (as found in the [sim](sim) directory) and a client and a server (as found in the [endpoint](endpoint) directory).

## Network topology

The build script creates two networks, `leftnet` (192.168.0.0/24) and `rightnet` (192.168.100.0/24). Leftnet is connected to the client, and rightnet is connected to the server. The ns3 simulation sits in the middle and forwards packets from leftnet to rightnet and vice versa, through the ns3 simulation.

```
                                      |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
                                      |                      sim                         |
                                      |                                                  |      
|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|     |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|‾‾‾‾‾‾‾‾‾‾‾‾‾|     |‾‾‾‾‾‾‾‾|     |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|     |‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾|
|     client    |     | docker-bridge |     eth0    |     |        |     |     eth1      | docker-bridge |     |      server     |
|               |-----|               |             |-----|  ns-3  |-----|               |               |-----|                 |
| 192.168.0.100 |     |  192.168.0.1  | 192.168.0.2 |     |        |     | 192.168.100.2 | 192.168.100.1 |     | 192.168.100.100 |
|_______________|     |_______________|_____________|     |________|     |_______________|_______________|     |_________________|
                                      |                                                  |
                                      |__________________________________________________|
```

## Running a simulation

### Setting up the networks

To set up `leftnet` and `rightnet`, run the following commands
```bash
docker network create leftnet --subnet 192.168.0.0/24
docker network create rightnet --subnet 192.168.100.0/24
```

The networks can be removed by executing
```bash
docker network rm leftnet rightnet
```

### Running the simulator

To run the simulator, you need to set up the networks first, as described above.
After that, build and run the simulator:
```bash
./run.sh "simple-p2p --delay=10ms --bandwidth=10Mbps"
```

All paramters to `run.sh` are passed to waf, i.e. the command run inside the container will be `./waf --run "simple-p2p --delay=10ms --bandwidth=10Mbps"`.

### Building your own QUIC endpoint

The [endpoint](endpoint) directory contains the base docker image for an endpoint container. 
The pre-built image is available on [dockerhub](https://hub.docker.com/r/martenseemann/quic-network-simulator-endpoint).

When building a Docker image for your own QUIC implementation, there are two files you need to create: `Dockerfile` and `run_endpoint.sh`.
Copy this Dockerfile and add the commands to build your QUIC implementation.
```docker
FROM martenseemann/quic-network-simulator-endpoint:latest

# download and build your QUIC implementation
# [ DO WORK HERE ]

# copy run script and run it
COPY run_endpoint.sh .
RUN chmod +x run_endpoint.sh
ENTRYPOINT [ "./run_endpoint.sh" ]
```

`run_endpoint.sh` could look like this:
```bash
# Set up the routing needed for the simulation.
/setup.sh

ROLE=$1
shift

if [ "$ROLE" == "client" ]; then
    [ INSERT COMMAND TO RUN YOUR QUIC CLIENT ]
elif [ "$ROLE" == "server" ]; then
    [ INSERT COMMAND TO RUN YOUR QUIC SERVER ]
fi
```

Build your image and assign a tag (`$YOURTAG`):
```
docker build . -t $YOURTAG
```

Run the client:
```bash
docker run --cap-add=NET_ADMIN --network leftnet --hostname client --ip 192.168.0.100 -it --entrypoint /bin/bash $YOURTAG
```

And the server:
```bash
docker run --cap-add=NET_ADMIN --network rightnet --hostname server --ip 192.168.100.100 -it --entrypoint /bin/bash $YOURTAG
```

For an example, have a look at the [quic-go Docker image](https://github.com/marten-seemann/quic-go-docker).
```bash
docker build . -t $YOURTAG
```
