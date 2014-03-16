var prt4njs = require('../prt4njs/build/Release/prt4njs');
var util = require('util');

var prtRoot		= __dirname + "/../../../prt";
var prtLicType	= "CityEngAdvFx";
var prtLicHost	= "";
var prtLogLevel = 1;

var testRule = "rpk:file:/Volumes/Data/Users/shaegler/Documents/esri/dev/prt_trunk/com.esri.prt.test/resources/rules/candler.01.rpk!/bin/candler.01.cgb";

if (prt4njs.init(prtRoot, prtLicType, prtLicHost, prtLogLevel) != 0)
	return;

//var info = prt4njs.getRuleInfoAdv(testRule);
//console.log("-- RULES (" + info.getNumRules() + ")");
//for (var ri = 0; ri < info.getNumRules(); ri++) {
//	var rule = info.getRule(ri);
//	var msg = "   " + rule.getName() + "(";
//	for (var pi = 0; pi < rule.getNumParameters(); pi++) {
//		var param = rule.getParameter(pi);
//		msg += (pi > 0 ? ", " : "") + param.getType() + " " + param.getName();
//	}
//	msg += ")";
//	console.log(msg);
//}
//console.log("-- ATTRS (" + info.getNumAttributes() + ")");
//for (var ai = 0; ai < info.getNumAttributes(); ai++) {
//	var attr = info.getAttribute(ai);
//	console.log("   " + attr.getReturnType() + " " + attr.getName());
//}


prt4njs.getRuleInfo(testRule, function(info) {
	console.log(util.inspect(info, false, null));
});


var initialShapes = [];

var initialShapeData = {
	'uid'		: "shape0001",
	'ruleSet'	: testRule,			// any URI, can also be data: URI
	'startRule'	: "Default$Init",	// Style$StartRule
	'vertices'	: [ ],				// vertex coordinate floats, multiple of 3
	'faces'		: [ ],				// face counts ccw: [ [face0idx], [face1idx], .. ]
	'attributes': [ 				// array with key/val pairs   
	    { "key" : 1.23 }
	]
}
initialShapes.push(prt4njs.createInitialShape(initialShapeData)); 

var callback = prt4njs.createCallback(function(jsonMetadata) {
	// process the metadata, e.g. a webserver would return it to the client who in turn can decide to fetch the actual data

	// example return json:
	//	[
	//	 {
	//		 'uid' : "shape0001",
	//		 'data': "http://localhost:1337/shape0001/data"
	//	 },
	//	 {
	//		 'uid' : "shape0002",
	//		 ...
	//	 }
	//	]
	
});
prt4njs.generate(initialShapes, encoder, cachedCallback);

prt4njs.shutdown();


// client <---------> srv (prt4njs) <-> prt
 