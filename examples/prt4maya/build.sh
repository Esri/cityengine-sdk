#!/bin/bash

BUILDTYPE=Release

if [ $# -eq 0 ]
then
	CLIENT_TARGET=install
	VER_MAJOR=0
	VER_MINOR=0
	VER_MICRO=0
else
  	CLIENT_TARGET=package
	VER_MAJOR=$1
	VER_MINOR=$2
	VER_MICRO=$3
fi

if [ -z "$2" ]
  then
    PRT4MAYA_VERSION=0
  else
  	PRT4MAYA_VERSION=$2
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
cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} -DPRT4MAYA_VERSION_MAJOR=${VER_MAJOR} -DPRT4MAYA_VERSION_MINOR=${VER_MINOR} -DPRT4MAYA_VERSION_MICRO=${VER_MICRO} ../src
make ${CLIENT_TARGET}
popd
popd