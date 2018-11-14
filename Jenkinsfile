#!/usr/bin/env groovy

import groovy.transform.Field


// -- PIPELINE LIBRARIES

@Library('psl@simo6772/prt-apps-library-reviewed')
import com.esri.zrh.jenkins.PipelineSupportLibrary 

@Field def psl = new PipelineSupportLibrary(this)


// -- SETUP

properties([ disableConcurrentBuilds() ])
psl.runsHere('production')


// -- LOAD & RUN PIPELINE

def impl

node {
	checkout scm
	impl = load('pipeline.groovy')
}

stage('esri-cityengine-sdk') {
	impl.pipeline()
}
