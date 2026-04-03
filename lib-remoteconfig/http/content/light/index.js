async function directory() {
	let d = await getJSON('config/directory')
	let h = ""
	let f = Object.keys(d["files"])
	f.forEach(function(key) {
		var v = d["files"][key]
		h += "<option value=" + key + ">" + v + "</option>"
	});
	document.getElementById("idDirectory").innerHTML = h
	get_txt(f[0])
}

async function get_txt(sel) {
	let txt = await getJSON(sel)
	let h = ""
	Object.keys(txt).forEach(function(key) {
		var v = txt[key]
		h += "<tr><td>" + key + '</td><td><input type="text" value="' + v + '" id="' + key + '"></td></tr>'
	});
	h += '<tr><td colspan="2"><button onclick="save(\'' + sel + '\')">Save</button>';
	h += '<button class="btn" onclick="reset(\'' + sel + '\')">Defaults</button></td></tr>';
	document.getElementById("idTxt").innerHTML = h
}

function save(sel) {
	var d = {}
	var inputs = document.getElementById("idTxt").getElementsByTagName("input")
	for (var i = 0; i < inputs.length; i++) {
		var k = inputs[i].id
		var v = inputs[i].value
		d[k] = v
	}
	var out = {}
	out = d
	var payload = JSON.stringify(out)
	fetch('/json/' + sel, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: payload
	}).then(response => { if (response.ok) { get_txt(sel); } });
}
