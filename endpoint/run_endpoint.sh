#!/bin/bash

# Set up the routing needed for the simulation.
/setup.sh

if [ "$ROLE" == "client" ]; then
    # Wait for the simulator to start up.
    /wait-for-it.sh sim:57832 -s -t 30
fi

/bin/bash
