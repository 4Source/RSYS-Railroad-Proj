#!/bin/bash

# Check for required kernel modules
REQUIRED_MODULES=("rtai_hal" "rtai_sched" "rtai_sem" "rtai_fifos")

for MODULE in "${REQUIRED_MODULES[@]}"; do
    if ! lsmod | grep "$MODULE" &> /dev/null; then
        echo "Loading module: $MODULE"
        sudo insmod "/usr/realtime/modules/$MODULE.ko"
    else
        echo "Module $MODULE is already loaded."
    fi
done

# Run make command to build the module
echo "Building the module..."
make

# Remove old module if present
MODULE_NAME="rtai_main"
if lsmod | grep "$MODULE_NAME" &> /dev/null; then
    echo "Removing old module: $MODULE_NAME"
    sudo rmmod "$MODULE_NAME"
else
    echo "No old module to remove."
fi

# Insert the newly built module
echo "Inserting the newly built module: $MODULE_NAME"
sudo insmod "./src/$MODULE_NAME.ko" 

echo "Running..."

./src/dcc