async function refresh() {
  try {
    let d=await getJSON('dmx/portstatus')
    let hdrs='<tr>'
    let tdd='<tr>'
    
    d.forEach(item => {
       hdrs+=`<th>${item.port}</th>`
       tdd+=`<td>${item.direction}</td>`
    });
    
    document.getElementById("idStats").innerHTML=hdrs+'</tr>'+tdd+'</tr>'
    
    let tr = await Promise.all(
      d.map(item => getJSON('dmx/status?' + item.port).then(resp => ({
          port: item.port,
          d: { sent: resp.dmx.sent, received: resp.dmx.received },
          r: {
            sent: { class: resp.rdm.sent.class, discovery: resp.rdm.sent.discovery },
            received: { good: resp.rdm.received.good, bad: resp.rdm.received.bad, discovery: resp.rdm.received.discovery
            }}}))));
        
    tr.sort((a, b) => {
      return d.findIndex(item => item.port === a.port) - d.findIndex(item => item.port === b.port)
    });
    
    hdrs='<tr><th rowspan="3">Port</th><th colspan="2">DMX</th><th colspan="5">RDM</th></tr><tr><th rowspan="2">Sent</th><th rowspan="2">Received</th><th colspan="2">Sent</th><th colspan="3">Received</th></tr><tr><th>Class</th><th>Discovery</th><th>Good</th><th>Bad</th><th>Discovery</th></tr>'
    tdd=''
    
    tr.forEach(p => {
      tdd+=`<tr><td>${p.port}</td><td>${p.d.sent}</td><td>${p.d.received}</td><td>${p.r.sent.class}</td><td>${p.r.sent.discovery}</td><td>${p.r.received.good}</td><td>${p.r.received.bad}</td><td>${p.r.received.discovery}</td></tr>`;
    });
        
    document.getElementById("idPorts").innerHTML=hdrs+tdd
    
  } catch (error){}
}