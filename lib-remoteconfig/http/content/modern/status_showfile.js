window.showfile = {
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
			console.error('Showfile status load failed', e);
			return;
		}

		const container = document.getElementById('modules');
		if (!container) return;

		let card = document.getElementById('status-showfile-card');
		if (!card) {
			card = document.createElement('div');
			card.id = 'status-showfile-card';
			card.className = 'card';
			container.appendChild(card);
		}

		card.innerHTML =
			'<h2>' + this._name + '</h2>' +
			'<div class="row"><label>Mode</label><span>' + valueOf(data.mode) + '</span></div>' +
			'<div class="row"><label>Show</label><span>' + valueOf(data.show) + '</span></div>' +
			'<div class="row"><label>Status</label><span>' + valueOf(data.status) + '</span></div>' +
			'<div class="row checkbox">' +
				'<label></label>' +
				'<input type="checkbox" disabled ' + checkedAttr(data.loop) + '>' +
				'<span>Loop</span>' +
			'</div>' +
			'<div class="row"><button onclick="showfile.load()">Refresh</button></div>';
	}
};

function valueOf(v) {
	return v ?? '';
}

function checkedAttr(v) {
	return (v === 1 || v === '1' || v === true || v === 'true') ? 'checked' : '';
}
