# Drop Rate

This scenario uses a bottleneck link similar to the [simple-p2p](../simple-p2p)
scenario and optionally allows configuring the link to drop packets in either
direction. Packets to be dropped are chosen randomly in both directions, based
on rates specified by the user.

This scenario has the following configurable properties:

* `--delay`: One-way delay of network. Specify with units. This is a required
  parameter. For example `--delay=15ms`.

* `--bandwidth`: Bandwidth of the link. Specify with units. This is a required
  parameter. For example `--bandwidth=10Mbps`. Specifying a value larger than
  10Mbps may cause the simulator to saturate the CPU.

* `--queue`: Queue size of the queue attached to the link. Specified in
  packets. This is a required parameter. For example `--queue=25`.

* `--rate_to_client`: A value between 0 and 100 specifying the packet drop rate
  (in percentage) in the server to client direction. This is a required
  parameter. For example, `--rate_to_client=10`.

* `--rate_to_server`: Same as `rate_to_client` but in the other direction.

For example,
```bash
./run.sh "drop-rate --delay=15ms --bandwidth=10Mbps --queue=25 --rate_to_client=10 --rate_to_server=20"
```
