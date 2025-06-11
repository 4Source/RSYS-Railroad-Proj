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

# Run make command to build the module
echo "Building the module..."
make

# Remove old module if present
MODULE_NAME=("rtai_module")
for (( idx=${#MODULE_NAME[@]}-1; idx>=0; idx-- )); do
    MODULE="${MODULE_NAME[idx]}"
    if lsmod | grep "$MODULE" &> /dev/null; then
        echo "Removing old module: '$MODULE'"
        sudo rmmod "$MODULE"
    else
        echo "No old module '$MODULE' to remove."
    fi
done

for MODULE in "${MODULE_NAME[@]}"; do
    # Insert the newly built module
    echo "Inserting the newly built module: '$MODULE'"
    sudo insmod "./src/$MODULE.ko" 
done

echo "Running..."

./src/dcc