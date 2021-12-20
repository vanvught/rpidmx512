async function getJSON(json) {
		let url='/json/' + json;
		try {
				let res=await fetch(url);
				return await res.json();
		} catch (error) {
				console.log(error);
		}
}

async function renderList() {
		let list=await getJSON('list');
		document.getElementById("idList").innerHTML="<li>"+list.list.name+"</li><li>"+list.list.node.type+"</li><li>"+list.list.node.port.type+"</li>";
}

async function renderDirectory() {
			let directory=await getJSON('directory');
			let html=""
			let filenames=Object.keys(directory["files"]);
			filenames.forEach(function(key) {
					var value = directory["files"][key];
					html+="<option value="+key+">"+value+"</option>";
			});
			document.getElementById("idDirectory").innerHTML=html;
			get_txt(filenames[0]);
}

async function renderVersion() {
		let version=await getJSON('version');
		document.getElementById("idVersion").innerHTML="<li>V"+version.version+"</li><li>"+version.build.date+"</li><li>"+version.build.time+"</li><li>"+version.board+"</li>";
}

async function get_txt(sel) {
	let txt=await getJSON(sel);
	let html="";
	Object.keys(txt[sel]).forEach(function(key) {
		var value = txt[sel][key];
		 html+="<tr><td>"+key+"</td><td>"+value+"</td></tr>";
	});
	document.getElementById("idTxt").innerHTML=html;
}

renderList();
renderDirectory();
renderVersion();
