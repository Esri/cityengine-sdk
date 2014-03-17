var prt4njs = require('../prt4njs/build/Debug/prt4njs');
//var prt4njs = require('../prt4njs/build/Release/prt4njs');
var util = require('util');

var prtRoot		= __dirname + "/../../../prt";
var prtLicType	= "CityEngAdvFx";
var prtLicHost	= "";
var prtLogLevel = 1;

var testRule = "rpk:file:/Volumes/Data/Users/shaegler/Documents/esri/dev/prt_trunk/com.esri.prt.test/resources/rules/candler.01.rpk!/bin/candler.01.cgb";

if (prt4njs.init(prtRoot, prtLicType, prtLicHost, prtLogLevel) != 0)
	return;

prt4njs.getRuleInfo(testRule, function(info) {
	console.log(util.inspect(info, false, null));
});

prt4njs.listEncoderIDs(function(ids) {
	console.log(ids);
});

prt4njs.getEncoderInfo("com.esri.prt.codecs.OBJEncoder", function(info) {
	console.log(util.inspect(info, false, null));
});

var initialShapes = [
    {
	'uid'		: "shape0001",
	'ruleSet'	: testRule,			// any URI, can also be data: URI
	'startRule'	: "Default$Init",	// Style$StartRule
	'vertices'	: [ ],				// vertex coordinate floats, multiple of 3
	'faces'		: [ ],				// face counts ccw: [ [face0idx], [face1idx], .. ]
	'attributes': [ 				// array with key/val pairs   
	    { "key" : 1.23 }
	]
    },
	//...
];


// open question: should the prt4njs impl offer a 'cached' callback (with optional webserver) or should it be done by the client code here


//var result = {
//	'encoder' : '<encId>',
//	'shapes' : [
//	{
//		'uid' : "shape0001",
//		'status': "ok | invalid rule | gen error | ...",
//		'data': "", // character or binary array (depends on encoder)
//	},
//	// next shape ...
//	]
//}
//
//var callbacks = {
//	'shapeBegin' : function(uid) { },
//	'shapeEnd'   : function(result) { }, // { 'uid' : "", 'data' : "" }
//	'generateEnd': function(result) { } // [ { 'uid': "", 'data': "" }, ... ]
//};
//
//prt4njs.generate(initialShapes, encoder, callbacks);

prt4njs.shutdown();


// client <---------> srv (prt4njs) <-> prt
 