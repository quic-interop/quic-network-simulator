# Network Simulator for QUIC benchmarking

This project builds a test framework that can be used for benchmarking and
measuring the performance of QUIC implementations under various network
conditions. It uses the [ns-3](https://www.nsnam.org/) network simulator for
simulating network conditions and cross-traffic, and for bridging the real world
with the simulated world. It uses docker for isolating and coercing traffic
between the client and server to flow through the simulated network.

## Framework

The framework uses three docker images: the network simulator (as found in the
[sim](sim) directory), and a client and a server (as found in the
[endpoint](endpoint) directory).

The framework uses two networks on the host machine: `leftnet` (192.168.0.0/24)
and `rightnet` (192.168.100.0/24). `leftnet` is connected to the client docker
image, and `rightnet` is connected to the server. The ns-3 simulation sits in the
middle and forwards packets between `leftnet` and `rightnet`.

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

## Setting up the networks

To set up `leftnet` and `rightnet`, run the following commands:

```bash
docker network create leftnet --subnet 192.168.0.0/24
docker network create rightnet --subnet 192.168.100.0/24
```

The networks can be removed by executing:

```bash
docker network rm leftnet rightnet
```

## Running the simulator

To run the simulator, you need to set up the networks first, as described above.
After that, build and run the simulator:

```bash
./run.sh "simple-p2p --delay=10ms --bandwidth=10Mbps"
```

All paramters to `run.sh` are passed to waf, i.e. the command run inside the
container will be `./waf --run "simple-p2p --delay=10ms --bandwidth=10Mbps"`.

## Building your own QUIC endpoint

The [endpoint](endpoint) directory contains the base docker image for an
endpoint container.  The pre-built image is available on
[dockerhub](https://hub.docker.com/r/martenseemann/quic-network-simulator-endpoint).

Follow these steps to build a Docker image for your own QUIC implementation:

1. Create a new directory for your implementation. You will create two files in
   this directory: `Dockerfile` and `run_endpoint.sh`, as described below.

1.  Copy the Dockerfile below and add the commands to build your QUIC
    implementation.

    ```dockerfile
    FROM martenseemann/quic-network-simulator-endpoint:latest

    # download and build your QUIC implementation
    # [ DO WORK HERE ]

    # copy run script and run it
    COPY run_endpoint.sh .
    RUN chmod +x run_endpoint.sh
    ENTRYPOINT [ "./run_endpoint.sh" ]
    ```

    1. Now, copy the script below into `run_endpoint.sh`, and add commands as instructed.

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

1. From inside the directory, build your image and assign a tag. For example, "my_quic_implementation":

   ```
   docker build . -t my_quic_implementation
   ```

1. Typically, you'll want to run the server first:

   ```bash
   docker run --cap-add=NET_ADMIN --network rightnet --hostname server --ip 192.168.100.100 -it --entrypoint /bin/bash my_quic_implementation
   ```

1. And then the client:
   ```bash
   docker run --cap-add=NET_ADMIN --network leftnet --hostname client --ip 192.168.0.100 -it --entrypoint /bin/bash my_quic_implementation
   ```

For an example, have a look at the [quic-go Docker image](https://github.com/marten-seemann/quic-go-docker).
