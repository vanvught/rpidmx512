window.showfile = {
	init: async function(path, name) {
		const json = await getJSON(path);
		if (!json) return;

		const div = document.createElement("div");
		div.className = "card";
		div.innerHTML =
			"<h2>" + name + "</h2>" +
			"<form onsubmit=\"showfile.save('" + path + "'); return false;\">" +

				"<div class='row'>" +
					"<label>Show</label>" +
					"<input id='show' type='number' min='0' max='99' required>" +
				"</div>" +

				"<div class='row checkbox'>" +
					"<label></label>" +
					"<input type='checkbox' id='auto_play'>" +
					"<span>Auto play</span>" +
				"</div>" +

				"<div class='row checkbox'>" +
					"<label></label>" +
					"<input type='checkbox' id='loop'>" +
					"<span>Loop</span>" +
				"</div>" +

				"<div class='row'>" +
					"<label>Incoming port</label>" +
					"<input id='incoming_port' type='number' min='1024' max='65535' required>" +
				"</div>" +

				"<div class='row'>" +
					"<label>Outgoing port</label>" +
					"<input id='outgoing_port' type='number' min='1024' max='65535' required>" +
				"</div>" +

				"<div class='row'>" +
					"<label></label>" +
					"<button type='submit'>Save</button>" +
				"</div>" +

			"</form>";

		document.getElementById("modules").appendChild(div);

		// load values
		document.getElementById("show").value = json.show ?? 0;
		document.getElementById("auto_play").checked = !!json.auto_play;
		document.getElementById("loop").checked = !!json.loop;
		document.getElementById("incoming_port").value = json.incoming_port ?? 8000;
		document.getElementById("outgoing_port").value = json.outgoing_port ?? 9000;
	},

	save: function(path) {
		const show = document.getElementById("show");
		const in_p = document.getElementById("incoming_port");
		const out_p = document.getElementById("outgoing_port");

		if (!show.checkValidity()) { show.reportValidity(); return; }
		if (!in_p.checkValidity()) { in_p.reportValidity(); return; }
		if (!out_p.checkValidity()) { out_p.reportValidity(); return; }

		fetch("json/" + path, {
			method: "POST",
			headers: { "Content-Type": "application/json" },
			body: JSON.stringify({
				show: +show.value,
				auto_play: document.getElementById("auto_play").checked ? 1 : 0,
				loop: document.getElementById("loop").checked ? 1 : 0,
				incoming_port: +in_p.value,
				outgoing_port: +out_p.value
			})
		}).then(function(r) {
			if (!r.ok) {
				console.log("Save failed");
			}
		});
	}
};