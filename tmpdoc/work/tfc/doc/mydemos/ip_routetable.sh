#!/bin/sh
ip rule add to 192.168.0.216 table 200
ip rule add to 192.168.0.109 table 201
ip rule add from 192.168.0.216 table 200
ip rule add from 192.168.0.109 table 201

ip route add default via 192.168.0.1 dev eth0 table 200
ip route add 192.168.0.0/24 dev eth0 table 200
ip route add default via 192.168.0.1 dev ra0 table 201
ip route add 192.168.0.0/24 dev ra0 table 201

