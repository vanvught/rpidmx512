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

        card.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Node name</label>
                    <input data-key="node_name" maxlength="64" pattern="[ -~]{1,64}" required>
                </div>

                <div class="row">
                    <label>Failsafe</label>
                    <select data-key="failsafe">
                        <option value="hold">Hold</option>
                        <option value="on">On</option>
                        <option value="off">Off</option>
                        <option value="playback">Playback</option>
                    </select>
                </div>

                <div class="row checkbox">
                    <label>Disable merge timeout</label>
                    <input data-key="disable_merge_timeout" type="checkbox">
                </div>

                <div class="ports-container"></div>

                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        const form = card.querySelector("form");
        const portsContainer = card.querySelector(".ports-container");

        for (let i = 0; i < suffixes.length; i++) {
            const suffix = suffixes[i];

            const row = document.createElement("div");
            row.className = "row";

            const label = document.createElement("label");
            label.textContent = "Port " + suffix.toUpperCase();
            row.appendChild(label);

            if (j["label_port_" + suffix] !== undefined) {
                const input = document.createElement("input");
                input.dataset.key = "label_port_" + suffix;
                input.maxLength = 18;
                input.pattern = "[ -~]{1,18}";
                input.required = true;
                row.appendChild(input);
            }

            if (j["universe_port_" + suffix] !== undefined) {
                const input = document.createElement("input");
                input.dataset.key = "universe_port_" + suffix;
                input.type = "number";
                input.min = "0";
                input.max = "32768";
                row.appendChild(input);
            }

            if (j["direction_port_" + suffix] !== undefined) {
                const select = document.createElement("select");
                select.dataset.key = "direction_port_" + suffix;

                const optOutput = document.createElement("option");
                optOutput.value = "output";
                optOutput.textContent = "output";
                select.appendChild(optOutput);

                const optInput = document.createElement("option");
                optInput.value = "input";
                optInput.textContent = "input";
                select.appendChild(optInput);

                const optDisable = document.createElement("option");
                optDisable.value = "disable";
                optDisable.textContent = "disable";
                select.appendChild(optDisable);

                row.appendChild(select);
            }

            if (j["merge_mode_port_" + suffix] !== undefined) {
                const select = document.createElement("select");
                select.dataset.key = "merge_mode_port_" + suffix;

                const optHtp = document.createElement("option");
                optHtp.value = "htp";
                optHtp.textContent = "htp";
                select.appendChild(optHtp);

                const optLtp = document.createElement("option");
                optLtp.value = "ltp";
                optLtp.textContent = "ltp";
                select.appendChild(optLtp);

                row.appendChild(select);
            }

            portsContainer.appendChild(row);
        }

        document.getElementById("modules").appendChild(card);

        form.onsubmit = () => {
            this.save(path, card);
            return false;
        };

        this.fill(card, j);
    },

    fill: function(card, j) {
        const fields = card.querySelectorAll("[data-key]");

        for (let i = 0; i < fields.length; i++) {
            const e = fields[i];
            const key = e.dataset.key;

            if (j[key] === undefined) {
                continue;
            }

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

            this.fill(card, j);
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