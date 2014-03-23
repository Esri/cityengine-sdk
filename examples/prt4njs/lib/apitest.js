var prt4njs = require('../prt4njs/install/prt4njs');
//var prt4njs = require('../prt4njs/build/Release/prt4njs');
var util = require('util');

var prtRoot		= __dirname + "/../../../prt";
var prtLicType	= "CityEngAdvFx";
var prtLicHost	= "";
var prtLogLevel = 1;

var testRPKURI		= "file:/Volumes/Data/Users/shaegler/Documents/esri/dev/prt_trunk/com.esri.prt.test/resources/rules/candler.01.rpk";
var testRulePath	= "/bin/candler.01.cgb";
var testRuleURI		= "rpk:" + testRPKURI + "!" + testRulePath;

//var prt = {
//'root' : prtRoot,
//'licType' : prtLicType,
//'licHost' : prtLicHost,
//'logLevel': prtLogLevel,
//}

if (prt4njs.init(prtRoot, prtLicType, prtLicHost, prtLogLevel) !== 0) {
	return;
}

prt4njs.getRuleInfo(testRuleURI, function(info) {
	console.log(util.inspect(info, false, null));
});

prt4njs.listEncoderIDs(function(ids) {
	console.log(ids);
});

var initialShapes =
	[ {
		'uid'		: "shape0",
		'rpk'		: testRPKURI,
		'ruleFile'	: testRuleURI,
		'startRule'	: "Default$Lot",
		'vertices'	: [  0, 0, 0,  0, 0, 20,  20, 0, 20,  20, 0, 0 ],
		'faces'		: [ [0, 1, 2, 3] ],
		'attributes': { }
	} ];

var callbacks = {
		'dataRoot': '/tmp/prt4njs',
		'generateBegin'	: function(uid) { },
		'shapeBegin' 	: function(uid) { },
		'shapeEnd'   	: function(result) { },
		'generateEnd'	: function(result) {

// spec for callback result
//			var result = {
//					'id' : '',
//					'options' : { },
//					'output': // output blocks as defined by simpleoutputcallback 'write' calls
//						[ 
//						 {
//							 'uids' : [ ], // one or more initial shape ids depending on encoder options
//							 'data' : "" // either an uri to the data or the data itself (???)
//						 },
//						 // next output block
//						 ] 
//			};

			console.log(result);
		}
};

var encodeInfo = undefined;

prt4njs.getEncoderInfo("com.esri.prt.codecs.OBJEncoder", function(info) {
	info.options.baseName = "njstest";
	console.log(util.inspect(info, false, null));
	encodeInfo = info;
});


var status = prt4njs.generate(initialShapes, encodeInfo, callbacks);

prt4njs.shutdown();
