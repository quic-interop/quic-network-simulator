# Simple Point-to-Point Link

This scenario builds a network with a simple configurable link, which is the
bottleneck link in the network. You can use this scenario to test your
implementation's performance under various bandwidth and delay
conditions. Importantly, you can set the queue size at the bottleneck link. This
enables a congestion controller to determine network bandwidth by witnessing
either queueing delay increases or loss when the queue is full.

This scenario can be used to compare your QUIC implementation's throughput when
competing with external traffic sources. For example, you can run iperf
alongside your client and server (within the same docker containers), to test
your QUIC implementation's performance when competing with one or multiple flows
of the host's TCP. NOTE: iperf sends traffic from the iperf client to the iperf
server.

This scenario has the following configurable properties:

* `--delay`: One-way delay of network. Specify with units. This is a required
  parameter. For example `--delay=15ms`.

* `--bandwidth`: Bandwidth of the link. Specify with units. This is a required
  parameter. For example `--bandwidth=10Mbps`. Specifying a value larger than
  10Mbps may cause the simulator to saturate the CPU.

* `--queue`: Queue size of the queue attached to the link. Specified in
  packets. This is a required parameter. For example `--queue=25`.

For example,
```bash
./run.sh "simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25"
```
