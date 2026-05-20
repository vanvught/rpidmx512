(function injectDmxStyle() {
	if (document.getElementById('dmx-style')) return;

	const style = document.createElement('style');
	style.id = 'dmx-style';

	style.textContent =
		'.dmx-table {' +
			'width:100%;border-collapse:collapse;font-size:12px;' +
		'}' +
		'.dmx-table th,.dmx-table td {' +
			'border:1px solid #555;padding:4px 6px;text-align:center;' +
		'}' +
		'.dmx-table th {' +
			'background:#2a2a2a;color:#ccc;' +
		'}';

	document.head.appendChild(style);
})();

window.dmx = {
	_path: '',
	_name: '',

	async init(path, name) {
		this._path = path;
		this._name = name;
		await this.load();
	},

	async load() {
		let data;
		try {
			data = await getJSON(this._path);
		} catch (e) {
			console.error('DMX status load failed', e);
			return;
		}

		const container = document.getElementById('modules');
		if (!container) return;

		let card = document.getElementById('status-dmx-card');
		if (!card) {
			card = document.createElement('div');
			card.id = 'status-dmx-card';
			card.className = 'card';
			container.appendChild(card);
		}

		let html =
			'<h2>' + this._name + '</h2>' +
			'<table class="dmx-table">' +
				'<thead>' +
					'<tr>' +
						'<th>Port</th>' +
						'<th>DMX Sent</th>' +
						'<th>DMX Received</th>' +
						'<th>RDM Good</th>' +
						'<th>RDM Bad</th>' +
						'<th>RDM Disc Resp</th>' +
						'<th>RDM Sent Classes</th>' +
						'<th>RDM Sent Disc Resp</th>' +
					'</tr>' +
				'</thead>' +
				'<tbody>';

		for (const p of data) {
			const dmx = p.dmx || {};
			const rdm = p.rdm || {};
			const rdmSent = rdm.sent || {};
			const rdmRecv = rdm.received || {};

			html +=
				'<tr>' +
					td(p.port) +
					td(dmx.sent) +
					td(dmx.received) +
					td(rdmRecv.good) +
					td(rdmRecv.bad) +
					td(rdmRecv.discovery) +
					td(rdmSent.class) +
					td(rdmSent.discovery) +
				'</tr>';
		}

		html +=
			'</tbody></table>' +
			'<div class="row"><button onclick="dmx.load()">Refresh</button></div>';

		card.innerHTML = html;
	}
};

function td(v) {
	return '<td>' + (v ?? '0') + '</td>';
}
