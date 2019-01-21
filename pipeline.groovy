import groovy.transform.Field
import com.esri.zrh.jenkins.PipelineSupportLibrary 
import com.esri.zrh.jenkins.PslFactory
import com.esri.zrh.jenkins.psl.UploadTrackingPsl
import com.esri.zrh.jenkins.JenkinsTools
import com.esri.zrh.jenkins.ce.CityEnginePipelineLibrary
import com.esri.zrh.jenkins.ce.PrtAppPipelineLibrary

@Field def psl = PslFactory.create(this, UploadTrackingPsl.ID)
@Field def cepl = new CityEnginePipelineLibrary(this, psl)
@Field def papl = new PrtAppPipelineLibrary(cepl)


// -- GLOBAL DEFINITIONS

@Field final String REPO         = cepl.GIT_REPO_CESDK
@Field final String SOURCES      = "esri-cityengine-sdk.git/examples"
@Field final String BUILD_TARGET = 'package'

@Field final List CONFIGS = [		
	[ os: cepl.CFG_OS_RHEL7, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_GCC63, cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64 ],
	[ os: cepl.CFG_OS_WIN10, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_VC141, cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64 ],
	[ os: cepl.CFG_OS_OSX12, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_AC81,  cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64 ],
]


// -- PIPELINE

@Field String myBranch = env.BRANCH_NAME

// entry point for standalone pipeline
def pipeline(String branchName = null) {
	cepl.runParallel(getTasks(branchName))
}

// entry point for embedded pipeline
Map getTasks(String branchName = null) {
	if (branchName) myBranch = branchName

	Map tasks = [:]
	tasks << taskGenPRT4CMD()
	tasks << taskGenSTLENC()
	tasks << taskGenSTLDEC()
	tasks << taskGenPRT4MAYA()
	return tasks
}


// -- TASK GENERATORS

Map taskGenPRT4CMD() {
	return cepl.generateTasks('prt4cmd', this.&taskBuildPRT4CMD, CONFIGS)
}

Map taskGenSTLENC() {
	return cepl.generateTasks('stlenc', this.&taskBuildSTLENC, CONFIGS)
}

Map taskGenSTLDEC() {
	return cepl.generateTasks('stldec', this.&taskBuildSTLDEC, CONFIGS)
}

Map taskGenPRT4MAYA() {
	return cepl.generateTasks('prt4maya', this.&taskBuildPRT4MAYA, CONFIGS.findAll{ it.os != cepl.CFG_OS_OSX12 }) // TODO: implement macos support
}


// -- TASK BUILDERS

def taskBuildPRT4CMD(cfg) {
	final String appName = 'prt4cmd'
	final List DEPS = [ PrtAppPipelineLibrary.Dependencies.CESDK ]
	final List defs = [
		[ key: 'prt_DIR',               val: PrtAppPipelineLibrary.Dependencies.CESDK.p ],
		[ key: 'PRT4CMD_VERSION_MAJOR', val: 0 ],
		[ key: 'PRT4CMD_VERSION_MINOR', val: 0 ],
		[ key: 'PRT4CMD_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, DEPS, defs)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg)
}

def taskBuildSTLENC(cfg) {
	final String appName = 'stlenc'
	final List DEPS = [ PrtAppPipelineLibrary.Dependencies.CESDK ]
	List defs = [
		[ key: 'prt_DIR',              val: PrtAppPipelineLibrary.Dependencies.CESDK.p ],
		[ key: 'STLENC_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, DEPS, defs)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg)
}

def taskBuildSTLDEC(cfg) {
	final String appName = 'stldec'
	final List DEPS = [ PrtAppPipelineLibrary.Dependencies.CESDK ]
	List defs = [
		[ key: 'prt_DIR',              val: PrtAppPipelineLibrary.Dependencies.CESDK.p ],
		[ key: 'STLDEC_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, DEPS, defs)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg)
}

def taskBuildPRT4MAYA(cfg) {
	final String appName = 'prt4maya'
	final List DEPS = [ PrtAppPipelineLibrary.Dependencies.CESDK, PrtAppPipelineLibrary.Dependencies.MAYA2018 ]
	List defs = [
		[ key: 'prt_DIR',                val: PrtAppPipelineLibrary.Dependencies.CESDK.p ],
		[ key: 'maya_DIR',               val: PrtAppPipelineLibrary.Dependencies.MAYA2018.p ],
		[ key: 'PRT4MAYA_VERSION_MAJOR', val: 0 ],
		[ key: 'PRT4MAYA_VERSION_MINOR', val: 0 ],
		[ key: 'PRT4MAYA_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, DEPS, defs)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg)
}


// -- make embeddable

return this
