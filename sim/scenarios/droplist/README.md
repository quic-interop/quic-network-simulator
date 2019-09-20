# Droplist

This scenario uses a bottleneck link similar to the [simple-p2p](../simple-p2p)
scenario and optionally allows configuring the link to drop specific packets in
either direction.

This scenario can be used to test the performance of QUIC implementations under
specific loss patterns. For instance, tests can:
* drop specific handshake packets,
* drop tail packets,
* drop important control packets, such as path validation packets.

This scenario has the following configurable properties:

* `--delay`: One-way delay of network. Specify with units. This is a required
  parameter. For example `--delay=15ms`.

* `--bandwidth`: Bandwidth of the link. Specify with units. This is a required
  parameter. For example `--bandwidth=10Mbps`. Specifying a value larger than
  10Mbps may cause the simulator to saturate the CPU.

* `--queue`: Queue size of the queue attached to the link. Specified in
  packets. This is a required parameter. For example `--queue=25`.

* `--drops_to_client`: A comma-separated list of packets to drop, starting at 1,
 in the server to client direction. These packet numbers are simply the index of
 the packets arriving at the bottleneck link. This index applies to all IP
 packets arriving at the link, and may not be the same as the QUIC packet
 number, especially if an endpoint coalesces packets or if an endpoint resets
 packet numbers across epochs. This is an optional parameter. For example,
 `--drops_to_client=1,3,15`. (Note that there are no spaces in the list.)`

* `--drops_to_server`: Same as `drops_to_client` but in the other direction.

For example,
```bash
./run.sh "droplist --delay=15ms --bandwidth=10Mbps --queue=25 --drops_to_client=1,3,4 --drops_to_server=5"
```
