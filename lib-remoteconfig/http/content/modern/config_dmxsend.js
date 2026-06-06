window.dmxsend = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";

        div.innerHTML = `
            <h2>${name}</h2>
            <form>
                <div class="row">
                    <label>Break time</label>
                    <input data-key="break_time" type="number" min="92" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>MAB time</label>
                    <input data-key="mab_time" type="number" min="12" max="1000000" required>
                    <span>us</span>
                </div>
                <div class="row">
                    <label>Refresh rate</label>
                    <input data-key="refresh_rate" type="number" min="1" required>
                    <span>Hz</span>
                </div>
                <div class="row">
                    <label>Slots count</label>
                    <input data-key="slots_count" type="number" min="2" max="512" step="2" required>
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
		        beforePost: function(out, card) {
		            const slotsField = card.querySelector("[data-key='slots_count']");
		            slotsField.setCustomValidity("");

		            if ((out.slots_count & 1) !== 0) {
		                slotsField.setCustomValidity("Slots count must be even.");
		                slotsField.reportValidity();
		                return false;
		            }

		            return true;
		        }
		    });

		    return false;
		};

        fillDataKeys(div, json);
    }
};
