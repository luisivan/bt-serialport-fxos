var adapter

function test() {
	console.log('hi2')

	navigator.mozBluetooth.getDefaultAdapter().onsuccess = function(e) {
		adapter = e.target.result
		adapter.getPairedDevices().onsuccess = function(e) {
			var devices = e.target.result
			for (var i in devices) {
				if (devices[i].address == "a4:db:30:21:98:a3") {
					adapter.connect(devices[i])
					adapter.sendSerialPort("a4:db:30:21:98:a3", "ola k ase")
				}
			}
			
		}
		

		adapter.onserialportdata = function(e) {
			console.log(e)
			alert('onSerialPortData fired!')
		}
		adapter.sendSerialPort('address', 'oi')
	}
}

document.getElementsByTagName('button')[0].onclick = test
