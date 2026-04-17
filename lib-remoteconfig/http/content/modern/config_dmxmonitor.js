window.dmxmonitor = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML = `
            <h2>${name}</h2>
            <form accept-charset="utf-8">
                <div class='row'>
                    <label>Start address</label>
                    <input data-key='dmx_start_address' type='number' min='1' max='512' required>
                </div>
                <div class='row'>
                    <label>Max channels</label>
                    <input data-key='dmx_max_channels' type='number' min='1' max='512' required>
                </div>
                <div class='row'>
                    <label>Format</label>
                    <select data-key='format'>
                        <option value='hex'>hex</option>
                        <option value='pct'>pct</option>
                        <option value='dec'>dec</option>
                    </select>
                </div>
                <div class='row'>
                    <label></label>
                    <button type='submit'>Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);

        div.querySelector("form").onsubmit = () => {
            this.save(path, div);
            return false;
        };

        this.fill(div, {
            dmx_start_address: json.dmx_start_address || 1,
            dmx_max_channels: json.dmx_max_channels || 16,
            format: json.format || "dec"
        });
    },

    fill: function(card, json) {
        const fields = card.querySelectorAll("[data-key]");
        fields.forEach(e => {
            const key = e.dataset.key;
            if (json[key] !== undefined) {
                e.value = json[key];
            }
        });
    },

    save: async function(path, card) {
        const fields = card.querySelectorAll("[data-key]");
        const out = {};

        for (let i = 0; i < fields.length; i++) {
            const e = fields[i];

            if (!e.checkValidity()) {
                e.reportValidity();
                return;
            }

            const key = e.dataset.key;
            out[key] = (e.type === "number") ? +e.value : e.value.trim();
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

            const json = await getJSON(path);
            if (!json) return;

            this.fill(card, json);
        } catch (e) {
            console.log("Error:", e);
        } finally {
            btn.disabled = false;
            btn.textContent = "Save";
        }
    }
};