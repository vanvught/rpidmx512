window.dmxsend = {
    init: async function(path, name) {
        const json = await getJSON(path);
        if (!json) return;

        const div = document.createElement("div");
        div.className = "card";
        div.innerHTML =
            "<h2>" + name + "</h2>" +
            "<form onsubmit=\"dmxsend.save('" + path + "'); return false;\">" +
            "<div class='row'>" +
            "<label>Break time</label>" +
            "<input id='break_time' type='number' min='92' required>" +
            "<span>us</span>" +
            "</div>" +
            "<div class='row'>" +
            "<label>MAB time</label>" +
            "<input id='mab_time' type='number' min='12' max='1000000' required>" +
            "<span>us</span>" +
            "</div>" +
            "<div class='row'>" +
            "<label>Refresh rate</label>" +
            "<input id='refresh_rate' type='number' min='1' required>" +
            "<span>Hz</span>" +
            "</div>" +
            "<div class='row'>" +
            "<label>Slots count</label>" +
            "<input id='slots_count' type='number' min='2' max='512' step='2' required>" +
            "</div>" +
            "<div class='row'>" +
            "<label></label>" +
            "<button type='submit'>Save</button>" +
            "</div>" +
            "</form>";

        document.getElementById("modules").appendChild(div);

        document.getElementById("break_time").value = json.break_time ?? 176;
        document.getElementById("mab_time").value = json.mab_time ?? 12;
        document.getElementById("refresh_rate").value = json.refresh_rate ?? 40;
        document.getElementById("slots_count").value = json.slots_count ?? 512;
    },

    save: async function(path) {
        const breakTime = document.getElementById("break_time");
        const mabTime = document.getElementById("mab_time");
        const refreshRate = document.getElementById("refresh_rate");
        const slotsCount = document.getElementById("slots_count");

        slotsCount.setCustomValidity("");

        if (!breakTime.checkValidity()) return breakTime.reportValidity();
        if (!mabTime.checkValidity()) return mabTime.reportValidity();
        if (!refreshRate.checkValidity()) return refreshRate.reportValidity();
        if (!slotsCount.checkValidity()) return slotsCount.reportValidity();

        const slots = +slotsCount.value;
        if ((slots & 1) !== 0) {
            slotsCount.setCustomValidity("Slots count must be even.");
            return slotsCount.reportValidity();
        }

        try {
            const res = await fetch("json/" + path, {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify({
                    break_time: +breakTime.value,
                    mab_time: +mabTime.value,
                    refresh_rate: +refreshRate.value,
                    slots_count: slots
                })
            });

            if (!res.ok) {
                console.log("Save failed");
                return;
            }

            const json = await getJSON(path);
            if (!json) return;

            breakTime.value = json.break_time;
            mabTime.value = json.mab_time;
            refreshRate.value = json.refresh_rate;
            slotsCount.value = json.slots_count;

        } catch (e) {
            console.log("Error:", e);
        }
    }
};