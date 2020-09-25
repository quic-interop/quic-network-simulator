# NAT Rebinding

This scenario uses a bottleneck link similar to the [simple-p2p](../simple-p2p)
scenario and simulates a NAT that is frequently changing the source addresses of
all client connections. This scenario can be used to test whether QUIC
implementations correctly handle NAT and CGN rebindings (in the latter case, the
IP address also changes and not just the port.) All flows are rebound to new
source ports (and possibly IP a new IP address) every time the timer expires.

This scenario has the following configurable properties:

* `--first_rebind`: The time of the first NAT/CGN rebind event. Zero to disable.
  For example, `--first_rebind=1s`.

* `--rebind_freq`: The frequency for subsequent NAT/CGN rebinds after the first
  event. For example, `--rebind_freq=3s`.

* `--cgn`: Whether the NAT acts as a CGN and also rewrites addresses. Default is
  `false`. For example, `--cgn` enables CGN mode.

For example,
```bash
./run.sh "nat --delay=15ms --bandwidth=10Mbps --queue=25 --first_rebind=1s"
```
