#!/bin/bash

# Set up the routing needed for the simulation.
/setup.sh
# Wait for the simulator to start up.
/wait-for-it.sh sim:57832 -s -t 30
/bin/bash
