#!/bin/bash

# Check for required kernel modules
REQUIRED_MODULES=("rtai_hal" "rtai_sched" "rtai_sem" "rtai_fifos")

for MODULE in "${REQUIRED_MODULES[@]}"; do
    if ! lsmod | grep "$MODULE" &> /dev/null; then
        echo "Loading module: '$MODULE'"
        sudo insmod "/usr/realtime/modules/$MODULE.ko"
    else
        echo "Module '$MODULE' is already loaded."
    fi
done