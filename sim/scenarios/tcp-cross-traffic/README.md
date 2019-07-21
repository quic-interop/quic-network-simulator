# TCP Cross-Traffic

This scenario builds a network with a simple configurable bottleneck link and
generates TCP traffic running over this link from the server to the client. If
there is no other traffic, this flow will saturate the link. Running your QUIC
traffic through this scenario allows you to test how your congestion controller
performs when competing against a TCP flow at the bottleneck. This scenario is
most useful with large transfers, for comparing long-term bandwidth sharing and
for looking into flow dynamics when competing.

Note that this TCP is a native implementation within ns-3 and uses Reno
congestion control.

The simulator prints the bandwidth seen by the receiving TCP application over
5-second intervals.

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
./run.sh "tcp-cross-traffic --delay=15ms --bandwidth=10Mbps --queue=25"
```
