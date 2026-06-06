window.artnet = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const suffixes = ["a", "b", "c", "d"].filter(function(suffix) {
            return json["protocol_port_" + suffix] !== undefined ||
                json["rdm_enable_port_" + suffix] !== undefined ||
                json["destination_ip_port_" + suffix] !== undefined;
        });

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
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

        const optionalRdmContainer = div.querySelector(".optional-rdm-container");
        const portsContainer = div.querySelector(".ports-container");

        if (json.enable_rdm !== undefined) {
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

            if (json["protocol_port_" + suffix] !== undefined) {
                const select = document.createElement("select");
                select.dataset.key = "protocol_port_" + suffix;
                select.innerHTML = "<option value='artnet'>artnet</option><option value='sacn'>sacn</option><option value='disabled'>disabled</option>";
                row.appendChild(select);
            }

            if (json["rdm_enable_port_" + suffix] !== undefined) {
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

            if (json["destination_ip_port_" + suffix] !== undefined) {
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

        document.getElementById("modules").appendChild(div);
        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div);
            return false;
        };

        fillDataKeys(div, json);
    }
};
