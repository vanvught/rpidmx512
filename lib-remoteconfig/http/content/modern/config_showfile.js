window.showfile = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Show</label>
                    <input data-key="show" type="number" min="0" max="99" required>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="auto_play" type="checkbox">
                    <span>Auto play</span>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="loop" type="checkbox">
                    <span>Loop</span>
                </div>
                <div class="row">
                    <label>Incoming port</label>
                    <input data-key="incoming_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label>Outgoing port</label>
                    <input data-key="outgoing_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);

        const form = div.querySelector("form");
        form.onsubmit = () => {
            this.save(path, div);
            return false;
        };

        this.fill(div, {
            show: json.show ?? 0,
            auto_play: json.auto_play ?? 0,
            loop: json.loop ?? 0,
            incoming_port: json.incoming_port ?? 8000,
            outgoing_port: json.outgoing_port ?? 9000
        });
    },

    fill: function(card, json) {
        const fields = card.querySelectorAll("[data-key]");

        for (let i = 0; i < fields.length; i++) {
            const e = fields[i];
            const key = e.dataset.key;

            if (json[key] === undefined) {
                continue;
            }

            if (e.type === "checkbox") {
                e.checked = !!json[key];
            } else {
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

            if (e.type === "checkbox") {
                out[e.dataset.key] = e.checked ? 1 : 0;
            } else if (e.type === "number") {
                out[e.dataset.key] = +e.value;
            } else {
                out[e.dataset.key] = e.value.trim();
            }
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

            this.fill(card, {
                show: json.show ?? 0,
                auto_play: json.auto_play ?? 0,
                loop: json.loop ?? 0,
                incoming_port: json.incoming_port ?? 8000,
                outgoing_port: json.outgoing_port ?? 9000
            });
        } catch (e) {
            console.log("Error:", e);
        } finally {
            btn.disabled = false;
            btn.textContent = "Save";
        }
    }
};