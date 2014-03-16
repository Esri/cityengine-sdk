var prt4njs = require('../prt4njs/build/Release/prt4njs');

var prtRoot		= __dirname + "/../../../prt";
var prtLicType	= "CityEngAdvFx";
var prtLicHost	= "";
var prtLogLevel = 3;

if (prt4njs.init(prtRoot, prtLicType, prtLicHost, prtLogLevel) != 0)
	return;

var info = prt4njs.ruleInfo("rpk:file:/Volumes/Data/Users/shaegler/Documents/esri/dev/prt_trunk/com.esri.prt.test/resources/rules/candler.01.rpk!/bin/candler.01.cgb");

console.log("-- RULES (" + info.getNumRules() + ")");
for (var ri = 0; ri < info.getNumRules(); ri++) {
	var rule = info.getRule(ri);
	var msg = "   " + rule.getName() + "(";
	for (var pi = 0; pi < rule.getNumParameters(); pi++) {
		var param = rule.getParameter(pi);
		msg += (pi > 0 ? ", " : "") + param.getType() + " " + param.getName();
	}
	msg += ")";
	console.log(msg);
}

console.log("-- ATTRS (" + info.getNumAttributes() + ")");
for (var ai = 0; ai < info.getNumAttributes(); ai++) {
	var attr = info.getAttribute(ai);
	console.log("   " + attr.getReturnType() + " " + attr.getName());
}

prt4njs.cleanup();
