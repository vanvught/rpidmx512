function $(id) { return document.getElementById(id); }

async function getJSON(path) {
	try {
		const r = await fetch('/json/' + path);
		return r.ok ? await r.json() : null;
	} catch {
		return null;
	}
}

async function postJSON(path, data) {
	try {
		const r = await fetch('/json/' + path, {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(data)
		});
		return r.ok;
	} catch {
		return false;
	}
}

function setValue(id, value) {
	const el = $(id);
	if (!el) return;

	if (el.type === 'checkbox') {
		el.checked = !!value;
	} else if ('value' in el) {
		el.value = value ?? '';
	} else {
		el.textContent = value ?? '';
	}
}

function escapeHtml(s) {
	return String(s)
		.replace(/&/g, '&amp;')
		.replace(/"/g, '&quot;')
		.replace(/</g, '&lt;')
		.replace(/>/g, '&gt;');
}

async function list() {
	const l = await getJSON('list');
	if (!l || !$('idList')) return;
	$('idList').innerHTML =
		`<li>${escapeHtml(l.list.name)}</li>` +
		`<li>${escapeHtml(l.list.node.type)}</li>` +
		`<li>${escapeHtml(l.list.node.output.type)}</li>`;
}

async function version() {
	const v = await getJSON('version');
	if (!v || !$('idVersion')) return;
	$('idVersion').innerHTML =
		'<li>V' + escapeHtml(v.version) + '</li><li>' +
		escapeHtml(v.build.date) + '</li><li>' +
		escapeHtml(v.build.time) + '</li><li>' +
		escapeHtml(v.board) + '</li>';
}

function reboot() {
	postJSON('action', { reboot: 1 });
}

function locate() {
	const b = $('locateButton');
	if (!b) return;

	const on = b.classList.contains('inactive');
	b.classList.toggle('inactive', !on);
	b.classList.toggle('active', on);
	b.innerHTML = on ? 'Locate On' : 'Locate Off';
	postJSON('action', { identify: on ? 1 : 0 });
}

function isCorePath(path, corePaths) {
	return corePaths.indexOf(path) >= 0;
}

async function loadModulesFrom(directory, corePaths, options) {
	const j = await getJSON(directory);
	if (!j) return;

	const files = j.files || {};
	for (const path in files) {
		if (isCorePath(path, corePaths)) continue;
		loadModule(path, files[path], options || {});
	}
}

function loadModule(path, name, options) {
	const file = path.split('/')[1];
	const script = document.createElement('script');
	script.src = path + '.js';

	script.onload = () => {
		if (window[file] && window[file].init) {
			window[file].init(path, name);
		}
	};

	script.onerror = () => {
		const id = 'mod_' + file;
		const div = document.createElement('div');
		div.className = 'card';
		div.innerHTML = '<h2>' + escapeHtml(name) + '</h2><div id="' + id + '"></div>';
		$('modules').appendChild(div);
		renderJsonTable(path, id, options || {});
	};

	document.body.appendChild(script);
}

async function renderJsonTable(path, id, options) {
	const json = await getJSON(path);
	if (!json) return;

	const readonly = !!options.readonly;
	let html = '<table>';
	Object.keys(json).forEach(function(key) {
		const value = json[key] ?? '';
		html += '<tr><td>' + escapeHtml(key) + '</td><td>';
		if (readonly) {
			html += '<span id="' + escapeHtml(key) + '">' + escapeHtml(value) + '</span>';
		} else {
			html += '<input type="text" id="' + escapeHtml(key) + '" value="' + escapeHtml(value) + '">';
		}
		html += '</td></tr>';
	});

	const label = readonly ? 'Refresh' : 'Save';
	const action = readonly ? 'refreshJsonTable' : 'saveJsonTable';
	html += '<tr><td colspan="2"><button onclick="' + action + '(\'' + path + '\',\'' + id + '\')">' + label + '</button></td></tr></table>';
	$(id).innerHTML = html;
}

function refreshJsonTable(path, id) {
	renderJsonTable(path, id, { readonly: true });
}

async function saveJsonTable(path, id) {
	const inputs = $(id).getElementsByTagName('input');
	const data = {};
	for (let i = 0; i < inputs.length; i++) {
		data[inputs[i].id] = inputs[i].value;
	}
	if (await postJSON(path, data)) {
		renderJsonTable(path, id, { readonly: false });
	}
}
