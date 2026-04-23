window.oscclient = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Server IP</label>
                    <input data-key="server_ip" pattern="(\\d{1,3}\\.){3}\\d{1,3}" required>
                </div>
                <div class="row">
                    <label>Incoming port</label>
                    <input data-key="incoming_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row">
                    <label>Outgoing port</label>
                    <input data-key="outgoing_port" type="number" min="1024" max="65535" required>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="ping_disable" type="checkbox">
                    <span>Disable ping</span>
                </div>
                <div class="row">
                    <label>Ping delay</label>
                    <input data-key="ping_delay" type="number" min="0" required>
                    <span>seconds</span>
                </div>
                <div class="row">
                    <label>Command 0</label>
                    <input data-key="cmd0" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 1</label>
                    <input data-key="cmd1" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 2</label>
                    <input data-key="cmd2" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 3</label>
                    <input data-key="cmd3" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 4</label>
                    <input data-key="cmd4" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 5</label>
                    <input data-key="cmd5" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 6</label>
                    <input data-key="cmd6" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>Command 7</label>
                    <input data-key="cmd7" maxlength="64" pattern="|/.{0,63}">
                </div>
                <div class="row">
                    <label>LED 0</label>
                    <input data-key="led0" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 1</label>
                    <input data-key="led1" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 2</label>
                    <input data-key="led2" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 3</label>
                    <input data-key="led3" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 4</label>
                    <input data-key="led4" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 5</label>
                    <input data-key="led5" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 6</label>
                    <input data-key="led6" maxlength="48" pattern="|/.{0,47}">
                </div>
                <div class="row">
                    <label>LED 7</label>
                    <input data-key="led7" maxlength="48" pattern="|/.{0,47}">
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

        fillDataKeys(div, {
            server_ip: json.server_ip ?? "192.168.2.153",
            incoming_port: json.incoming_port ?? 9000,
            outgoing_port: json.outgoing_port ?? 8000,
            ping_disable: json.ping_disable ?? 0,
            ping_delay: json.ping_delay ?? 10,
            cmd0: json.cmd0 ?? "",
            cmd1: json.cmd1 ?? "",
            cmd2: json.cmd2 ?? "",
            cmd3: json.cmd3 ?? "",
            cmd4: json.cmd4 ?? "",
            cmd5: json.cmd5 ?? "",
            cmd6: json.cmd6 ?? "",
            cmd7: json.cmd7 ?? "",
            led0: json.led0 ?? "/led0",
            led1: json.led1 ?? "",
            led2: json.led2 ?? "",
            led3: json.led3 ?? "",
            led4: json.led4 ?? "",
            led5: json.led5 ?? "",
            led6: json.led6 ?? "/status/led",
            led7: json.led7 ?? ""
        });
    }
};