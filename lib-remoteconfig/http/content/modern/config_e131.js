window.e131 = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const keys = Object.keys(json).filter(function(k) {
            return k.indexOf("priority_port_") === 0;
        });

        if (!keys.length) return;

        // Create card
        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Priority</label>
                    <div class="ports"></div>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);

        const portsContainer = div.querySelector(".ports");

        // Dynamically create ports
        for (let i = 0; i < keys.length; i++) {
            const key = keys[i];
            const suffix = key.substring(key.length - 1).toUpperCase();

            const portDiv = document.createElement("div");
            portDiv.className = "port";

            const span = document.createElement("span");
            span.textContent = suffix;

            const input = document.createElement("input");
            input.type = "number";
            input.min = "100";
            input.max = "200";
            input.required = true;
            input.dataset.key = key;

            portDiv.appendChild(span);
            portDiv.appendChild(input);
            portsContainer.appendChild(portDiv);
        }

        // Bind submit
        const form = div.querySelector("form");
        form.onsubmit = () => {
            this.save(path, div);
            return false;
        };

        // Fill values
        this.fill(div, json);
    },

    fill: function(card, json) {
        const fields = card.querySelectorAll("[data-key]");

        for (let i = 0; i < fields.length; i++) {
            const e = fields[i];
            const key = e.dataset.key;

            if (json[key] !== undefined) {
                e.value = json[key];
            }
        }
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

            out[e.dataset.key] = +e.value;
        }

        const btn = card.querySelector("button[type='submit']");
        btn.disabled = true;
        btn.textContent = "Saving...";

        try {
            const r = await fetch("json/" + path, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(out)
            });

            if (!r.ok) {
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