window.tcnet = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Node name</label>
                    <input data-key="node_name" maxlength="8" pattern="[ -~]{1,8}" required>
                </div>
                <div class="row">
                    <label>Layer</label>
                    <select data-key="layer">
						<option value="1">1</option>
						<option value="2">2</option>
						<option value="3">3</option>
						<option value="4">4</option>
                        <option value="A">A</option>
                        <option value="B">B</option>
                        <option value="M">M</option>
                        <option value="C">C</option>
                    </select>
                </div>
                <div class="row">
                    <label>Timecode type</label>
                    <select data-key="timecode_type">
                        <option value="24">24</option>
                        <option value="25">25</option>
                        <option value="29">29</option>
                        <option value="30">30</option>
                    </select>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="use_timecode" type="checkbox">
                    <span>Use timecode</span>
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
                        node_name: data.node_name ?? "AvV",
                        layer: (data.layer ?? "M").toUpperCase(),
                        timecode_type: data.timecode_type ?? 30,
                        use_timecode: data.use_timecode ?? 0
                    });
                }
            });
            return false;
        };

        fillDataKeys(div, {
            node_name: json.node_name ?? "AvV",
            layer: (json.layer ?? "M").toUpperCase(),
            timecode_type: json.timecode_type ?? 30,
            use_timecode: json.use_timecode ?? 0
        });
    }
};