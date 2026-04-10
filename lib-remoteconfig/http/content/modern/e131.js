window.e131 = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const keys = Object.keys(json).filter(function(k) {
            return k.indexOf("priority_port_") === 0;
        });

        if (!keys.length) return;

        let h = "<h2>" + name + "</h2>";
        h += "<form onsubmit=\"e131.save('" + path + "'); return false;\">";
        h += "<div class='row'>";
        h += "<label>Priority</label>";
        h += "<div class='ports'>";

        for (let i = 0;i < keys.length;i++) {
            const key = keys[i];
            const suffix = key.substring(key.length - 1).toUpperCase();
            h += "<div class='port'>";
            h += "<span>" + suffix + "</span>";
            h += "<input id='" + key + "' type='number' min='100' max='200' required>";
            h += "</div>";
        }

        h += "</div>";
        h += "<button type='submit'>Save</button>";
        h += "</div>";
        h += "</form>";

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML = h;
        document.getElementById("modules").appendChild(div);

        for (let i = 0;i < keys.length;i++) {
            document.getElementById(keys[i]).value = json[keys[i]];
        }
    },

    save: function(path) {
        const inputs = document.querySelectorAll("input[id^='priority_port_']");
        const out = {};

        for (let i = 0;i < inputs.length;i++) {
            const e = inputs[i];

            if (!e.checkValidity()) {
                e.reportValidity();
                return;
            }

            out[e.id] = +e.value;
        }

        fetch("json/" + path, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(out)
        }).then(function(r) {
            if (!r.ok) {
                console.log("Save failed");
            }
        });
    }
};