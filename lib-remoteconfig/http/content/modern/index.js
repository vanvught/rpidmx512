async function getJSON(path) {
	try {
		const r = await fetch('/json/' + path);
		return r.ok ? await r.json() : null;
	} catch {
		return null;
	}
}

async function postJSON(path, obj) {
	try {
		const r = await fetch(path, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(obj)
		});
		return r.ok;
	} catch {
		return false;
	}
}

function setValue(id, value) {
	document.getElementById(id).value = value || "";
}

async function loadField(id, path, key) {
	const json = await getJSON(path);
	if (json) {
		setValue(id, json[key]);
	}
}

async function saveField(id, path, key, done) {
	const ok = await postJSON(path, { [key]: document.getElementById(id).value });
	if (ok && done) {
		done();
	}
}

async function display() {
	await loadField('display_name', 'config/remote', 'display_name');
	await loadField('utc_offset', 'config/global', 'utc_offset');
	loadNetwork()
}

async function loadNetwork() {
	const j = await getJSON('config/network');
	if (!j) return;

	const ids = [
		"hostname",
		"ip_address",
		"net_mask",
		"default_gateway",
		"ntp_server",
		"secondary_ip",
		"use_static_ip"
	];

	for (const id of ids) {
		const e = document.getElementById(id);
		if (!e) continue;

		if (id === "ntp_server") {
			if (!(id in j)) {
				e.value = "Not supported.";
				e.disabled = true;
				continue;
			} else {
				e.disabled = false;
			}
		}

		if (e.type === "checkbox") {
			e.checked = !!j[id];
		} else {
			e.value = j[id] || "";
		}
	}
}

async function saveNetwork() {
	const ids = ["hostname", "ip_address", "net_mask", "default_gateway"];

	for (const id of ids) {
		const e = document.getElementById(id);
		if (!e.checkValidity()) {
			e.reportValidity();
			return;
		}
	}

	const o = {
		hostname: document.getElementById("hostname").value,
		ip_address: document.getElementById("ip_address").value,
		net_mask: document.getElementById("net_mask").value,
		default_gateway: document.getElementById("default_gateway").value,
		use_static_ip: document.getElementById("use_static_ip").checked ? 1 : 0
	};

	const ntp = document.getElementById("ntp_server").value.trim();
	if (ntp !== "") {
		o.ntp_server = ntp;
	}

	const ok = await postJSON("json/config/network", o);
	
	if (ok) {loadNetwork()}
}

async function loadModules() {
	const j = await getJSON("config/directory");
	if (!j) return;

	const files = j.files;

	for (const path in files) {
		if (isCore(path)) continue;  // skip core

		loadModule(path, files[path]);
	}
}

function loadModule(path, name) {
	const file = path.split("/")[1];
	const script = document.createElement("script");

	script.src = path + ".js";

	script.onload = () => {
		if (window[file] && window[file].init) {
			window[file].init(path, name);
		}
	};

	script.onerror = () => {
		console.log("Fallback for", file);

		const id = "mod_" + file;

		const div = document.createElement("div");
		div.className = "card";
		div.innerHTML =
			"<h2>" + name + "</h2>" +
			"<div id='" + id + "'></div>";

		document.getElementById("modules").appendChild(div);

		get(path, id);
	};

	document.body.appendChild(script);
}

function escapeHtml(s) {
	return String(s)
		.replace(/&/g, "&amp;")
		.replace(/"/g, "&quot;")
		.replace(/</g, "&lt;")
		.replace(/>/g, "&gt;");
}

async function get(sel, id) {
	const txt = await getJSON(sel);
	if (!txt) return;

	let h = "<table>";

	Object.keys(txt).forEach(function(key) {
		const v = txt[key] ?? "";

		h += "<tr><td>" + key + "</td><td>" +
		     '<input type="text" id="' + key + '" value="' + escapeHtml(v) + '">' +
		     "</td></tr>";
	});

	h += '<tr><td colspan="2">' +
	     '<button onclick="save(\'' + sel + '\', \'' + id + '\')">Save</button>' +
	     "</td></tr>";

	h += "</table>";

	document.getElementById(id).innerHTML = h;
}

function save(sel, id) {
	const inputs = document.getElementById(id).getElementsByTagName("input");

	const d = {};

	for (let i = 0; i < inputs.length; i++) {
		d[inputs[i].id] = inputs[i].value;
	}

	fetch('/json/' + sel, {
		method: 'POST',
		headers: { 'Content-Type': 'application/json' },
		body: JSON.stringify(d)
	}).then(r => {
		if (r.ok) get(sel, id);
	});
}

function isCore(path) {
	return path === "config/remote"
		|| path === "config/global"
		|| path === "config/network";
}

async function list() {
	const l = await getJSON('list');
	if (l) {
		document.getElementById("idList").innerHTML =
			`<li>${l.list.name}</li><li>${l.list.node.type}</li><li>${l.list.node.output.type}</li>`;
	}
}

async function version() {
	const v = await getJSON('version');
	if (v) {
		document.getElementById("idVersion").innerHTML =
			"<li>V" + v.version + "</li><li>" +
			v.build.date + "</li><li>" +
			v.build.time + "</li><li>" +
			v.board + "</li>";
	}
}

function post(u, j) {
    return fetch('/json/' + u, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(j)
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
    post('action', { reboot: 1 })
}

function locate() {
    var b = document.getElementById('locateButton');
    if (b.classList.contains('inactive')) {
        b.classList.remove('inactive')
        b.classList.add('active')
        b.innerHTML = 'Locate On'
        post('action', { identify: 1 })
    } else {
        b.classList.remove('active')
        b.classList.add('inactive')
        b.innerHTML = 'Locate Off'
        post('action', { identify: 0 })
    }
}

function reset(sel) {
    var d = {};
    var out = {};
    out[sel] = d;
    var payload = JSON.stringify(out);
    fetch('/json/' + sel, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: payload
    }).then(response => { if (response.ok) { get_txt(sel); } });
}