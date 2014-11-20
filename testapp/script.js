var adapter

navigator.mozBluetooth.getDefaultAdapter().onsuccess = function(e) {
	adapter = e.target.result
	adapter.getPairedDevices().onsuccess = function(e) {
		var devices = e.target.result
		for (var i in devices) {
			var button = document.createElement("button")
		    button.textContent = devices[i].name
		    button.onclick = send.bind(undefined, devices[i])
		    document.body.appendChild(button)
		}
	}
	adapter.onserialportdata = function(e) {
		console.log(e)
		alert('onSerialPortData fired!')
	}
}

function send(device) {
	console.log('Sending to ' + device.name)
	adapter.connect(device, 0x1101)
	adapter.sendSerialPort(device.address, "ola k ase")
}