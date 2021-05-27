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
@Field final String REPO_CREDS   = 'esri-cityengine-sdk-deployer-key'
@Field final String SOURCES      = "cityengine-sdk.git/examples"
@Field final String BUILD_TARGET = 'package'
@Field final Map    CESDK_LATEST = PrtAppPipelineLibrary.Dependencies.CESDK_LATEST

@Field final List CONFIGS = [		
	[ grp: 'latest',  os: cepl.CFG_OS_RHEL7, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_GCC93,  cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64, deps: [ CESDK_LATEST ] ],
	[ grp: 'latest',  os: cepl.CFG_OS_WIN10, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_VC1427, cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64, deps: [ CESDK_LATEST ] ],
	[ grp: 'default', os: cepl.CFG_OS_RHEL7, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_GCC93,  cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64, deps: [] ],
	[ grp: 'default', os: cepl.CFG_OS_WIN10, bc: cepl.CFG_BC_REL, tc: cepl.CFG_TC_VC1427, cc: cepl.CFG_CC_OPT, arch: cepl.CFG_ARCH_X86_64, deps: [] ],
]


// -- PIPELINE

@Field String myBranch = env.BRANCH_NAME

// entry point for standalone pipeline
def pipeline(String branchName = null) {
	cepl.runParallel(getTasks(branchName))
	papl.finalizeRun('cityengine-sdk', myBranch)
}

// entry point for embedded pipeline
Map getTasks(String branchName = null) {
	if (branchName) myBranch = branchName

	Map tasks = [:]
	tasks << taskGenPRT4CMD(CONFIGS)
	tasks << taskGenSTLENC(CONFIGS)
	tasks << taskGenSTLDEC(CONFIGS)
	return tasks
}


// -- TASK GENERATORS

Map taskGenPRT4CMD(configs) {
	return cepl.generateTasks('prt4cmd', this.&taskBuildPRT4CMD, configs)
}

Map taskGenSTLENC(configs) {
	return cepl.generateTasks('stlenc', this.&taskBuildSTLENC, configs)
}

Map taskGenSTLDEC(configs) {
	return cepl.generateTasks('stldec', this.&taskBuildSTLDEC, configs)
}


// -- TASK BUILDERS

def taskBuildPRT4CMD(cfg) {
	final String appName = 'prt4cmd'
	final List defs = getPrtDirDefinition(cfg.deps) + [
		[ key: 'PRT4CMD_VERSION_MAJOR', val: 0 ],
		[ key: 'PRT4CMD_VERSION_MINOR', val: 0 ],
		[ key: 'PRT4CMD_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, cfg.deps, defs, REPO_CREDS)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg, { cepl.getArchiveClassifier(cfg) + '-cesdk_' + cfg.grp })
}

def taskBuildSTLENC(cfg) {
	final String appName = 'stlenc'
	final List defs = getPrtDirDefinition(cfg.deps) + [
		[ key: 'STLENC_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, cfg.deps, defs, REPO_CREDS)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg, { cepl.getArchiveClassifier(cfg) + '-cesdk_' + cfg.grp })
}

def taskBuildSTLDEC(cfg) {
	final String appName = 'stldec'
	final List defs = getPrtDirDefinition(cfg.deps) + [
		[ key: 'STLDEC_VERSION_MICRO', val: env.BUILD_NUMBER ]
	]
	papl.buildConfig(REPO, myBranch, "${SOURCES}/${appName}/src", BUILD_TARGET, cfg, cfg.deps, defs, REPO_CREDS)
	papl.publish(appName, myBranch, "esri_${appName}*.zip", { "0.0.${env.BUILD_NUMBER}" }, cfg, { cepl.getArchiveClassifier(cfg) + '-cesdk_' + cfg.grp })
}


// -- HELPER FUNCTIONS

List getPrtDirDefinition(deps) {
	if (deps.contains(CESDK_LATEST)) {
		return [ [ key: 'prt_DIR', val: CESDK_LATEST.p ] ]
	}
	return []
}


// -- make embeddable

return this
