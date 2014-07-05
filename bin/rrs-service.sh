#!/bin/bash

set -e -u

# options to pass render_service, eg --listen=25000
OPTS=${1:-}

# script directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


RRS_PID=
LOCKFILE=$(mktemp $DIR/.service-XXXXXX)

# remove lockfile, kill running process
cleanup(){
	rm ${LOCKFILE}
	[ ! -z $RRS_PID ] && kill $RRS_PID

	exit
}

# check if lockfile exists
lockfile(){
	if [ -f ${LOCKFILE} ]; then
		return 0
	fi

	return 1
}

# check if process is still running
running(){
	return $( ps -p $RRS_PID &> /dev/null )
}

trap "cleanup" SIGINT SIGTERM

# restart renderer as long as LOCKFILE exists
while lockfile; do
	# launch renderer
	./render_service ${OPTS} &> /dev/null &

	# write process ID to LOCKFILE
	RRS_PID=$!
	echo ${RRS_PID} > $LOCKFILE

	# wait until lockfile or process disappears
	while lockfile && running; do
		sleep 2s
	done

	# lockfile is missing, terminate process
	if running; then
		kill $RRS_PID
	else
		# small pause between process restart
		sleep 1
	fi
done

