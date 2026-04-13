window.dmxnode = {
	init: async function(path, name) {
		const j = await getJSON(path);
		if (!j) return;

		const suffixes = ["a", "b", "c", "d"].filter(function(suffix) {
			return j["label_port_" + suffix] !== undefined ||
				j["universe_port_" + suffix] !== undefined ||
				j["direction_port_" + suffix] !== undefined ||
				j["merge_mode_port_" + suffix] !== undefined;
		});

		const card = document.createElement("div");
		card.className = "card";

		let h = "<h2>" + name + "</h2>";
		h += "<form>";

		h += "<div class='row'>";
		h += "<label>Node name</label>";
		h += "<input data-key='node_name' maxlength='64' pattern='[ -~]{1,64}' required>";
		h += "</div>";

		h += "<div class='row'>";
		h += "<label>Failsafe</label>";
		h += "<select data-key='failsafe'>";
		h += "<option value='hold'>Hold</option>";
		h += "<option value='on'>On</option>";
		h += "<option value='off'>Off</option>";
		h += "<option value='playback'>Playback</option>";
		h += "</select>";
		h += "</div>";

		h += "<div class='row checkbox'>";
		h += "<label>Disable merge timeout</label>";
		h += "<input data-key='disable_merge_timeout' type='checkbox'>";
		h += "</div>";

		for (let i = 0; i < suffixes.length; i++) {
			const suffix = suffixes[i];

			h += "<div class='row'>";
			h += "<label>Port " + suffix.toUpperCase() + "</label>";

			if (j["label_port_" + suffix] !== undefined) {
				h += "<input data-key='label_port_" + suffix + "' maxlength='18' pattern='[ -~]{1,18}' required>";
			}

			if (j["universe_port_" + suffix] !== undefined) {
				h += "<input data-key='universe_port_" + suffix + "' type='number' min='0' max='32768'>";
			}

			if (j["direction_port_" + suffix] !== undefined) {
				h += "<select data-key='direction_port_" + suffix + "'>";
				h += "<option value='output'>output</option>";
				h += "<option value='input'>input</option>";
				h += "<option value='disable'>disable</option>";
				h += "</select>";
			}

			if (j["merge_mode_port_" + suffix] !== undefined) {
				h += "<select data-key='merge_mode_port_" + suffix + "'>";
				h += "<option value='htp'>htp</option>";
				h += "<option value='ltp'>ltp</option>";
				h += "</select>";
			}

			h += "</div>";
		}

		h += "<div class='row'>";
		h += "<label></label>";
		h += "<button type='submit'>Save</button>";
		h += "</div>";

		h += "</form>";

		card.innerHTML = h;
		document.getElementById("modules").appendChild(card);

		const form = card.querySelector("form");
		form.onsubmit = function() {
			window.dmxnode.save(path, card);
			return false;
		};

		window.dmxnode.fill(card, j);
	},

	fill: function(card, j) {
		const fields = card.querySelectorAll("[data-key]");

		for (let i = 0; i < fields.length; i++) {
			const e = fields[i];
			const key = e.dataset.key;

			if (j[key] === undefined) continue;

			if (e.type === "checkbox") {
				e.checked = !!j[key];
			} else {
				e.value = j[key];
			}
		}
	},

	save: async function(path, card) {
		const fields = card.querySelectorAll("[data-key]");
		const out = {};

		for (let i = 0; i < fields.length; i++) {
			const e = fields[i];
			const key = e.dataset.key;

			if (!e.checkValidity()) {
				e.reportValidity();
				return;
			}

			if (e.type === "checkbox") {
				out[key] = e.checked ? 1 : 0;
			} else if (e.type === "number") {
				out[key] = +e.value;
			} else {
				out[key] = e.value.trim();
			}
		}

		const btn = card.querySelector("button[type='submit']");
		if (btn) {
			btn.disabled = true;
			btn.textContent = "Saving...";
		}

		try {
			const res = await fetch("json/" + path, {
				method: "POST",
				headers: { "Content-Type": "application/json" },
				body: JSON.stringify(out)
			});

			if (!res.ok) {
				console.log("Save failed");
				return;
			}

			const j = await getJSON(path);
			if (!j) return;

			window.dmxnode.fill(card, j);
		} catch (e) {
			console.log("Error:", e);
		} finally {
			if (btn) {
				btn.disabled = false;
				btn.textContent = "Save";
			}
		}
	}
};