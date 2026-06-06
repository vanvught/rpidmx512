window.ltcdisplays = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>OLED intensity</label>
                    <input data-key="oled_intensity" type="number" min="0" max="255" required>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="rotary_fullstep" type="checkbox">
                    <span>Rotary full step</span>
                </div>
                <div class="row">
                    <label>MAX7219 type</label>
                    <select data-key="max7219_type">
                        <option value="matrix">matrix</option>
                        <option value="7segment">7segment</option>
                    </select>
                </div>
                <div class="row">
                    <label>MAX7219 intensity</label>
                    <input data-key="max7219_intensity" type="number" min="0" max="255" required>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);
        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div);
            return false;
        };

        fillDataKeys(div, json);
    }
};