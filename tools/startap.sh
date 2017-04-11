#!/bin/bash

ip addr add 10.0.4.1/24 broadcast 10.0.4.255 dev wlp0s20f0u1 || exit 1
systemctl start hostapd.service || exit 1
systemctl start dnsmasq.service
