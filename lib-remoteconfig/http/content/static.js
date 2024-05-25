async function getJSON(json) {
	try {
		const r = await fetch('/json/'+json)
		if (!r.ok) {
			throw new Error('Error')
		}
		return r.json()
	} catch (error) {}
}

async function list() {
	const l = await getJSON('list')
	document.getElementById("idList").innerHTML = "<li>"+l.list.name+"</li><li>"+l.list.node.type+"</li><li>"+l.list.node.port.type+"</li>"
}

async function version() {
	const v = await getJSON('version')
	document.getElementById("idVersion").innerHTML = "<li>V"+v.version+"</li><li>"+v.build.date+"</li><li>"+v.build.time+"</li><li>"+v.board+"</li>"
}

function post(s) {
	return fetch('/json/action', {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: JSON.stringify(s)
	})
}

function delet(s) {
  return fetch('/json/action', {
    method: 'DELETE',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(s)
  })
}

function reboot() {
	post({ reboot: 1 })
}

function locate() {
	var b = document.getElementById('locateButton');
	if (b.classList.contains('inactive')) {
		b.classList.remove('inactive')
		b.classList.add('active')
		b.innerHTML = 'Locate On'
		post({ identify: 1 })
	} else {
		b.classList.remove('active')
		b.classList.add('inactive')
		b.innerHTML = 'Locate Off'
		post({ identify: 0 })
	}
}