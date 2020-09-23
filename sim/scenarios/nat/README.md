# NAT Rebinding

This scenario uses a bottleneck link similar to the [simple-p2p](../simple-p2p)
scenario and simulates a NAT that is frequently changing the source addresses of
all client connections. This scenario can be used to test whether QUIC
implementations correctly handle NAT rebindings.

This scenario has the following configurable properties:

* `--rebind`: The frequency with which the NAT rebinds all connections. The
  rebinding timer starts at the start of the simulation, and all flows are
  rebound to new source ports every time the timer expires. This is a required
  parameter. For example `--rebind=3s`.

For example,
```bash
./run.sh "nat --delay=15ms --bandwidth=10Mbps --queue=25 --rebind=3s"
```
