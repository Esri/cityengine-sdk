#!/bin/bash

BUILDTYPE=Release

pushd codec
rm -rf build install && mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} ../src
make install
popd
popd

pushd client
# TODO: setup CC/CXX for gcc 4.1.2 to be on the safe side (as required by maya 2012)
rm -rf build install && mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} ../src
make install
popd
popd