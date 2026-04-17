function isCore(path) {
	return path === "status/identify"
	|| path === "status/display"
	|| path === "status/phy"
	|| path === "status/emac";
}

async function loadModules() {
	const j = await getJSON("status/directory");
	if (!j) return;
	
	const files = j.files;
	
	for (const path in files) {
		if (isCore(path)) continue;
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
	     '<button onclick="refresh(\'' + sel + '\', \'' + id + '\')">Refresh</button>' +
	     "</td></tr>";

	h += "</table>";

	document.getElementById(id).innerHTML = h;
}

async function getJSON(path) {
	try {
		const r = await fetch('/json/' + path);
		return r.ok ? await r.json() : null;
	} catch {
		return null;
	}
}

function setValue(id, value) {
    const el = document.getElementById(id);
    if (!el) return;

    if (el.type === "checkbox") {
        el.checked = !!value;
    } else if ("value" in el) {
        el.value = value ?? "";
    } else {
        el.textContent = value ?? "";
    }
}

async function loadCore(core) {
	console.log(core)
	const json = await getJSON('status/' + core);
	if (json) {
		Object.keys(json).forEach(function(key) {
			console.log(key + ':' + json[key])
			setValue(key, json[key]);
		});
	}
}

async function loadGeneral() {
    await loadCore('display');
    await loadCore('identify');
}

async function loadCores() {
    loadGeneral();
    await loadCore('phy');
	await loadCore('emac');
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