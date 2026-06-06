(function() {
	function navItem(href, label, active) {
		return '<li><a href="' + href + '"' + (active ? ' class="active"' : '') + '>' + label + '</a></li>';
	}

	function renderNav(activePage) {
		const el = document.getElementById('appNav');
		if (!el) return;

		const pages = window.UI_PAGES || [
			['/config', 'Configuration', 'config'],
			['/status', 'Status', 'status'],
			['/time', 'Time', 'time'],
			['/rtc', 'RTC', 'rtc'],
			['/upload', 'Firmware', 'upload']
		];

		let html = '<nav><ul>';
		for (const p of pages) {
			html += navItem(p[0], p[1], activePage === p[2]);
		}
		html += '</ul></nav>';
		el.innerHTML = html;
	}

	function renderHeader() {
		const el = document.getElementById('appHeader');
		if (el) el.innerHTML = '<header><ul id="idList"></ul></header>';
	}

	function renderFooter() {
		const el = document.getElementById('appFooter');
		if (el) el.innerHTML = '<footer><ul id="idVersion"></ul></footer>';
	}

	function renderActions() {
		const el = document.getElementById('appActions');
		if (!el) return;
		el.innerHTML = '<div><button id="locateButton" class="inactive" onclick="locate()">Locate Off</button><button onclick="reboot()">Reboot</button></div>';
	}

	window.bootPage = function(activePage) {
		renderNav(activePage);
		renderHeader();
		renderFooter();
		renderActions();
	};
})();
