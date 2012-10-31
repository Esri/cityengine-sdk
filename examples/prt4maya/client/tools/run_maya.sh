#!/bin/bash

ROOT="$(dirname "$0")"/..

export MAYA_PLUG_IN_PATH="${MAYA_PLUG_IN_PATH}:${ROOT}/install"
export MAYA_SCRIPT_PATH="${MAYA_SCRIPT_PATH}:${ROOT}/install"
#maya -d gdb
maya
