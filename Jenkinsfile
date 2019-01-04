#!/usr/bin/env groovy

import groovy.transform.Field


// -- PIPELINE LIBRARIES

@Library('psl')
import com.esri.zrh.jenkins.PipelineSupportLibrary 
import com.esri.zrh.jenkins.PslFactory 

@Field def psl = PslFactory.create(this)


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
