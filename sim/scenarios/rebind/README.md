# Port and IP Address Rebinding

This scenario uses a bottleneck link similar to the [simple-p2p](../simple-p2p)
scenario and simulates a path that is frequently changing the source port (and
optionally source IP address) of all client connections (similar to a NAT). This
scenario can be used to test whether QUIC implementations correctly handle such
rebindings. All flows are rebound every time the rebind timer expires.

This scenario has the following configurable properties:

* `--first-rebind`: The time of the first rebind event. This is a required
  parameter. For example, `--first-rebind=1s`.

* `--rebind-freq`: The frequency for subsequent rebinds after the first event.
  This is an optional parameter. By default, only a single rebind is performed.
  For example, `--rebind-freq=3s`.

* `--rebind-addr`: Whether IP addresses are also rebound. This is an optional
  parameter. By default, only client ports are rebound; `--rebind-addr` will
  rebind source IP addresses and source ports.

For example,
```bash
./run.sh "rebind --delay=15ms --bandwidth=10Mbps --queue=25 --first-rebind=1s"
```
