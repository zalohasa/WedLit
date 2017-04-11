#!/bin/bash

systemctl stop dnsmasq.service || exit 1
systemctl stop hostapd.service  || exit 1
ip addr del 10.0.4.1/24 dev wlp0s20f0u1 || exit 1
