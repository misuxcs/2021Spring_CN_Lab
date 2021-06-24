#!/bin/bash
ip tuntap add tun$1 mode tun
echo "[INFO] Add new tun"
ip link set tun$1 up
ip addr add $2/24 dev tun$1
./tun -i tun$1 -s
