window.oscserver = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Path</label>
                    <input data-key="path" maxlength="128" pattern="/.{0,127}" required>
                </div>
                <div class="row">
                    <label>Path info</label>
                    <input data-key="path_info" maxlength="128" pattern="/.{0,127}" required>
                </div>
                <div class="row">
                    <label>Path blackout</label>
                    <input data-key="path_blackout" maxlength="128" pattern="/.{0,127}" required>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="partial_transmission" type="checkbox">
                    <span>Partial transmission</span>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);
        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div, {
                afterLoad: function(card, data) {
                    fillDataKeys(card, {
                        path: data.path ?? "/dmx1",
                        path_info: data.path_info ?? "/2",
                        path_blackout: data.path_blackout ?? "/dmx1/blackout",
                        partial_transmission: data.partial_transmission ?? 0
                    });
                }
            });
            return false;
        };

        fillDataKeys(div, {
            path: json.path ?? "/dmx1",
            path_info: json.path_info ?? "/2",
            path_blackout: json.path_blackout ?? "/dmx1/blackout",
            partial_transmission: json.partial_transmission ?? 0
        });
    }
};