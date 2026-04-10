window.rdmdevice = {
    init: function(path, name) {
        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML =
            "<h2>" + name + "</h2>" +
            "<form onsubmit=\"saveField('label','json/" + path + "','label'); return false;\">" +
            "<div class='row'>" +
            "<label>Label</label>" +
            "<input id='label' maxlength='32' pattern='.{1,32}' required>" +
            "<button type='submit'>Save</button>" +
            "</div>" +
            "</form>";

        document.getElementById("modules").appendChild(div);

        loadField("label", path, "label");
    }
};