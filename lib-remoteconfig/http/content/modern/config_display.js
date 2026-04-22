window.display = {
    init: async function(path, name) {
        const j = await getJSON(path);
        if (!j) return;

        const optionalFields = [
            "title",
            "board_name",
            "version",
            "hostname",
            "ip_address",
            "net_mask",
            "default_gateway",
            "active_ports"
        ];

        const labels = {
            title: "Title",
            board_name: "Board name",
            version: "Version",
            hostname: "Hostname",
            ip_address: "IP address",
            net_mask: "Net mask",
            default_gateway: "Default gateway",
            active_ports: "Active ports"
        };

        const rowAssignments = {};
        for (let i = 0; i < optionalFields.length; i++) {
            const key = optionalFields[i];
            const v = j[key];
            if (typeof v === "number" && v >= 1 && v <= 6) {
                rowAssignments[v] = key;
            }
        }

        const card = document.createElement("div");
        card.className = "card";

        let h = "<h2>" + name + "</h2><form>";
        h += "<div class='row'><label>Intensity</label><input data-key='intensity' type='number' min='0' max='255' required></div>";
        h += "<div class='row'><label>Sleep timeout</label><input data-key='sleep_timeout' type='number' min='0' max='60' required></div>";
        h += "<div class='row checkbox'><label>Flip vertically</label><input data-key='flip_vertically' type='checkbox'></div>";

        for (let row = 1; row <= 6; row++) {
            h += "<div class='row'><label>Row " + row + "</label><select data-row='" + row + "'><option value=''>-</option>";
            for (let i = 0; i < optionalFields.length; i++) {
                const key = optionalFields[i];
                h += "<option value='" + key + "'>" + labels[key] + "</option>";
            }
            h += "</select></div>";
        }

        h += "<div class='row'><label></label><button type='submit'>Save</button></div></form>";
        card.innerHTML = h;
        document.getElementById("modules").appendChild(card);

        for (let row = 1; row <= 6; row++) {
            const e = card.querySelector("select[data-row='" + row + "']");
            if (e && rowAssignments[row] !== undefined) {
                e.value = rowAssignments[row];
            }
        }

        fillDataKeys(card, {
            intensity: j.intensity ?? 128,
            sleep_timeout: j.sleep_timeout ?? 0,
            flip_vertically: j.flip_vertically ?? 0
        });

        card.querySelector("form").onsubmit = function() {
            window.display.save(path, card, optionalFields);
            return false;
        };
    },

    save: async function(path, card, optionalFields) {
        const fields = [
            card.querySelector("[data-key='intensity']"),
            card.querySelector("[data-key='sleep_timeout']")
        ];

        if (!validateFields(fields)) {
            return;
        }

        const rowSelects = card.querySelectorAll("select[data-row]");
        const used = {};
        for (let i = 0; i < rowSelects.length; i++) {
            const e = rowSelects[i];
            if (!e.value) continue;
            if (used[e.value]) {
                alert("Each display field can only be assigned once.");
                e.focus();
                return;
            }
            used[e.value] = true;
        }

        const out = collectDataKeys(card);
        if (!out) return;

        for (let i = 0; i < optionalFields.length; i++) {
            out[optionalFields[i]] = 0;
        }

        for (let i = 0; i < rowSelects.length; i++) {
            const e = rowSelects[i];
            if (e.value) {
                out[e.value] = +e.dataset.row;
            }
        }

        const btn = card.querySelector("button[type='submit']");
        if (btn) {
            btn.disabled = true;
            btn.textContent = "Saving...";
        }

        try {
            const ok = await postJSON("json/" + path, out);
            if (!ok) {
                console.log("Save failed");
                return;
            }

            const json = await getJSON(path);
            if (!json) return;

            fillDataKeys(card, {
                intensity: json.intensity ?? 128,
                sleep_timeout: json.sleep_timeout ?? 0,
                flip_vertically: json.flip_vertically ?? 0
            });

            for (let row = 1; row <= 6; row++) {
                const select = card.querySelector("select[data-row='" + row + "']");
                select.value = "";
            }

            for (let i = 0; i < optionalFields.length; i++) {
                const key = optionalFields[i];
                const row = json[key];
                if (typeof row === "number" && row >= 1 && row <= 6) {
                    const select = card.querySelector("select[data-row='" + row + "']");
                    if (select) {
                        select.value = key;
                    }
                }
            }
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
