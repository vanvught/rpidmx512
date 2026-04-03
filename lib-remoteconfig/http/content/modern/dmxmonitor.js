window.dmxmonitor = {
	init: async function(path, name) {
		const json = await getJSON(path);
		if (!json) return;

		const div = document.createElement("div");
		div.className = "card";
		div.innerHTML =
			"<h2>" + name + "</h2>" +
			"<form onsubmit=\"dmxmonitor.save('" + path + "'); return false;\">" +
				"<div class='row'>" +
					"<label>Start address</label>" +
					"<input id='dmx_start_address' type='number' min='1' max='512' required>" +
				"</div>" +
				"<div class='row'>" +
					"<label>Max channels</label>" +
					"<input id='dmx_max_channels' type='number' min='1' max='512' required>" +
				"</div>" +
				"<div class='row'>" +
					"<label>Format</label>" +
					"<select id='format'>" +
						"<option value='hex'>hex</option>" +
						"<option value='pct'>pct</option>" +
						"<option value='dec'>dec</option>" +
					"</select>" +
				"</div>" +
				"<div class='row'>" +
					"<label></label>" +
					"<button type='submit'>Save</button>" +
				"</div>" +
			"</form>";

		document.getElementById("modules").appendChild(div);

		document.getElementById("dmx_start_address").value = json.dmx_start_address || 1;
		document.getElementById("dmx_max_channels").value = json.dmx_max_channels || 16;
		document.getElementById("format").value = json.format || "dec";
	},

	save: function(path) {
		const start = document.getElementById("dmx_start_address");
		const max = document.getElementById("dmx_max_channels");

		if (!start.checkValidity()) {
			start.reportValidity();
			return;
		}

		if (!max.checkValidity()) {
			max.reportValidity();
			return;
		}

		fetch("json/" + path, {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				dmx_start_address: +start.value,
				dmx_max_channels: +max.value,
				format: document.getElementById("format").value
			})
		}).then(function(r) {
			if (!r.ok) {
				console.log("Save failed");
			}
		});
	}
};