async function refresh() {
  let data = await getJSON('dsa/portstatus')
  let h = '<table><tr><th>Port</th><th>Link</th><th>Speed</th><th>Duplex</th><th>Flow Control</th></tr>';
  data.forEach(item => {
    h += `<tr><td>${item.port}</td><td>${item.link}</td><td>${item.speed}</td><td>${item.duplex}</td><td>${item.flowcontrol}</td></tr>`;
  });
  h += '</table>'
  
  h += '<br>'
  
  data = await getJSON('dsa/vlantable')
  h += '<table><tr><th>Port</th><th>VLANTable</th></tr>';
  data.forEach(item => {
    h += `<tr><td>${item.port}</td><td>${item.VLANTable}</td></tr>`;
  });
  h += '</table>'
  document.getElementById("idTxt").innerHTML = h;
}