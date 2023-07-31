#!/bin/bash

script_dir=$(dirname "$0")

# Function to kill both blocking programs and exit the script
function cleanup_and_exit() {
    echo "Exiting script and killing the blocking programs..."
    kill "$pid1" "$pid2" 2>/dev/null
    exit
}

# Set up signal handler for Ctrl + C
trap 'cleanup_and_exit' SIGINT


cd "$script_dir/../cmake-build-debug/"

(./CanSotProtocol-example-client-linux -i vcan_client0 | sed 's/^/[CLIENT] /') &
pid1=$!

sleep 0.1

(./CanSotProtocol-example-master-linux -i vcan_master | sed 's/^/[MASTER] /') &
pid2=$!

wait "$pid1"
wait "$pid2"