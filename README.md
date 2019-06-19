# Network Simulator for QUIC benchmarking

The build script builds the network simulator (as found in the [sim](sim) directory) and a client and a server (as found in the [endpoint](endpoint) directory).

```sh
./build.sh
```

## Network topology

The build script creates two networks, `leftnet` (10.0.0.0/16) and `rightnet` (10.100.0.0/16). Leftnet is connected to the client, and rightnet is connected to the server. The ns3 simulation sits in the middle and forwards packets from leftnet to rightnet and vice versa, through the ns3 simulation.

## Running a simulation

The build script automatically runs the three containers.
You can open a shell in the containers by running
```sh
docker exec -it client /bin/bash
```
Now you can ping the server
```sh
ping 10.100.0.100
```
All containers have tcpdump installed, so you can follow the packets flowing through the simulation.
