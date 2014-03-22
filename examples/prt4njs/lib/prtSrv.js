/**
 * New node file
 */

const PORT = 1337;
const ADDRESS = '0.0.0.0';

var http = require('http');
var prt4njs = require('../prt4njs/build/Release/prt4njs');

var prtRoot		= __dirname + "/../../../prt";
var prtLicType = "CityEngAdvFx";
var prtLicHost = "";
var prtLogLevel = 1;

if (prt4njs.init(prtRoot, prtLicType, prtLicHost, prtLogLevel) != 0)
	return;

function prtCleanup() {
	console.log("closing down");
	prt4njs.shutdown();
}

var server = http.createServer(function (req, res) {
	res.writeHead(200, {'Content-Type': 'text/plain'});
	// TODO: return json with metadata (e.g. data link) for each generated feature
	// prt4njs will serve the actual data upon request
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