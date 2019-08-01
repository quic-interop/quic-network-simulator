# Blackhole

This scenario uses a bottleneck link like the [../simple-p2p](simple-p2p)
scenario. The link can be configured to start dropping (blackholing) all
packets for a certain duration, and unblock some time later. Optionally,
this pattern can be repeated.

This scenario can be used to test various features of QUIC:
* Short durations can be used to test PTOs.
* Slightly longer durations can be used to test
  the detection of persistent congestion.
* Long durations can be used to test the idle timeout.

This scenario has the following configurable properties:

* `--delay`: One-way delay of network. Specify with units. This is a required
  parameter. For example `--delay=15ms`.

* `--bandwidth`: Bandwidth of the link. Specify with units. This is a required
  parameter. For example `--bandwidth=10Mbps`. Specifying a value larger than
  10Mbps may cause the simulator to saturate the CPU.

* `--queue`: Queue size of the queue attached to the link. Specified in
  packets. This is a required parameter. For example `--queue=25`.

* `--on`: Time period that traffic is allowed to flow. Specify with units. This is a
  required parameter. For example `--on=10s`.

* `--off`: Time period that traffic is blocked. Specify with units. This is a required
  parameter. For example `--off=3s`.

* `--repeat`: Repeatedly block and unblock traffic. This is an optional
  parameter, which defaults to 1. For example `--repeat=2` will allow traffic
  `on` seconds, block it for `off` seconds, allow it for `on` seconds, block it
  for `off` seconds, and then allow traffic indefinitely.



For example,
```bash
./run.sh "blackhole --delay=15ms --bandwidth=10Mbps --queue=25 --on=10s --off=2s"
```
