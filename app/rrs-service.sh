#!/bin/bash

set -e -u

# options to pass render_service, eg --listen=25000
OPTS=${1:---listen=25000}

# script directory
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"


RRS_PID=
LOCKFILE=$DIR/.service-running

# remove lockfile, kill running process
cleanup(){
	[ -f $LOCKFILE ] && rm $LOCKFILE
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

trap cleanup EXIT

touch $LOCKFILE

# restart renderer as long as LOCKFILE exists
while lockfile; do
	if [ -f ./render_service_new ]; then
		echo "Copying new file"
		mv -f ./render_service_new ./render_service
	fi

	# launch renderer
	./render_service $OPTS &> /dev/null &
	RRS_PID=$!

	# log output to terminal
	tail -F render_service.log &
	LOG_PID=$!

	# wait until lockfile or process disappears
	while lockfile && running; do
		sleep 2s
	done

	kill $LOG_PID

	# lockfile is missing, terminate process
	if running; then
		kill $RRS_PID
	else
		# small pause between process restart
		sleep 1
	fi
done

