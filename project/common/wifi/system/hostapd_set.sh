insmod /system/lib/modules/common/8188eu.ko
ifconfig wlan1 up
ifconfig wlan1 192.168.100.1
ifconfig
#sleep 1
mkdir /var/run/hostapd
#/system/bin/hostapd ./hostapd_2G.conf &
/system/bin/hostapd /media/sdcardc1/wifi/hostapd_2G.conf &
mkdir -p /var/lib/misc
cp /system/dnsmasq.leases /var/lib/misc/
#/system/bin/dnsmasq -C /system/dnsmasq.conf -I /system/dnsmasq.leases
/system/bin/dnsmasq -C /media/sdcardc1/wifi/dnsmasq.conf -I /media/sdcardc1/wifi/dnsmasq.leases
