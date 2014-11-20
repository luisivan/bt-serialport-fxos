var serial = require("serialport")

serial.list(function (err, ports) {

    if (err || ports.length == 0)
        return console.log ( (err) ? err : 'No serial ports available' )

    ports.forEach(function(port) {
        console.log(port.comName)
        console.log(port.pnpId)
        console.log(port.manufacturer)
    })
})

var serialPort = new 
serial.SerialPort("/dev/rfcomm0", {
    baudrate: 57600
})

serialPort.on("open", function () {

    console.log('Serial Port opened')
    serialPort.on('data', function(data) {
        console.log('Data received: ' + data)
    })
    serialPort.write("Plz receive this data\n", 
function(err, results) {
        console.log('Err is ' + err)
        console.log('Results are ' + results)
    })
})
