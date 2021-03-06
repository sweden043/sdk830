#!/bin/sh
#
# Start elecard test server or testtool.
#

[% IF CONFIG_ELECD_ENABLE -%]
	[% SKIPFILE %]
[% END -%]

TESTSERVER=main_sdk7105_7105_ST40_LINUX_32BITS.out

case "$1" in
  start)
		STAPISDK_ROOT=/root
#		STAPISDK_ROOT=/root_33.1

		if [ -z "`cat /proc/cmdline | grep /dev/nfs`" ]; then
			. /etc/profile
			echo "Starting testServer..."
			$STAPISDK_ROOT/$TESTSERVER 0</dev/console
		fi

#	fi # runTests
	;;
  stop)
	echo -n "Stopping test server..."
	killall $TESTSERVER
	echo "done."
	;;
  restart|reload)
	"$0" stop
	"$0" start
	;;
  *)
	echo $"Usage: $0 {start|stop|restart}"
	exit 1
esac

exit $?

