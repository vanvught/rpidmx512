async function loadField(id, path, key) {
	const json = await getJSON(path);
	if (json) setValue(id, json[key]);
}

async function saveField(id, path, key, done) {
	const ok = await postJSON(path, { [key]: $(id).value });
	if (ok && done) done();
}

async function display() {
	await loadField('display_name', 'config/remote', 'display_name');
	await loadField('utc_offset', 'config/global', 'utc_offset');
	loadNetwork();
}

async function loadNetwork() {
	const j = await getJSON('config/network');
	if (!j) return;

	const ids = ['hostname', 'ip_address', 'net_mask', 'default_gateway', 'ntp_server', 'secondary_ip', 'use_static_ip'];
	for (const id of ids) {
		const e = $(id);
		if (!e) continue;

		if (id === 'ntp_server' && !(id in j)) {
			e.value = 'Not supported.';
			e.disabled = true;
			continue;
		}
		if (id === 'ntp_server') e.disabled = false;
		setValue(id, j[id]);
	}
}

async function saveNetwork() {
	const required = ['hostname', 'ip_address', 'net_mask', 'default_gateway'];
	for (const id of required) {
		const e = $(id);
		if (!e.checkValidity()) {
			e.reportValidity();
			return;
		}
	}

	const data = {
		hostname: $('hostname').value,
		ip_address: $('ip_address').value,
		net_mask: $('net_mask').value,
		default_gateway: $('default_gateway').value,
		use_static_ip: $('use_static_ip').checked ? 1 : 0
	};

	const ntp = $('ntp_server').value.trim();
	if (ntp !== '' && !$('ntp_server').disabled) data.ntp_server = ntp;
	if (await postJSON('config/network', data)) loadNetwork();
}

function loadModules() {
	loadModulesFrom('config/directory', ['config/remote', 'config/global', 'config/network'], { readonly: false });
}

function fillDataKeys(root, data) {
	if (!data) return;

	root = root || document;

	const elements = root.querySelectorAll("[data-key]");

	for (let i = 0; i < elements.length; i++) {
		const el = elements[i];
		const value = data[el.dataset.key];

		if (value === undefined || value === null) {
			continue;
		}

		if (el.type === "checkbox") {
			el.checked = value === true || value === 1 || value === "1";
		} else {
			el.value = value;
		}
	}
}

function saveDataKeyForm(path, root) {
	const values = {};
	const elements = root.querySelectorAll("[data-key]");

	for (let i = 0; i < elements.length; i++) {
		const el = elements[i];

		values[el.dataset.key] =
			(el.type === "checkbox") ? (el.checked ? 1 : 0) : el.value;
	}

	postJSON(path, values);
}
