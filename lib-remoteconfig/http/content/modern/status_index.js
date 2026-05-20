async function loadCore(core) {
	const json = await getJSON('status/' + core);
	if (!json) return;
	Object.keys(json).forEach(function(key) {
		setValue(key, json[key]);
	});
}

async function loadGeneral() {
	await loadCore('display');
	await loadCore('identify');
}

async function loadCores() {
	await loadGeneral();
	await loadCore('phy');
	await loadCore('emac');
}

function loadModules() {
	loadModulesFrom('status/directory', ['status/identify', 'status/display', 'status/phy', 'status/emac'], { readonly: true });
}
