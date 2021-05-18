#!/bin/sh

set -e -u

DIR=$(readlink -f $(dirname $0))
SOURCE=${DIR}/ryzom-core
BUILD=${DIR}/build
APPDIR=${DIR}/../app

if [ ! -d ${SOURCE} ]; then
	git clone --depth 1 --branch feature/develop-atys https://gitlab.com/ryzom/ryzom-core.git ${SOURCE}
fi

if [ ! -f ${SOURCE}/personal/CMakeLists.txt ]; then
	cd ${SOURCE}/personal
	ln -s ${DIR}/.. rrs-server
	echo "ADD_SUBDIRECTORY(rrs-server)" > CMakeLists.txt
fi

if [ ! -d ${BUILD} ]; then
	mkdir -p ${BUILD}
fi

export CXXFLAGS="${CXXFLAGS:-} -DNEL_LOG_IN_FILE=0"

cd ${BUILD}
cmake ${SOURCE} \
	-DCMAKE_BUILD_TYPE=Release \
	-DWITH_BMSITE_RRS=ON \
	-DWITH_GUI=OFF \
	-DWITH_LOGIC=OFF \
	-DWITH_NEL_SAMPLES=OFF \
	-DWITH_NEL_TESTS=OFF \
	-DWITH_NEL_TOOLS=OFF \
	-DWITH_PACS=OFF \
	-DWITH_PERSONAL=ON \
	-DWITH_RYZOM_CLIENT=OFF \
	-DWITH_RYZOM_SERVER=OFF \
	-DWITH_RYZOM_TOOLS=OFF \
	-DWITH_SOUND=OFF \
	-DWITH_STATIC=ON \
	-DWITH_STATIC_DRIVERS=ON \
	-DWITH_EXTERNAL=ON \
	-DEXTERNAL_PATH=/ryzom-external

make -j $(($(nproc) - 1))

cp ${BUILD}/bin/render_service $APPDIR

