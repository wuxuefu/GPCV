#!/bin/sh 
# based on http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=65516 

#debug 
#echo "action: $ACTION / mdev: $MDEV / devpath: $DEVPATH / subsystem: $SUBSYSTEM / mntdir: $MNT_DIR" >/dev/console

# MMC slot 0 
mmc_name=$1

# check mount / unmount
if [ "$ACTION" = "remove" ]; then
	# debug
#	echo `lsof -t "$MNT_DIR"` >> /tmp/hotplug.log
	# send notification to process with mounted files open
	#kill -HUP `lsof -t "$MNT_DIR"`
#   kill -INT `lsof -t "$MNT_DIR"`
	# unmount
	/bin/umount -lf "/media/$mmc_name"
	/bin/rmdir "/media/$mmc_name"

#   echo -e "$ACTION" "0" "$DEVTYPE" "$DEVNAME" "/media/$mmc_name"  '\n' > /AppStorageMonitor &

else
	# mount
	n=1
	maxLoopCount=5
	while [ $n -le $maxLoopCount ]
	do
		if [ ! -d "/media/$mmc_name" ]; then
			/bin/mkdir "/media/$mmc_name"
		fi
		
		fstype=`blkid /dev/$1 | awk '{print $3}'`
		
		if [ "$fstype" == 'TYPE="vfat"' ] ; then
			/bin/mount -t vfat -o utf8 /dev/$1 "/media/$mmc_name"
		elif [ "$fstype" == 'TYPE="exfat"' ] ; then
			mount.exfat /dev/$1 "/media/$mmc_name"
		else
			/bin/mount -t auto -o utf8 /dev/$1 "/media/$mmc_name"
		fi
		
		result=$?
		if [ ! "$result" == "0" ]; then
			/bin/rmdir "/media/$mmc_name"
		else
			break
		fi
		usleep 10000
		n=$(( n+1 ))	 # increments $n
	done
	
#	echo -e "$ACTION" "$result" "$DEVTYPE" "$DEVNAME" "/media/$mmc_name" '\n' > /AppStorageMonitor &
fi
