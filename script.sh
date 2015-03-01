#!/bin/bash
SOCKET_PATH='/AVS/socket/server_socket'
VDB_PATH='/AVS/db/virus_database_1.txt'
LOG_PATH='/AVS/log/logfile.txt'
export SOCKET_PATH VDB_PATH LOG_PATH

if !(ps -A|grep 'AVS_Server'>/dev/null); then
	echo "Started"
	./AVS_Server &
	exit 0
fi
echo 'Is already running'
exit 0
