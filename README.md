# Network Simulator for QUIC benchmarking

This project builds a test framework that can be used for benchmarking and
measuring the performance of QUIC implementations under various network
conditions. It uses the [ns-3](https://www.nsnam.org/) network simulator for
simulating network conditions and cross-traffic, and for bridging the real world
with the simulated world. It uses docker for isolating and coercing traffic
between the client and server to flow through the simulated network.

## Framework

The framework uses docker-compose to compose three docker images: the network
simulator (as found in the [sim](sim) directory), and a client and a server (as
found in the individual QUIC implementation directories, or for a simple shell,
the [endpoint](endpoint) directory).

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


## Building your own QUIC docker image

The [endpoint](endpoint) directory contains the base Docker image for an
endpoint Docker container.  The pre-built image of this container is available
on
[dockerhub](https://hub.docker.com/r/martenseemann/quic-network-simulator-endpoint).

Follow these steps to set up your own QUIC implementation:

1. Create a new directory for your implementation (say, my_impl_dir). You will
   create four files in this directory: `Dockerfile`, `run_endpoint.sh`,
   `server.yml`, and `client.yml` as described below.

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

1. Now, copy the script below into `run_endpoint.sh`, and add commands as
   instructed. Logs should be recorded in `/qnslogs/` for them to be available
   after simulation completion (more on this later). Note that we insert a 10
   second sleep at the client because we currently have no synchronization, but
   we need the server and simulator to be running before the client.

    ```bash
    #!/bin/bash
    
    # Set up the routing needed for the simulation
    /setup.sh

    # The following variables are available for use:
    # - ROLE contains the role of this execution context, client or server
    # - SERVER_PARAMS contains user-supplied command line parameters
    # - CLIENT_PARAMS contains user-supplied command line parameters

    if [ "$ROLE" == "client" ]; then
        sleep 10     
        [ INSERT COMMAND TO RUN YOUR QUIC CLIENT ]
    elif [ "$ROLE" == "server" ]; then
        [ INSERT COMMAND TO RUN YOUR QUIC SERVER ]
    fi
    ```

1. Next, you need to create your custom YAML files that will be used by
   docker-compose to bring up your server or client. Copy the following into
   `server.yml` file.

    ```yaml
    version: "3.7"
    services:
        server:
            image: my_quic_implementation
    ```

    And the following into a `client.yml` file:
    ```yaml
    version: "3.7"
    services:
        client:
            image: my_quic_implementation
    ```

1. Finally, from the quic-network-simulator directory, build your image and assign a
   tag. For example, "my_quic_implementation":

   ```
   docker build my_impl_dir -t my_quic_implementation
   ```

   Note that you will need to run this build command any time you change your
   implementation, the Dockerfile, or the `run_endpoint.sh` file.

For an example, have a look at the [quic-go
setup](https://github.com/marten-seemann/quic-go-docker) or the [quicly
setup](https://github.com/h2o/h2o-qns).


## Running a Simulation

You will want to run the setup with a scenario. The scenarios that are currently
provided are listed below:
   
* [Simple point-to-point link, with configurable link properties](sim/scenarios/simple-p2p)

* [Single TCP connection running over a configurable point-to-point link](sim/scenarios/tcp-cross-traffic)

You can now run the experiment as follows:
```
   CLIENT_PARAMS=[params to client] \
   SERVER_PARAMS=[params to server] \
   SCENARIO=[scenario] \
   docker-compose -f base.yml \
        -f [server_image]/server.yml \
        -f [client_image]/client.yml \
        up
```

For instance, the following command runs a simple point-to-point scenario, and
specifies a command line parameter for the client implementation:
```
   CLIENT_PARAMS="-p /10000.txt" \
   SCENARIO="simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25" \
   docker-compose -f base.yml \
        -f my_quic_implementation/server.yml \
        -f another_quic_impl/client.yml \
        up
```


A mounted directory, `qnslogs`, is provided for recording logs from the
endpoints. This directory is created by the `run_server.sh` and `run_client.sh`
in the directories from which these scripts are run. Inside the docker
container, the directory is available as `/qnslogs/`.


## Debugging and FAQs

1. With the server (similarly for the client) up and running, you can get a root
   shell in the server docker container using the following:

   ```bash
   docker exec -it server /bin/bash
   ```
