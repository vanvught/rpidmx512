window.showfile = {
	async init(path, name) {
		let data;
		try {
			data = await getJSON(path);
		} catch (e) {
			console.error('Showfile status load failed', e);
			return;
		}

		const container = document.getElementById('modules');
		if (!container) return;

		const card = document.createElement('div');
		card.className = 'card';

		card.innerHTML =
			'<h2>' + name + '</h2>' +
			'<div class="row"><label>Mode</label><span>' + valueOf(data.mode) + '</span></div>' +
			'<div class="row"><label>Show</label><span>' + valueOf(data.show) + '</span></div>' +
			'<div class="row"><label>Status</label><span>' + valueOf(data.status) + '</span></div>' +
			'<div class="row checkbox">' +
				'<label></label>' +
				'<input type="checkbox" disabled ' + checkedAttr(data.loop) + '>' +
				'<span>Loop</span>' +
			'</div>';

		container.appendChild(card);
	}
};

function valueOf(v) {
	return v ?? '';
}

function checkedAttr(v) {
	return (v === 1 || v === '1' || v === true || v === 'true') ? 'checked' : '';
}