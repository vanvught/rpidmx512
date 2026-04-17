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

        card.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Map universe 0</label>
                    <input data-key="map_universe0" type="checkbox">
                </div>

                <div class="optional-rdm-container"></div>
                <div class="ports-container"></div>

                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        const optionalRdmContainer = card.querySelector(".optional-rdm-container");
        const portsContainer = card.querySelector(".ports-container");
        const form = card.querySelector("form");

        if (j.enable_rdm !== undefined) {
            const row = document.createElement("div");
            row.className = "row checkbox";

            const label = document.createElement("label");
            label.textContent = "Enable RDM";

            const input = document.createElement("input");
            input.type = "checkbox";
            input.dataset.key = "enable_rdm";

            row.appendChild(label);
            row.appendChild(input);
            optionalRdmContainer.appendChild(row);
        }

        for (let i = 0; i < suffixes.length; i++) {
            const suffix = suffixes[i];

            const row = document.createElement("div");
            row.className = "row";

            const portLabel = document.createElement("label");
            portLabel.textContent = "Port " + suffix.toUpperCase();
            row.appendChild(portLabel);

            if (j["protocol_port_" + suffix] !== undefined) {
                const select = document.createElement("select");
                select.dataset.key = "protocol_port_" + suffix;

                const optArtnet = document.createElement("option");
                optArtnet.value = "artnet";
                optArtnet.textContent = "artnet";
                select.appendChild(optArtnet);

                const optSacn = document.createElement("option");
                optSacn.value = "sacn";
                optSacn.textContent = "sacn";
                select.appendChild(optSacn);

                const optDisabled = document.createElement("option");
                optDisabled.value = "disabled";
                optDisabled.textContent = "disabled";
                select.appendChild(optDisabled);

                row.appendChild(select);
            }

            if (j["rdm_enable_port_" + suffix] !== undefined) {
                const rdmLabel = document.createElement("label");
                rdmLabel.textContent = "RDM";
                rdmLabel.style.width = "auto";
                rdmLabel.style.marginLeft = "8px";

                const rdmInput = document.createElement("input");
                rdmInput.type = "checkbox";
                rdmInput.dataset.key = "rdm_enable_port_" + suffix;

                row.appendChild(rdmLabel);
                row.appendChild(rdmInput);
            }

            if (j["destination_ip_port_" + suffix] !== undefined) {
                const ipLabel = document.createElement("label");
                ipLabel.textContent = "Destination IP";
                ipLabel.style.width = "auto";
                ipLabel.style.marginLeft = "8px";

                const ipInput = document.createElement("input");
                ipInput.dataset.key = "destination_ip_port_" + suffix;
                ipInput.pattern = "(\\d{1,3}\\.){3}\\d{1,3}";

                row.appendChild(ipLabel);
                row.appendChild(ipInput);
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