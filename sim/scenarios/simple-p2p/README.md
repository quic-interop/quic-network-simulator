# Simple point-to-point link

This scenario builds a network with a simple link, with the following
configurable properties:

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
