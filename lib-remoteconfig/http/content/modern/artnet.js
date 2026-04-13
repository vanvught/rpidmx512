window.artnet = {
	init: async function(path, name) {
		const j = await getJSON(path);
		if (!j) return;

		const suffixes = ["a", "b", "c", "d"].filter(function(suffix) {
			return j["protocol_port_" + suffix] !== undefined ||
				j["rdm_enable_port_" + suffix] !== undefined ||
				j["destination_ip_port_" + suffix] !== undefined;
		});

		const card = document.createElement("div");
		card.className = "card";

		let h = "<h2>" + name + "</h2>";
		h += "<form>";

		h += "<div class='row'>";
		h += "<label>Map universe 0</label>";
		h += "<input data-key='map_universe0' type='checkbox'>";
		h += "</div>";

		if (j.enable_rdm !== undefined) {
			h += "<div class='row checkbox'>";
			h += "<label>Enable RDM</label>";
			h += "<input data-key='enable_rdm' type='checkbox'>";
			h += "</div>";
		}

		for (let i = 0; i < suffixes.length; i++) {
			const suffix = suffixes[i];

			h += "<div class='row'>";
			h += "<label>Port " + suffix.toUpperCase() + "</label>";

			if (j["protocol_port_" + suffix] !== undefined) {
				h += "<select data-key='protocol_port_" + suffix + "'>";
				h += "<option value='artnet'>artnet</option>";
				h += "<option value='sacn'>sacn</option>";
				h += "<option value='disabled'>disabled</option>";
				h += "</select>";
			}

			if (j["rdm_enable_port_" + suffix] !== undefined) {
				h += "<label style='width:auto; margin-left:8px;'>RDM</label>";
				h += "<input data-key='rdm_enable_port_" + suffix + "' type='checkbox'>";
			}

			if (j["destination_ip_port_" + suffix] !== undefined) {
				h += "<label style='width:auto; margin-left:8px;'>Destination IP</label>";
				h += "<input data-key='destination_ip_port_" + suffix + "' pattern='(\\d{1,3}\\.){3}\\d{1,3}'>";
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
			window.artnet.save(path, card);
			return false;
		};

		window.artnet.fill(card, j);
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

			window.artnet.fill(card, j);
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