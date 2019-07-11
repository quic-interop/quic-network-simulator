# Network Simulator for QUIC benchmarking

The build script builds the network simulator (as found in the [sim](sim) directory) and a client and a server (as found in the [endpoint](endpoint) directory).

```sh
./build.sh
```

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

The build script automatically runs the three containers.
You can open a shell in the containers by running
```sh
docker exec -it client /bin/bash
```
Now you can ping the server
```sh
ping 192.168.100.100
```
All containers have tcpdump installed, so you can follow the packets flowing through the simulation.
