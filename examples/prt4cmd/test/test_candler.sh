#!/bin/bash

T="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
R="$(dirname "${T}")"
U="https://github.com/Esri/cityengine-sdk/releases/download/3.1.9666/esri_ce_sdk-example_data-v3.zip"
D="${R}/data"
O="${R}/output"

if [ ! -d "${D}" ]
then
  wget -O "${R}/data.zip" "${U}"
  unzip "${R}/data.zip" -d "${R}"
fi

rm -rf "${O}"

"${R}/install/bin/prt4cmd" \
     -l 3 \
     -g "${D}/candler_footprint.obj" \
     -p "${D}/candler.rpk" \
     -a ruleFile:string=bin/candler.cgb \
     -a startRule:string=Default\$Footprint \
     -a BuildingHeight:float=45 \
     -e com.esri.prt.codecs.OBJEncoder \
     -z baseName:string=theCandler \
     -o "${O}"
