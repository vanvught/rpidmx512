window.ltc = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        const ipPattern = "(\\d{1,3}\\.){3}\\d{1,3}";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Source</label>
                    <select data-key="source">
                        <option value="ltc">ltc</option>
                        <option value="artnet">artnet</option>
                        <option value="midi">midi</option>
                        <option value="tcnet">tcnet</option>
                        <option value="internal">internal</option>
                        <option value="rtp-midi">rtp-midi</option>
                        <option value="systime">systime</option>
                        <option value="etc">etc</option>
                    </select>
                </div>

                <div class="row checkbox">
                    <label>Disable</label>
                    <input data-key="disable_display" type="checkbox"><span>Display</span>
                    <input data-key="disable_max7219" type="checkbox"><span>MAX7219</span>
                    <input data-key="disable_ltc" type="checkbox"><span>LTC</span>
                    <input data-key="disable_midi" type="checkbox"><span>MIDI</span>
                    <input data-key="disable_artnet" type="checkbox"><span>Art-Net</span>
                    <input data-key="disable_rtp-midi" type="checkbox"><span>RTP-MIDI</span>
                    <input data-key="disable_etc" type="checkbox"><span>ETC</span>
                </div>

                <div class="row checkbox">
                    <label>Options</label>
                    <input data-key="show_systime" type="checkbox"><span>Show system time</span>
                    <input data-key="disable_timesync" type="checkbox"><span>Disable time sync</span>
                    <input data-key="auto_start" type="checkbox"><span>Auto start</span>
                </div>

                <div class="row checkbox">
                    <label></label>
                    <input data-key="gps_start" type="checkbox"><span>GPS start</span>
                </div>

                <div class="row">
                    <label>UTC offset</label>
                    <input data-key="utc_offset" pattern="[+\\-]?([01][0-9]|2[0-3]):[0-5][0-9]" placeholder="00:00" required>
                </div>

                <div class="row">
                    <label>FPS</label>
                    <select data-key="fps">
                        <option value="24">24</option>
                        <option value="25">25</option>
                        <option value="29">29</option>
                        <option value="30">30</option>
                    </select>
                </div>

                <div class="row">
                    <label>Start</label>
                    <input data-key="start_hour" type="number" min="0" max="23" required><span>h</span>
                    <input data-key="start_minute" type="number" min="0" max="59" required><span>m</span>
                    <input data-key="start_second" type="number" min="0" max="59" required><span>s</span>
                    <input data-key="start_frame" type="number" min="0" required><span>f</span>
                </div>

                <div class="row">
                    <label>Stop</label>
                    <input data-key="stop_hour" type="number" min="0" max="23" required><span>h</span>
                    <input data-key="stop_minute" type="number" min="0" max="59" required><span>m</span>
                    <input data-key="stop_second" type="number" min="0" max="59" required><span>s</span>
                    <input data-key="stop_frame" type="number" min="0" required><span>f</span>
                </div>

                <div class="row checkbox">
                    <label>Flags</label>
                    <input data-key="ignore_start" type="checkbox"><span>Ignore start</span>
                    <input data-key="ignore_stop" type="checkbox"><span>Ignore stop</span>
                    <input data-key="alt_function" type="checkbox"><span>Alt function</span>
                    <input data-key="skip_seconds" type="checkbox"><span>Skip seconds</span>
                    <input data-key="skip_free" type="checkbox"><span>Skip free</span>
                </div>

                <div class="row">
                    <label>Timecode IP</label>
                    <input data-key="timecode_ip" pattern="${ipPattern}" required>
                </div>

                <div class="row">
                    <label>Volume</label>
                    <input data-key="volume" type="number" min="1" max="31" required>
                </div>

                <div class="row checkbox">
                    <label>NTP</label>
                    <input data-key="ntp_enable" type="checkbox"><span>Enable</span>
                    <span>Year</span><input data-key="year" type="number" min="0" max="9999" required>
                    <span>Month</span><input data-key="month" type="number" min="1" max="12" required>
                    <span>Day</span><input data-key="day" type="number" min="1" max="31" required>
                </div>

                <div class="row checkbox">
                    <label>OSC</label>
                    <input data-key="osc_enable" type="checkbox"><span>Enable</span>
                    <span>Port</span><input data-key="osc_port" type="number" min="1024" max="65535" required>
                </div>

                <div class="row checkbox">
                    <label>Displays</label>
                    <input data-key="ws28xx_enable" type="checkbox"><span>WS28xx</span>
                    <input data-key="rgbpanel_enable" type="checkbox"><span>RGB panel</span>
                </div>

                <div class="row">
                    <label></label>
                    <button type="submit">Save</button>
                </div>
            </form>
        `;

        document.getElementById("modules").appendChild(div);
        fillDataKeys(div, json);
        setCurrentDateDefaults(div);
        setFrameMax(div);

        div.querySelector("[data-key='fps']").onchange = () => setFrameMax(div);

        div.querySelector("form").onsubmit = () => {
            saveDataKeyForm(path, div, {
                beforePost: function(values, card) {
                    return validateLtcForm(values, card);
                }
            });
            return false;
        };
    }
};

function setCurrentDateDefaults(root) {
    const now = new Date();
    const defaults = {
        year: now.getFullYear(),
        month: now.getMonth() + 1,
        day: now.getDate()
    };

    for (const key in defaults) {
        const el = root.querySelector("[data-key='" + key + "']");
        if (el && (el.value === "" || Number(el.value) === 0)) {
            el.value = defaults[key];
        }
    }
}

function setFrameMax(root) {
    const fps = getNumber(root, "fps");
    const maxFrame = Math.max(0, fps - 1);

    ["start_frame", "stop_frame"].forEach(function(key) {
        const el = root.querySelector("[data-key='" + key + "']");
        if (!el) return;
        el.max = String(maxFrame);
        if (Number(el.value) > maxFrame) {
            el.value = maxFrame;
        }
    });
}

function validateLtcForm(values, root) {
    clearCustomValidity(root);
    setFrameMax(root);

    const ipField = root.querySelector("[data-key='timecode_ip']");
    if (!isIPv4(values.timecode_ip)) {
        ipField.setCustomValidity("Enter a valid IPv4 address.");
        ipField.reportValidity();
        return false;
    }

    return true;
}

function clearCustomValidity(root) {
    const elements = root.querySelectorAll("[data-key]");
    for (let i = 0; i < elements.length; i++) {
        elements[i].setCustomValidity("");
    }
}

function timecodeTotal(hour, minute, second, frame, fps) {
    return ((((Number(hour) * 60) + Number(minute)) * 60) + Number(second)) * fps + Number(frame);
}

function getNumber(root, key) {
    const el = root.querySelector("[data-key='" + key + "']");
    return el ? Number(el.value) : 0;
}

function isIPv4(value) {
    const parts = String(value).split(".");
    if (parts.length !== 4) return false;

    for (let i = 0; i < parts.length; i++) {
        if (!/^\d{1,3}$/.test(parts[i])) return false;
        const n = Number(parts[i]);
        if (n < 0 || n > 255) return false;
    }

    return true;
}
