/**
 * New node file
 */

const PORT = 1337;
const ADDRESS = '0.0.0.0';

var http = require('http');
var prt4njs = require('../prt4njs/build/Release/prt4njs');

var prtExtLib = __dirname + "/../../../prt/lib";
var prtLicLib = __dirname + "/../../../prt/bin/libflexnet_prt.dylib";
var prtLicType = "CityEngAdvFx";
var prtLicHost = "";
var prtLogLevel = 1;

if (prt4njs.init(prtExtLib, prtLicLib, prtLicType, prtLicHost, prtLogLevel) != 0)
	return;

function prtCleanup() {
	console.log("closing down");
	prt4njs.cleanup();
}

var server = http.createServer(function (req, res) {
	res.writeHead(200, {'Content-Type': 'text/plain'});
	res.end("prt4njs init");
});

server.listen(PORT, ADDRESS, function() {
	console.log('Server running at http://%s:%d/', ADDRESS, PORT);
	console.log('Press CTRL+C to exit');
	
    // Check if we are running as root
    if (process.getgid() === 0) {
      process.setgid('nobody');
      process.setuid('nobody');
    }
});

process.on('SIGTERM', function () {
	if (server === undefined) return;
	server.close(function () {
		prtCleanup();
		// Disconnect from cluster master
		process.disconnect && process.disconnect();
	});
});

process.on('SIGINT', function () {
	if (server === undefined) return;
	server.close(function () {
		prtCleanup();
		// Disconnect from cluster master
		process.disconnect && process.disconnect();
	});
});

process.on('uncaughtException', function(err) {
	console.log("caught exception: " + err);
});