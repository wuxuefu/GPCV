#insmod 8188 wifi module
insmod /system/lib/modules/common/8188eu.ko
rm /etc/wpa_supplicant.conf
#cp /system/wpa_supplicant_linux.conf /etc/wpa_supplicant.conf
cp /media/sdcardc1/wifi/wpa_supplicant_linux.conf /etc/wpa_supplicant.conf
cp /system/bin/wpa_supplicant /bin/
sync
ifconfig wlan0 up
mkdir -p /var/run/wpa_supplicant
wpa_supplicant -B -iwlan0 -c /etc/wpa_supplicant.conf
udhcpc -i wlan0 -s /etc/udhcp/default.script

