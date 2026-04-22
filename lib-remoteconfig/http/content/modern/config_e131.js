window.e131 = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const keys = Object.keys(json).filter(function(k) {
            return k.indexOf("priority_port_") === 0;
        });

        if (!keys.length) return;

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Priority</label>
                    <div class="ports"></div>
                </div>
                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);

        const portsContainer = div.querySelector(".ports");
        for (let i = 0; i < keys.length; i++) {
            const key = keys[i];
            const suffix = key.substring(key.length - 1).toUpperCase();

            const portDiv = document.createElement("div");
            portDiv.className = "port";
            portDiv.innerHTML = `
                <span>${suffix}</span>
                <input type='number' min='100' max='200' required data-key='${key}'>
            `;
            portsContainer.appendChild(portDiv);
        }

        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div);
            return false;
        };

        fillDataKeys(div, json);
    }
};
