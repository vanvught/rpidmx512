window.display = {
	init: async function(path, name) {
		const j = await getJSON(path);
		if (!j) return;

		const optionalFields = [
			"title",
			"board_name",
			"version",
			"hostname",
			"ip_address",
			"net_mask",
			"default_gateway",
			"active_ports"
		];

		const labels = {
			title: "Title",
			board_name: "Board name",
			version: "Version",
			hostname: "Hostname",
			ip_address: "IP address",
			net_mask: "Net mask",
			default_gateway: "Default gateway",
			active_ports: "Active ports"
		};

		const rowAssignments = {};
		for (let i = 0; i < optionalFields.length; i++) {
			const key = optionalFields[i];
			const v = j[key];

			if (typeof v === "number" && v >= 1 && v <= 6) {
				rowAssignments[v] = key;
			}
		}

		const card = document.createElement("div");
		card.className = "card";

		let h = "<h2>" + name + "</h2>";
		h += "<form>";

		h += "<div class='row'>";
		h += "<label>Intensity</label>";
		h += "<input data-key='intensity' type='number' min='0' max='255' required>";
		h += "</div>";

		h += "<div class='row'>";
		h += "<label>Sleep timeout</label>";
		h += "<input data-key='sleep_timeout' type='number' min='0' max='60' required>";
		h += "</div>";

		h += "<div class='row checkbox'>";
		h += "<label>Flip vertically</label>";
		h += "<input data-key='flip_vertically' type='checkbox'>";
		h += "</div>";

		for (let row = 1; row <= 6; row++) {
			h += "<div class='row'>";
			h += "<label>Row " + row + "</label>";
			h += "<select data-row='" + row + "'>";
			h += "<option value=''>-</option>";

			for (let i = 0; i < optionalFields.length; i++) {
				const key = optionalFields[i];
				h += "<option value='" + key + "'>" + labels[key] + "</option>";
			}

			h += "</select>";
			h += "</div>";
		}

		h += "<div class='row'>";
		h += "<label></label>";
		h += "<button type='submit'>Save</button>";
		h += "</div>";

		h += "</form>";

		card.innerHTML = h;
		document.getElementById("modules").appendChild(card);

		for (let row = 1; row <= 6; row++) {
			const e = card.querySelector("select[data-row='" + row + "']");
			if (e && rowAssignments[row] !== undefined) {
				e.value = rowAssignments[row];
			}
		}

		const intensity = card.querySelector("[data-key='intensity']");
		const sleepTimeout = card.querySelector("[data-key='sleep_timeout']");
		const flipVertically = card.querySelector("[data-key='flip_vertically']");

		intensity.value = j.intensity ?? 128;
		sleepTimeout.value = j.sleep_timeout ?? 0;
		flipVertically.checked = !!j.flip_vertically;

		const form = card.querySelector("form");
		form.onsubmit = function() {
			window.display.save(path, card, optionalFields);
			return false;
		};
	},

	save: async function(path, card, optionalFields) {
		const intensity = card.querySelector("[data-key='intensity']");
		const sleepTimeout = card.querySelector("[data-key='sleep_timeout']");
		const flipVertically = card.querySelector("[data-key='flip_vertically']");
		const rowSelects = card.querySelectorAll("select[data-row]");

		if (!intensity.checkValidity()) {
			intensity.reportValidity();
			return;
		}

		if (!sleepTimeout.checkValidity()) {
			sleepTimeout.reportValidity();
			return;
		}

		const used = {};
		for (let i = 0; i < rowSelects.length; i++) {
			const e = rowSelects[i];
			if (!e.value) continue;

			if (used[e.value]) {
				alert("Each display field can only be assigned once.");
				e.focus();
				return;
			}
			used[e.value] = true;
		}

		const out = {
			intensity: +intensity.value,
			sleep_timeout: +sleepTimeout.value,
			flip_vertically: flipVertically.checked ? 1 : 0
		};

		for (let i = 0; i < optionalFields.length; i++) {
			out[optionalFields[i]] = "";
		}

		for (let i = 0; i < rowSelects.length; i++) {
			const e = rowSelects[i];
			const row = +e.dataset.row;

			if (e.value) {
				out[e.value] = row;
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

			intensity.value = j.intensity ?? 128;
			sleepTimeout.value = j.sleep_timeout ?? 0;
			flipVertically.checked = !!j.flip_vertically;

			for (let row = 1; row <= 6; row++) {
				const e = card.querySelector("select[data-row='" + row + "']");
				if (e) e.value = "";
			}

			for (let i = 0; i < optionalFields.length; i++) {
				const key = optionalFields[i];
				const v = j[key];

				if (typeof v === "number" && v >= 1 && v <= 6) {
					const e = card.querySelector("select[data-row='" + v + "']");
					if (e) e.value = key;
				}
			}
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