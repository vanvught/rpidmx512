window.showfile = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Show</label>
                    <input data-key="show" type="number" min="0" max="99" required>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="auto_play" type="checkbox">
                    <span>Auto play</span>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="loop" type="checkbox">
                    <span>Loop</span>
                </div>
                <div class="row">
                    <label>Incoming port</label>
                    <input data-key="incoming_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label>Outgoing port</label>
                    <input data-key="outgoing_port" type="number" min="1024" max="65535" required>
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
