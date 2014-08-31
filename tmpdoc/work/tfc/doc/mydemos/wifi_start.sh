#!/bin/sh
ifconfig ra0 up
cat > /tmp/wpa_supplicant.conf <<filecontent
ctrl_interface=DIR=/tmp/wpa_supplicant GROUP=root
network={
    ssid="WIFI_CAM"
    scan_ssid=1
    proto=WPA
    key_mgmt=WPA-PSK
    psk="12345678"
    pairwise=CCMP TKIP
    priority=5
}
filecontent
wpa_supplicant -B -C /tmp/wpa_supplicant -c /tmp/wpa_supplicant.conf -i ra0
rm -rf /var/run/dhcpcd-ra0.pid
dhcpcd ra0
