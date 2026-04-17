window.rdmdevice = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Label</label>
                    <input data-key="label" maxlength="32" pattern=".{1,32}" required>
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
            label: json.label || ""
        });
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

            out[e.dataset.key] = e.value.trim();
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