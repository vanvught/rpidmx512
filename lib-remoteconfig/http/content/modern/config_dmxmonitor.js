window.dmxmonitor = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML = `
            <h2>${name}</h2>
            <form accept-charset="utf-8">
                <div class='row'>
                    <label>Start address</label>
                    <input data-key='dmx_start_address' type='number' min='1' max='512' required>
                </div>
                <div class='row'>
                    <label>Max channels</label>
                    <input data-key='dmx_max_channels' type='number' min='1' max='512' required>
                </div>
                <div class='row'>
                    <label>Format</label>
                    <select data-key='format'>
                        <option value='hex'>hex</option>
                        <option value='pct'>pct</option>
                        <option value='dec'>dec</option>
                    </select>
                </div>
                <div class='row'>
                    <label></label>
                    <button type='submit'>Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);
        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div);
            return false;
        };

        fillDataKeys(div, {
            dmx_start_address: json.dmx_start_address || 1,
            dmx_max_channels: json.dmx_max_channels || 16,
            format: json.format || "dec"
        });
    }
};
