#!/bin/bash
# Make sure the script runs with super user privileges.
[ "$UID" -eq 0 ] || exec sudo bash "$0" "$@"
# Load the kernel module.
modprobe vcan
modprobe can-gw

# Create the virtual CAN interface.
ip link add dev vcan_master type vcan
ip link add dev vcan_client0 type vcan


# Bring the virtual CAN interface online.
ip link set up vcan_master
ip link set up vcan_client0

# clear all routing rules
cangw -F
# route all from master to client and vise versa
cangw -A -s vcan_master -d vcan_client0 -e
cangw -A -s vcan_client0 -d vcan_master -e

echo "Can routing rules: "
cangw -L