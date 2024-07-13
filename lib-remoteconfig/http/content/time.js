async function refresh() {
  let data = await getJSON('timedate')
  const node = formatDateTime(new Date(data.date))
  document.getElementById('nodeTime').textContent = `Node Time: ${node}`
}

async function syncWithLocalTime() {
  const node = formatDateTime(new Date())
  document.getElementById('nodeTime').textContent = `Node Time: ${node}`
  const data = { date: node }
  await post(data)
  refresh()
}