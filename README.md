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
image, and `rightnet` is connected to the server. The ns-3 simulation sits in
the middle and forwards packets between `leftnet` and `rightnet`.

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

You need to set up the networks in docker once, and then docker remembers them
forever.  These networks are required for running the simulator (as described
below). To set up `leftnet` and `rightnet`, run the following commands:

```bash
docker network create leftnet --subnet 192.168.0.0/24
docker network create rightnet --subnet 192.168.100.0/24
```

## Building your own QUIC docker image

The [endpoint](endpoint) directory contains the base Docker image for an
endpoint Docker container.  The pre-built image of this container is available
on
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
    #!/bin/bash
    
    # Set up the routing needed for the simulation
    /setup.sh

    ROLE=$1
    shift

    if [ "$ROLE" == "client" ]; then
        [ INSERT COMMAND TO RUN YOUR QUIC CLIENT ]
    elif [ "$ROLE" == "server" ]; then
        [ INSERT COMMAND TO RUN YOUR QUIC SERVER ]
    fi
    ```

1. From inside the directory, build your image and assign a tag. For example,
   "my_quic_implementation":

   ```
   docker build . -t my_quic_implementation
   ```

   You will need to run this build command any time you change your
   implementation or either of the two files above.

For an example, have a look at the [quic-go
setup](https://github.com/marten-seemann/quic-go-docker) or the [quicly
setup](https://github.com/h2o/h2o-qns).


## Running a Simulation

1. You will want to run the simulator with a scenario first, so that the network
   scenario is set up before you run your client and server. The scenarios that
   are currently provided are listed below:
   
   * [Simple point-to-point link, with configurable link properties](sim/scenarios/simple-p2p)

   * [Single TCP connection running over a configurable point-to-point link](sim/scenarios/tcp-cross-traffic)


    The provided `run_sim.sh` script builds the simulator Docker container and runs
    the specified scenario. Build and run a scenario as follows:

    ```bash
    ./run_sim.sh "[scenario name] [scenario-specific parameters]"
    ```

    For example, the following command would run a simple point-to-point scenario:
    ```bash
    ./run_sim.sh "simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25"
    ```

1. With a network scenario running, you'll want to run the server next.  A
   convenience script is provided for this purpose. You can pass along any
   parameters to your `run_endpoint.sh` script (which in turn can pass them
   along to your server implementation):

   ```bash
   ./run_server.sh my_quic_implementation [params]
   ```

1. And then the client. A convenience script is provided for this purpose. You
   can pass along any parameters to your `run_endpoint.sh` script (which in turn
   can pass them along to your client implementation):

   ```bash
   ./run_client.sh my_quic_implementation [params]
   ```

## Debugging and FAQs

1. With the server running, you can get a root shell in the server docker
   container using the following (similarly for the client):

   ```bash
   docker exec -it server /bin/bash
   ```
