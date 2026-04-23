window.gps = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Module</label>
                    <select data-key="module">
                        <option value="ATGM336H">ATGM336H</option>
                        <option value="ublox-NEO7">ublox-NEO7</option>
                        <option value="MTK3339">MTK3339</option>
                    </select>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="enable" type="checkbox">
                    <span>Enable</span>
                </div>
                <div class="row">
                    <label>UTC offset</label>
                    <input data-key="utc_offset" pattern="[+-]?\\d{2}:\\d{2}" required>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

		document.getElementById("modules").appendChild(card);
		card.querySelector("form").onsubmit = () => {
		    saveDataKeyForm(path, card);
		    return false;
		};

        fillDataKeys(div, {
            module: json.module ?? "ATGM336H",
            enable: json.enable ?? 0,
            utc_offset: json.utc_offset ?? "00:00"
        });
    }
};