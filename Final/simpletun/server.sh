#!/bin/bash
sudo ip tuntap add tun$1 mode tun
echo "[INFO] Add new tun"
sudo ip link set tun$1 up
sudo ip addr add $2/24 dev tun$1
sudo ./tun -i tun$1 -s
echo "Server running..."
