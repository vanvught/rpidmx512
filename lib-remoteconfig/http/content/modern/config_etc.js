window.etc = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Destination IP</label>
                    <input data-key="destination_ip" pattern="(\\d{1,3}\\.){3}\\d{1,3}" required>
                </div>
                <div class="row">
                    <label>Destination port</label>
                    <input data-key="destination_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label>Source multicast IP</label>
                    <input data-key="source_multicast_ip" pattern="(\\d{1,3}\\.){3}\\d{1,3}" required>
                </div>
                <div class="row">
                    <label>Source port</label>
                    <input data-key="source_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label>UDP terminator</label>
                    <select data-key="udp_terminator">
                        <option value="None">None</option>
                        <option value="CR">CR</option>
                        <option value="LF">LF</option>
                        <option value="CRLF">CRLF</option>
                    </select>
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
                        destination_ip: data.destination_ip ?? "0.0.0.0",
                        destination_port: data.destination_port ?? 0,
                        source_multicast_ip: data.source_multicast_ip ?? "0.0.0.0",
                        source_port: data.source_port ?? 0,
                        udp_terminator: data.udp_terminator ?? "None"
                    });
                }
            });
            return false;
        };

        fillDataKeys(div, {
            destination_ip: json.destination_ip ?? "0.0.0.0",
            destination_port: json.destination_port ?? 0,
            source_multicast_ip: json.source_multicast_ip ?? "0.0.0.0",
            source_port: json.source_port ?? 0,
            udp_terminator: json.udp_terminator ?? "None"
        });
    }
};