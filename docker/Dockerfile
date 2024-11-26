FROM debian:bookworm

MAINTAINER Meelis Mägi "nimetu@gmail.com"
ENV DEBIAN_FRONTEND noninteractive
RUN dpkg --add-architecture i386 \
	&& apt-get update \
	&& apt-get upgrade -y \
	&& apt-get install -y --no-install-recommends \
		blackbox \
		less \
		mc \
		mesa-utils \
		p7zip \
		procps \
		tigervnc-common \
		tigervnc-standalone-server \
		tigervnc-tools \
		unzip \
		vim \
		x11-xserver-utils \
		xterm \
	&& apt-get autoremove -y --purge \
		pm-utils \
		xscreensaver* \
	&& apt-get autoremove -y --purge \
	&& apt-get clean -y \
	&& rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV RRS_ROOT=/rrs \
	TERM=xterm \
	DISPLAY=:1 \
	VNC_COL_DEPTH=24 \
	VNC_RESOLUTION=1042x768 \
	VNC_PW=vncpass \
	VNC_VIEW_ONLY=false \
	VNC_PORT=5901 \
	RRS_PORT=25000

EXPOSE \
	$VNC_PORT \
	$RRS_PORT

RUN useradd -ms /bin/bash ryzom && echo "ryzom:ryzom" | chpasswd && adduser ryzom sudo && \
	# create persistent volume using user credentials (not root:root)
	mkdir -p $RRS_ROOT   && chown ryzom:ryzom -R $RRS_ROOT

COPY --chown=ryzom:ryzom ./Xsession /home/ryzom/.Xsession
COPY ./vnc_startup.sh /

USER ryzom

WORKDIR /rrs

ENTRYPOINT ["/vnc_startup.sh"]
CMD ["sleep", "infinity"]


