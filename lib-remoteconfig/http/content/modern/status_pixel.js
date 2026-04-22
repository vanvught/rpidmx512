window.pixel = {
	async init(path, name) {
		let data;
		try {
			data = await getJSON(path);
		} catch (e) {
			console.error('Pixel status load failed', e);
			return;
		}

		const container = document.getElementById('modules');
		if (!container) return;

		const card = document.createElement('div');
		card.className = 'card';

		card.innerHTML =		
			'<h2>' + name + '</h2>' +
			'<div class="row"><label>Max FPS</label><span>' + data.refresh_rate + '</span></div>' +
			'<div class="row"><label>Current FPS</label><span>' + data.frame_rate + '</span></div>' +
			'</div>';

		container.appendChild(card);
	}
};
