async function refresh() {
  try {
    directory()
    let d=await getJSON('showfile/status')
    let s=(parseInt(d.show) >= 0 && parseInt(d.show) <= 99) ? d.show : 'None'
    let loop=d.loop === "1" ? "Yes" : "No"
    h=`<tr><td>Mode</td><td>${d.mode}</td> </tr>`
    h+=`<tr><td>Show</td><td>${s}</td> </tr>`
    h+=`<tr><td>Status</td><td>${d.status}</td></tr>`
    h+=`<tr><td>Looping</td><td>${loop}</td></tr>`
    document.getElementById("idStatus").innerHTML='<table>'+h+'</table>'
    let b=d.loop === "1" ? "No loop" : "Looping"
    document.getElementById("id4").innerHTML=b
  } catch (error){}
}

async function sel() {
  const v=document.getElementById("id1").value
  await post({ show:`${v}` })
  refresh()
}

async function rec() {
  const v=document.getElementById("id3").innerHTML
  await post({ show:"",recorder:`${v}` })
  refresh()
}

async function del() {
    const v=document.getElementById("id2").value
    await delet({ show:`${v}` })
    refresh()
}

function ff(d) {
    const s = d.shows.map(show => show.show)
    let e = 0;
    for (let n of s) {
        if (n !== e) { return e;}
        e++;
    }
    return e
}

async function directory() {
  try {
  let d=await getJSON('showfile/directory')
  let h=""
  let f=Object.keys(d["shows"])
  f.forEach(function(key) {
    var v = d["shows"][key]
    h += "<option value="+v.show+">"+v.show+" | "+v.size+"</option>"
  });
  document.getElementById("id1").innerHTML = h
  document.getElementById("id2").innerHTML = h
  document.getElementById("id3").innerHTML = ff(d)
  } catch (error){}
}