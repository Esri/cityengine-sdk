from unittest import TestCase
from nose.tools import assert_equal

from prt4py import *

class SimpleTest(TestCase):

	@classmethod
	def setupClass(cls):
		prtRoot = "/home/shaegler/procedural/dev/eclipse_workspaces/prt_eclipse_workspace/com.esri.prt.build/build/PRT_BC_REL-PRT_CC_OPT-PRT_TC_GCC/prt"
		#prtRoot = "/home/shaegler/procedural/dev/builds/cesdk/esri_ce_sdk_1_1_1375_rhel6_gcc44_rel_opt"
		initStatus = prtInit(prtRoot, "CityEngAdvFx")
		assert_equal(initStatus, 0)
		print "setUpAll"

	def test_generate(self):
		rulePackageURI = "file:/home/shaegler/procedural/dev/eclipse_workspaces/prt_eclipse_workspace/com.esri.prt.test/resources/rules/candler.01.rpk"
		initialShapes = {
			"theBigShape" : {
				"ruleFile":		"bin/candler.01.cgb",
				"startRule":	"Default$Lot",
				"randomSeed":	666,
				"vtxCoords":	[ 0.0, 0.0, 0.0,  0.0, 0.0, 50.0,  50.0, 0.0, 50.0,  50.0, 0.0, 0.0 ],
				"indices":		[ 0, 1, 2, 3 ],
				"faceCounts":	[ 4 ]
			},
			"theSmallShape" : {
				"ruleFile":		"bin/candler.01.cgb",
				"startRule":	"Default$Lot",
				"randomSeed":	666,
				"vtxCoords":	[ 100.0, 0.0, 0.0,  100.0, 0.0, 10.0,  110.0, 0.0, 10.0,  110.0, 0.0, 0.0 ],
				"indices":		[ 0, 1, 2, 3 ],
				"faceCounts":	[ 4 ]
			}
		}
		genStat = prtGenerate(rulePackageURI, initialShapes, "com.esri.prt.codecs.OBJEncoder", "/tmp/prt4py")
		print prtGetStatusMessage(genStat)
		assert_equal(genStat, 0)

	
	@classmethod
	def teardownClass(cls):
		prtRelease()
