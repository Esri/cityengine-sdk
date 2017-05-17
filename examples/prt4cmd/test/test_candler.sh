#!/bin/bash

R="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
D=${R}/../../../data

${R}/../install/bin/prt4cmd \
     -f CityEngAdvFx \
     -l 3 \
     -g $D/candler_footprint.obj \
     -p $D/candler.rpk \
     -a ruleFile:string=bin/candler.cgb \
     -a startRule:string=Default\$Footprint \
     -a BuildingHeight:float=45 \
     -e com.esri.prt.codecs.ColladaEncoder \
     -z baseName:string=theCandler
