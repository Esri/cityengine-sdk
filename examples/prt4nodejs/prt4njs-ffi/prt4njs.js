var ffi = require("node-ffi");

var prt = new ffi.Library(__dirname + "/../../../prt/bin/libcom.esri.prt.core", {
	"prt::getVersion": [ "pointer", [ "void" ]]
}) ;

prt.getVersion();