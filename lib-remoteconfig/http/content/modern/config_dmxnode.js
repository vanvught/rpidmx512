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
                select.innerHTML = "<option value='output'>output</option><option value='input'>input</option><option value='disable'>disable</option>";
                row.appendChild(select);
            }

            if (j["merge_mode_port_" + suffix] !== undefined) {
                const select = document.createElement("select");
                select.dataset.key = "merge_mode_port_" + suffix;
                select.innerHTML = "<option value='htp'>htp</option><option value='ltp'>ltp</option>";
                row.appendChild(select);
            }

            portsContainer.appendChild(row);
        }

        document.getElementById("modules").appendChild(card);
        card.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, card);
            return false;
        };

        fillDataKeys(card, j);
    }
};
