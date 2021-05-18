#
# Ryzom
#
# Multi-stage build requires docker 17.05 or newer
#
FROM debian:buster as base
LABEL maintainer="Meelis Mägi <nimetu@gmail.com>"

ENV RYZOM_EXTERNAL=/ryzom-external

RUN apt-get update -y && apt-get install -y \
		autoconf \
		automake \
		build-essential \
		cmake \
		debhelper \
		git \
		less \
		libboost-dev \
		libgl1-mesa-dev \
		libopenal-dev \
		libtool \
		libxrandr-dev \
		libxrender-dev \
		libxxf86vm-dev \
		lintian \
		nano \
		quilt \
		vim \
		zlib1g-dev \
	&& apt-get clean

# TODO: libavcodec-dev, libavformat-dev, libavutil-dev, libswresample-dev
# ===============================
FROM base as lzma

ARG NAME=xz-utils
ARG VER=5.2.2
ARG SRC=${NAME}_${VER}.orig.tar.xz
ARG SUB=x
ARG SOURCEDIR=/tmp/${NAME}-${VER}

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x --xz --directory $SOURCEDIR \
	&& ./configure --prefix=$RYZOM_EXTERNAL --enable-shared=no --enable-static=yes \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/bin \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/share

# ===============================
FROM base as libxml2

ARG NAME=libxml2
ARG VER=2.9.4
ARG SRC=${NAME}_${VER}+dfsg1.orig.tar.xz
ARG SUB=libx
ARG SOURCEDIR=/tmp/${NAME}-${VER}

COPY --from=lzma /tmp/dest/$RYZOM_EXTERNAL $RYZOM_EXTERNAL

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x --xz --directory $SOURCEDIR \
	&& ./configure --prefix=$RYZOM_EXTERNAL CPPFLAGS=-I$RYZOM_EXTERNAL/include LDFLAGS=-L$RYZOM_EXTERNAL/lib --enable-shared=no --enable-static=yes --without-ftp --without-python --without-dockbook --without-debug \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/share

# ===============================
FROM base as openssl

ARG NAME=openssl
ARG VER=1.1.1k
ARG SRC=${NAME}_${VER}.orig.tar.gz
ARG SUB=o
RUN echo "Compiling $SRC"
RUN cd /tmp \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar -xzC /tmp \
	&& cd /tmp/$NAME-$VER \
	&& ./config --prefix=$RYZOM_EXTERNAL --openssldir=$RYZOM_EXTERNAL no-shared \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install_sw \
	&& rm -fR \
		/tmp/dest/$RYZOM_EXTERNAL/bin \
		/tmp/dest/$RYZOM_EXTERNAL/lib/engines-1.1

# ===============================
FROM base as libcurl

ARG NAME=curl
ARG VER=7.74.0
ARG SRC=${NAME}_${VER}.orig.tar.gz
ARG SUB=c

COPY --from=openssl /tmp/dest/$RYZOM_EXTERNAL/include $RYZOM_EXTERNAL
COPY --from=openssl /tmp/dest/$RYZOM_EXTERNAL/lib     $RYZOM_EXTERNAL

RUN echo "Compiling $SRC"
RUN cd /tmp \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar -xzC /tmp \
	&& mkdir /tmp/build && cd /tmp/build \
	&& cmake /tmp/$NAME-$VER \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$RYZOM_EXTERNAL \
		-DCMAKE_SYSTEM_INCLUDE_PATH=$RYZOM_EXTERNAL/include \
		-DBUILD_CURL_EXE=OFF \
		-DBUILD_CURL_TESTS=OFF \
		-DCMAKE_USE_OPENSSL=ON \
		-DCURL_STATICLIB=ON \
		-DENABLE_IPV6=ON \
		-DHTTP_ONLY=ON \
	&& make && make DESTDIR=/tmp/dest install

# ===============================
FROM base as libpng

ARG NAME=libpng1.6
ARG VER=1.6.28
ARG SRC=${NAME}_${VER}.orig.tar.xz
ARG SUB=libp
ARG SOURCEDIR=/tmp/${NAME}-${VER}

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x --xz --directory $SOURCEDIR \
	&& mkdir /tmp/build && cd /tmp/build \
	&& cmake $SOURCEDIR \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$RYZOM_EXTERNAL \
		-DCMAKE_SYSTEM_INCLUDE_PATH=$RYZOM_EXTERNAL/include \
		-DPNG_SHARED=OFF \
		-DPNG_STATIC=ON \
		-DSKIP_INSTALL_EXECUTABLES=ON \
		-DSKIP_INSTALL_EXPORT=ON \
		-DSKIP_INSTALL_FILES=ON \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install

# ===============================
FROM base as libjpeg

ARG NAME=libjpeg9
ARG VER=9d
ARG SRC=${NAME}_${VER}.orig.tar.gz
ARG SUB=libj
ARG SOURCEDIR=/tmp/${NAME}-${VER}

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x -z --directory $SOURCEDIR \
	&& ./configure --prefix=$RYZOM_EXTERNAL --enable-shared=no --enable-static=yes \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/bin \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/share

# && tar --strip-components=1 -x --bzip2 --directory $SOURCEDIR -f /tmp/${NAME}-${VER}.tar.bz2 \
# ===============================
FROM base as giflib

ARG NAME=giflib
ARG VER=5.1.4
ARG SRC=${NAME}_${VER}.orig.tar.bz2
ARG SUB=g
ARG SOURCEDIR=/tmp/${NAME}-${VER}

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x --bzip2 --directory $SOURCEDIR \
	&& ./configure --prefix=$RYZOM_EXTERNAL --enable-shared=no --enable-static=yes \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/bin

# ===============================
FROM base as freetype

ARG NAME=freetype
ARG VER=2.9.1
ARG SRC=${NAME}_${VER}.orig.tar.gz
ARG SUB=f
ARG SOURCEDIR=/tmp/${NAME}-${VER}

RUN echo "Compiling $NAME-$VER"
RUN mkdir $SOURCEDIR && cd $SOURCEDIR \
	&& curl -SL http://ftp.debian.org/debian/pool/main/$SUB/$NAME/$SRC | tar --strip-components=1 -x -z --directory $SOURCEDIR \
	&& mkdir /tmp/build && cd /tmp/build \
	&& cmake $SOURCEDIR \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX=$RYZOM_EXTERNAL \
	&& make -j2 \
	&& make DESTDIR=/tmp/dest install \
	&& rm -r /tmp/dest/$RYZOM_EXTERNAL/lib/cmake

# ===============================
# = Final container
FROM base

COPY --from=lzma      /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=libxml2   /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=openssl   /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=libcurl   /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=libpng    /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=libjpeg   /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=giflib    /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}
COPY --from=freetype  /tmp/dest/${RYZOM_EXTERNAL} ${RYZOM_EXTERNAL}

WORKDIR /rrs/rrsdev

RUN useradd -ms /bin/bash ryzom

USER ryzom

CMD ["/rrs/rrsdev/build.sh"]

