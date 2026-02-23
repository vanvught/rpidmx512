function reset(sel) {
	var d = {};
	var out = {};
	out[sel] = d;
	var payload = JSON.stringify(out);
	fetch('/json/' + sel, {
		method: 'POST',
		headers: {
			'Content-Type': 'application/json'
		},
		body: payload
	}) .then(response => {if (response.ok) { get_txt(sel); }});
}