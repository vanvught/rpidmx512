async function refresh() {
  try {
    let data=await getJSON('showfile/status')
    let show=(parseInt(data.show) >= 0 && parseInt(data.show) <= 99) ? data.show : 'None'
    let loop=data.loop === "1" ? "Yes" : "No"
    h=`<tr><td>Show</td><td>${show}</td> </tr>`
    h+=`<tr><td>Status</td><td>${data.status}</td></tr>`
    h+=`<tr><td>Looping</td><td>${loop}</td></tr>`
    document.getElementById("idStatus").innerHTML='<table>'+h+'</table>'
    let b=data.loop === "1" ? "No loop" : "Looping"
    document.getElementById("idLoop").innerHTML=b
  } catch (error){}
}

function send(status) {
  post({ show: "", status: status }).then(refresh)
}

function start() {
  send("start")
}

function stop() {
  send("stop")
}

function resume() {
  send("resume")
}

function select() {
  const v=document.getElementById("idDirectory").value;
  post({ show:`${v}` }).then(() => { refresh() })
}

function loop() {
  const v=document.getElementById('idLoop').innerHTML =="Looping" ? "1" : "0"
  post({ show:"",loop:`${v}` }).then(() => { refresh() })
}

async function directory() {
  try {
  let d=await getJSON('showfile/directory')
  let h=""
  let f=Object.keys(d["shows"])
  f.forEach(function(key) {
    var v = d["shows"][key]
    h += "<option value="+v+">"+v+"</option>"
  });
  document.getElementById("idDirectory").innerHTML = h
  } catch (error){}
}