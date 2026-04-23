window.dmxpca9685 = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Mode</label>
                    <select data-key="mode">
                        <option value="led">led</option>
                        <option value="servo">servo</option>
                    </select>
                </div>
                <div class="row">
                    <label>Channel count</label>
                    <input data-key="channel_count" type="number" min="1" max="16" required>
                </div>
                <div class="row">
                    <label>DMX start address</label>
                    <input data-key="dmx_start_address" type="number" min="1" max="512" required>
                </div>
                <div class="row">
                    <label>LED PWM frequency</label>
                    <input data-key="led_pwm_frequency" type="number" min="24" max="1526" required>
                    <span>Hz</span>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="use_8bit" type="checkbox">
                    <span>Use 8-bit</span>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="led_output_invert" type="checkbox">
                    <span>LED output invert</span>
                </div>
                <div class="row checkbox">
                    <label></label>
                    <input data-key="led_output_opendrain" type="checkbox">
                    <span>LED output open-drain</span>
                </div>
                <div class="row">
                    <label>Servo left</label>
                    <input data-key="servo_left_us" type="number" min="0" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>Servo center</label>
                    <input data-key="servo_center_us" type="number" min="0" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>Servo right</label>
                    <input data-key="servo_right_us" type="number" min="0" required>
                    <span>us</span>
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
                        mode: data.mode ?? "led",
                        channel_count: data.channel_count ?? 16,
                        dmx_start_address: data.dmx_start_address ?? 1,
                        led_pwm_frequency: data.led_pwm_frequency ?? 120,
                        use_8bit: data.use_8bit ?? 0,
                        led_output_invert: data.led_output_invert ?? 0,
                        led_output_opendrain: data.led_output_opendrain ?? 0,
                        servo_left_us: data.servo_left_us ?? 1000,
                        servo_center_us: data.servo_center_us ?? 0,
                        servo_right_us: data.servo_right_us ?? 2000
                    });
                }
            });
            return false;
        };

        fillDataKeys(div, {
            mode: json.mode ?? "led",
            channel_count: json.channel_count ?? 16,
            dmx_start_address: json.dmx_start_address ?? 1,
            led_pwm_frequency: json.led_pwm_frequency ?? 120,
            use_8bit: json.use_8bit ?? 0,
            led_output_invert: json.led_output_invert ?? 0,
            led_output_opendrain: json.led_output_opendrain ?? 0,
            servo_left_us: json.servo_left_us ?? 1000,
            servo_center_us: json.servo_center_us ?? 0,
            servo_right_us: json.servo_right_us ?? 2000
        });
    }
};