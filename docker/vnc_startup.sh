#!/bin/bash

set -e

sigterm(){
	echo "Caught SIGTERM signal!"

	echo "Stopping rrs-server"
	pkill rrs-service.sh

	echo "Stopping monitor"
	kill -TERM $logger_pid
}

# docker will seng SIGTERM when shutting down
trap sigterm SIGTERM

#######################################################################
#### vnc
source $HOME/.bashrc

if [[ $1 =~ --skip ]]; then
	echo "command: '${@:2}'"
	exec "${@:2}"
fi

# override vnc password on each startup
# if VNC_PW is empty, then there will be no password
mkdir -p "$HOME/.vnc"
PASSWD_PATH="$HOME/.vnc/passwd"
echo -e "$VNC_PW\n$VNC_PW\n"| vncpasswd -f > $PASSWD_PATH
chmod 600 $PASSWD_PATH

SEC_TYPE=
if [[ -z $VNC_PW ]]; then
	SEC_TYPE="-SecurityType none --I-KNOW-THIS-IS-INSECURE"
fi

#TODO: novnc
vncserver -kill $DISPLAY || rm -frv /tmp/.X*-lock /tmp/.X11-unit || echo "remove old locks"
vncserver $DISPLAY -depth $VNC_COL_DEPTH -geometry $VNC_RESOLUTION -localhost no $SEC_TYPE

#######################################################################
if [ -z "$1" ] || [[ $1 =~ -t|--tail-log ]]; then
	echo -e "\n---- $HOME/.vnc/*$DISPLAY.log ----"
	cat $HOME/.vnc/*$DISPLAY.log

	LOGFILE=/rrs/render_service.log
	touch $LOGFILE

	tail -F $LOGFILE &
    logger_pid=$!
    wait $logger_pid

    exit
else
	echo "command: '$@'"
	exec $@
fi

