var adapter

function test() {
	console.log('hi2')

	navigator.mozBluetooth.getDefaultAdapter().onsuccess = function(e) {
		console.log(e)
		adapter = e.target.result
		if (typeof adapter.sendSerialPort == 'function')
			alert('sendSerialPort is defined!')
	}
}

document.getElementsByTagName('button')[0].onclick = test
