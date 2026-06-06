window.ltc = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;
		
		const now = new Date();

        const div = document.createElement("div");
        div.className = "card";

		div.innerHTML = `
		    <h2>${name}</h2>
		    <form>
		        <div class="row">
		            <label>Source</label>
		            <select data-key="source">
		                <option value="ltc">LTC</option>
		                <option value="artnet">Art-Net</option>
		                <option value="midi">Midi</option>
		                <option value="tcnet">TCNet</option>
		                <option value="internal">Internal</option>
		                <option value="rtp-midi">RTP-midi</option>
		                <option value="systime">Systime</option>
		                <option value="etc">ETC</option>
		            </select>
		        </div>

		        <div class="row checkbox">
		            <label>Disable</label>
		            <input data-key="disable_display" type="checkbox"><span>OLED</span>
		            <input data-key="disable_max7219" type="checkbox"><span>MAX7219</span>
		            <input data-key="disable_ltc" type="checkbox"><span>LTC</span>
		            <input data-key="disable_midi" type="checkbox"><span>Midi</span>
		            <input data-key="disable_artnet" type="checkbox"><span>Art-Net</span>
		            <input data-key="disable_rtp-midi" type="checkbox"><span>RTP-midi</span>
		            <input data-key="disable_etc" type="checkbox"><span>ETC</span>
		        </div>

		        <div class="row checkbox">
		            <label>Options</label>
		            <input data-key="show_systime" type="checkbox"><span>Show system time</span>
		            <input data-key="disable_timesync" type="checkbox"><span>Disable timesync</span>
		            <input data-key="auto_start" type="checkbox"><span>Auto start</span>
		        </div>

		        <div class="row checkbox">
		            <label>GPS</label>
		            <input data-key="gps_start" type="checkbox"><span>GPS start</span>
		            <span>UTC offset</span>
		            <input data-key="utc_offset" pattern="[+\\-]?[0-9]{2}:[0-9]{2}" maxlength="6" required>
		        </div>

		        <h3>Timecode</h3>

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
		            <input data-key="start_hour" type="number" min="0" max="23" required>
		            <span>h</span>
		            <input data-key="start_minute" type="number" min="0" max="59" required>
		            <span>m</span>
		            <input data-key="start_second" type="number" min="0" max="59" required>
		            <span>s</span>
		            <input data-key="start_frame" type="number" min="0" max="29" required>
		            <span>f</span>
		        </div>

		        <div class="row">
		            <label>Stop</label>
		            <input data-key="stop_hour" type="number" min="0" max="23" required>
		            <span>h</span>
		            <input data-key="stop_minute" type="number" min="0" max="59" required>
		            <span>m</span>
		            <input data-key="stop_second" type="number" min="0" max="59" required>
		            <span>s</span>
		            <input data-key="stop_frame" type="number" min="0" max="29" required>
		            <span>f</span>
		        </div>

		        <div class="row checkbox">
		            <label></label>
		            <input data-key="ignore_start" type="checkbox"><span>Ignore start</span>
		            <input data-key="ignore_stop" type="checkbox"><span>Ignore stop</span>
		            <input data-key="alt_function" type="checkbox"><span>Alt function</span>
		            <input data-key="skip_seconds" type="checkbox"><span>Skip seconds</span>
		            <input data-key="skip_free" type="checkbox"><span>Skip free</span>
		        </div>

		        <div class="row">
		            <label>Timecode IP</label>
		            <input data-key="timecode_ip" pattern="(\\d{1,3}\\.){3}\\d{1,3}" required>
		        </div>

		        <div class="row">
		            <label>Volume</label>
		            <input data-key="volume" type="number" min="1" max="31" required>
		        </div>

		        <div class="row checkbox">
		            <label>NTP</label>
		            <input data-key="ntp_enable" type="checkbox"><span>Enable</span>
		            <span>Year</span>
		            <input data-key="year" type="number" min="1970" max="2099" required>
		            <span>Month</span>
		            <input data-key="month" type="number" min="1" max="12" required>
		            <span>Day</span>
		            <input data-key="day" type="number" min="1" max="31" required>
		        </div>

		        <div class="row checkbox">
		            <label>OSC</label>
		            <input data-key="osc_enable" type="checkbox"><span>Enable</span>
		            <span>Port</span>
		            <input data-key="osc_port" type="number" min="1" max="65535" required>
		        </div>

		        <div class="row checkbox">
		            <label>Output</label>
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
		
		const form = div.querySelector("form");

		const key = function(name) {
		    return div.querySelector('[data-key="' + name + '"]');
		};

		const toInt = function(name) {
		    return parseInt(key(name).value, 10) || 0;
		};

		const fpsValue = function() {
		    return parseInt(key("fps").value, 10) || 30;
		};

		const updateFrameLimits = function() {
		    const fps = fpsValue();
		    key("start_frame").max = fps - 1;
		    key("stop_frame").max = fps - 1;
		};

		const totalFrames = function(prefix) {
		    const fps = fpsValue();
		    return (((toInt(prefix + "_hour") * 60 + toInt(prefix + "_minute")) * 60 + toInt(prefix + "_second")) * fps) + toInt(prefix + "_frame");
		};

		const isValidIp = function(value) {
		    const parts = value.trim().split(".");
		    if (parts.length !== 4) return false;

		    for (let i = 0; i < parts.length; i++) {
		        if (!/^[0-9]{1,3}$/.test(parts[i])) return false;
		        const n = parseInt(parts[i], 10);
		        if (n < 0 || n > 255) return false;
		    }

		    return true;
		};

		const daysInMonth = function(year, month) {
		    return new Date(year, month, 0).getDate();
		};

		const validateForm = function() {
		    updateFrameLimits();

		    const timecodeIp = key("timecode_ip");
		    timecodeIp.setCustomValidity(isValidIp(timecodeIp.value) ? "" : "Enter a valid IPv4 address.");

		    const startFrame = key("start_frame");
		    const stopFrame = key("stop_frame");
		    const fps = fpsValue();

		    startFrame.setCustomValidity(toInt("start_frame") < fps ? "" : "Start frame must be lower than FPS.");
		    stopFrame.setCustomValidity(toInt("stop_frame") < fps ? "" : "Stop frame must be lower than FPS.");

		    const stopSecond = key("stop_second");
		    stopSecond.setCustomValidity(totalFrames("stop") > totalFrames("start") ? "" : "Stop time must be greater than start time.");

		    const year = toInt("year");
		    const month = toInt("month");
		    const day = key("day");
		    const maxDay = daysInMonth(year, month);
		    day.max = maxDay;
		    day.setCustomValidity(toInt("day") <= maxDay ? "" : "Invalid day for selected month.");
		};

		form.addEventListener("input", validateForm);
		form.addEventListener("change", validateForm);
        
		div.querySelector("form").onsubmit = () => {
			validateForm();

			if (!form.checkValidity()) {
			    form.reportValidity();
			    return false;
			}
			
            saveDataKeyForm(path, div);
            return false;
        };

        fillDataKeys(div, json);
    }
};
