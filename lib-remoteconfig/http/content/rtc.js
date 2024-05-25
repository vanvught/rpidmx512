async function refresh() {
  let d1=await getJSON('timedate')
  const node = formatDateTime(new Date(d1.date))
  document.getElementById('nodeTime').textContent = `Node Time: ${node}`
  let d2=await getJSON('rtcalarm')
  const rtc = formatDateTime(new Date(d2.rtc))
  document.getElementById('rtcTime').textContent = `RTC Time: ${rtc}`
  document.getElementById('alarmInput').value = formatDateTime(new Date(d2.alarm))
  document.getElementById('chkbox').checked = (d2.enabled === "1")
}

async function hcToSys() {
  await post({ rtc: "", action: "hctosys" });
  refresh();
}

async function sysToHc() {
  await post({ rtc: "", action: "systohc" });
  refresh();
}

async function alarm() {
  const a = document.getElementById('alarmInput').value;
  const b = document.getElementById('chkbox').checked ? "1" : "0";
  await post({ rtc:"", alarm: a, enable: b });
  refresh();
}
