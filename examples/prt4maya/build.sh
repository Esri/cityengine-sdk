#!/bin/bash

BUILDTYPE=Release

if [ $# -eq 0 ]
then
	echo "ERROR: first argument must be maya path"
	exit 1
fi

export maya_DIR=$1
MAYA_VERSION=${maya_DIR:-4}

if [ $# -eq 1 ]
then
	CLIENT_TARGET=install
	VER_MAJOR=0
	VER_MINOR=0
	VER_MICRO=0
else
  	CLIENT_TARGET=package
	VER_MAJOR=$2
	VER_MINOR=$3
	VER_MICRO=$4
fi

pushd codec
rm -rf build install && mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} ../src
make install
popd
popd

pushd client
# TODO: enforce CC/CXX for gcc 4.1.2 to be on the safe side (as required by maya 2012)
rm -rf build install && mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} -DPRT4MAYA_VERSION_MAJOR=${VER_MAJOR} -DPRT4MAYA_VERSION_MINOR=${VER_MINOR} -DPRT4MAYA_VERSION_MICRO=${VER_MICRO} -DMAYA_VERSION=${MAYA_VERSION} ../src
make ${CLIENT_TARGET}
popd
popd
