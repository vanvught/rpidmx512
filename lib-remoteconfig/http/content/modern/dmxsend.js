window.dmxsend = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Break time</label>
                    <input data-key="break_time" type="number" min="92" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>MAB time</label>
                    <input data-key="mab_time" type="number" min="12" max="1000000" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>Refresh rate</label>
                    <input data-key="refresh_rate" type="number" min="1" required>
                    <span>Hz</span>
                </div>
                <div class="row">
                    <label>Slots count</label>
                    <input data-key="slots_count" type="number" min="2" max="512" step="2" required>
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
            break_time: json.break_time ?? 176,
            mab_time: json.mab_time ?? 12,
            refresh_rate: json.refresh_rate ?? 40,
            slots_count: json.slots_count ?? 512
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
        let slotsCount = null;

        for (let i = 0; i < fields.length; i++) {
            const e = fields[i];
            const key = e.dataset.key;

            e.setCustomValidity("");

            if (!e.checkValidity()) {
                e.reportValidity();
                return;
            }

            const value = +e.value;
            out[key] = value;

            if (key === "slots_count") {
                slotsCount = e;
            }
        }

        if (slotsCount) {
            const slots = out.slots_count;

            if ((slots & 1) !== 0) {
                slotsCount.setCustomValidity("Slots count must be even.");
                slotsCount.reportValidity();
                return;
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

            const json = await getJSON(path);
            if (!json) return;

            this.fill(card, {
                break_time: json.break_time ?? 176,
                mab_time: json.mab_time ?? 12,
                refresh_rate: json.refresh_rate ?? 40,
                slots_count: json.slots_count ?? 512
            });
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