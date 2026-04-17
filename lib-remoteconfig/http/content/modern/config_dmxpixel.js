window.dmxpixel = {
    init: async function(path, name) {
        const j = await getJSON(path);
        if (!j) return;

        const ports = Object.keys(j).filter(k => k.startsWith("start_uni_port_"));

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML = `
			<h2>${name}</h2>
			<form>
				<div class='row'><label>Type</label><input data-key='type'></div>
				<div class='row'><label>Count</label><input data-key='count' type='number' min='1'></div>
				<div class='row'><label>T0H</label><input data-key='t0h'></div>
				<div class='row'><label>T1H</label><input data-key='t1h'></div>

				<div class='row'>
					<label>Map</label>
					<select data-key='map'>
						<option>RGB</option><option>RBG</option>
						<option>GRB</option><option>GBR</option>
						<option>BRG</option><option>BGR</option>
					</select>
				</div>

				<div class='row'><label>Clock</label><input data-key='clock_speed_hz' type='number'></div>
				<div class='row'><label>Brightness</label><input data-key='global_brightness' type='number' min='0' max='255'></div>
				<div class='row'><label>Group</label><input data-key='group_count' type='number' min='1'></div>
				<div class='row'><label>Active out</label><input data-key='active_out' type='number' min='1'></div>

				<div class='row universe-row'>
					<label>Universe</label>
					<div class='ports'></div>
				</div>

				<div class='row'><label>Test</label><input data-key='test_pattern' type='number'></div>

				<div class='row'>
					<label></label>
					<button type='submit'>Save</button>
				</div>
			</form>
		`;

        document.getElementById("modules").appendChild(div);

        const portsDiv = div.querySelector(".ports");
        if (ports.length) {
            ports.forEach((key, i) => {
                const p = document.createElement("div");
                p.className = "port";
                p.innerHTML = `
					<span>${i + 1}</span>
					<input data-key='${key}' type='number' min='0' max='32768'>
				`;
                portsDiv.appendChild(p);
            });
        } else {
            div.querySelector(".universe-row").style.display = "none";
        }

        div.querySelector("form").onsubmit = () => {
            this.save(path, div);
            return false;
        };

        this.fill(div, j);
    },

    fill: function(card, j) {
        const fields = card.querySelectorAll("[data-key]");
        fields.forEach(e => {
            const k = e.dataset.key;
            if (j[k] !== undefined) {
                e.value = j[k];
            }
        });
    },

    save: async function(path, card) {
        const fields = card.querySelectorAll("[data-key]");
        const out = {};

        for (let i = 0;i < fields.length;i++) {
            const e = fields[i];

            if (!e.checkValidity()) {
                e.reportValidity();
                return;
            }

            const k = e.dataset.key;

            if (e.type === "number") {
                out[k] = +e.value;
            } else {
                out[k] = e.value.trim();
            }
        }

        const btn = card.querySelector("button[type='submit']");
        btn.disabled = true;
        btn.textContent = "Saving...";

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

            this.fill(card, j);

        } catch (e) {
            console.log("Error:", e);
        } finally {
            btn.disabled = false;
            btn.textContent = "Save";
        }
    }
};