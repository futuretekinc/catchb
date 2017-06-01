#!/bin/sh

case    "$1" in
"start")
	/etc/rc.d/S22syslogd start
	;;

"restart")
	/etc/rc.d/S22syslogd start
	;;

"get")
	cat /etc/config/syslogd | awk '{if ($2 ~ /^ip$/) { print $2, $3 } else if ($2 ~ /^id$/) { print $2, $3} else if ($2 ~/^comment$/) { print $2, $3}}'
	;;

"ip")
	cat /etc/config/syslogd | awk '{if ($2 ~ /^ip$/) print $3 }'
	;;

"add")
	if [ "$#" -ge 3 ]
	then
		echo "config server" > /tmp/syslogd.tmp
		echo "    option id $2" >> /tmp/syslogd.tmp
		echo "    option ip $3" >> /tmp/syslogd.tmp
		echo "    option comment $4" >> /tmp/syslogd.tmp

		[ -e "/etc/config/syslogd" ] && mv /etc/config/syslogd /etc/config/syslogd.old
		mv /tmp/syslogd.tmp /etc/config/syslogd
		sync

		/etc/rc.d/S22syslogd start
	fi
	;;

"del")
	[ -e "/etc/config/syslogd" ] && mv /etc/config/syslogd /etc/config/syslogd.old
	;;
esac
