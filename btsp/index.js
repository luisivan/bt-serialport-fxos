var readline = require('readline'),
	serial = require('serialport')

var rl = readline.createInterface({
  	input: process.stdin,
  	output: process.stdout
})

rl.on('line', function (cmd) {
  	if (cmd == 's') {
  		console.log('Sending some data')
  		sendData()
  	} else if (cmd == 'c') {
  		console.log('Bye')
  		// Force it, otherwise it won't work
  		process.kill(process.pid, 'SIGKILL')
  	}
})

var serialPort

(function() {
	serialPort = new serial.SerialPort('/dev/rfcomm0', {
		baudrate: 57600
	})
	serialPort.on('data', function(data) {
	    console.log('Data received: ' + data)
	})
	console.log('Serial Port opened')
})()

function sendData() {
    serialPort.write('Plz receive this data\n', function(err, results) {
        console.log('Err is ' + err)
        // results always = 22, which must be this Bluetooth event:
        // #define L2CEVT_L2CA_CONNECT_RSP 22 /* Upper layer connect response */
        // It's sending a configuration response I guess
        console.log('Results are ' + results)
    })
}