let uploadAbortController = null;

function uploadEl(id) {
	return document.getElementById(id);
}

function setUploadStatus(text) {
	const el = uploadEl('uploadStatus');
	if (el) el.textContent = text;
}

function logUpload(text) {
	const el = uploadEl('output');
	if (!el) return;
	el.textContent += text + '\n';
	el.scrollTop = el.scrollHeight;
}

function clearUploadLog() {
	const el = uploadEl('output');
	if (el) el.textContent = '';
}

function setUploadProgress(done, total) {
	const percent = total > 0 ? Math.floor((done * 100) / total) : 0;
	const bar = uploadEl('progressBar');
	if (bar) bar.style.width = percent + '%';
}

function formatUploadSize(bytes) {
	if (!Number.isFinite(bytes)) return '-';
	const units = ['B', 'KB', 'MB', 'GB'];
	let size = bytes;
	let unit = 0;
	while (size >= 1024 && unit < units.length - 1) {
		size /= 1024;
		unit++;
	}
	return (unit === 0 ? size.toString() : size.toFixed(1)) + ' ' + units[unit];
}

function selectedFirmwareFile() {
	const input = uploadEl('firmwareInput');
	return input && input.files && input.files.length > 0 ? input.files[0] : null;
}

function refreshFirmwareInfo() {
	const file = selectedFirmwareFile();
	uploadEl('fileName').textContent = file ? file.name : '-';
	uploadEl('fileSize').textContent = file ? formatUploadSize(file.size) : '-';
	setUploadProgress(0, 1);
	setUploadStatus(file ? 'Ready' : 'Idle');
}

function setUploadBusy(busy) {
	const uploadButton = uploadEl('uploadButton');
	const cancelButton = uploadEl('cancelButton');
	const fileInput = uploadEl('firmwareInput');

	if (uploadButton) uploadButton.disabled = busy;
	if (cancelButton) cancelButton.disabled = !busy;
	if (fileInput) fileInput.disabled = busy;
}

function cancelUpload() {
	if (uploadAbortController) {
		uploadAbortController.abort();
	}
}

async function checkedFetch(url, options, errorText) {
	const res = await fetch(url, options);
	if (!res.ok) {
		const body = await res.text().catch(() => '');
		throw new Error(errorText + ': HTTP ' + res.status + (body ? ' - ' + body : ''));
	}
	return res;
}

async function uploadFirmware() {
	const file = selectedFirmwareFile();
	if (!file) {
		setUploadStatus('No file selected');
		clearUploadLog();
		logUpload('Please select a firmware file.');
		return;
	}

	const chunkSize = 1024;
	const totalChunks = Math.ceil(file.size / chunkSize);
	uploadAbortController = new AbortController();
	const signal = uploadAbortController.signal;

	setUploadBusy(true);
	setUploadProgress(0, file.size);
	setUploadStatus('Starting');
	clearUploadLog();
	logUpload('Starting upload: ' + file.name + ' (' + formatUploadSize(file.size) + ')');

	try {
		await checkedFetch('/upload_start', {
			method: 'POST',
			headers: {
				'X-Upload-Size': file.size.toString(),
				'X-Upload-Name': file.name
			},
			signal: signal
		}, 'Upload start failed');

		for (let offset = 0; offset < file.size; offset += chunkSize) {
			const chunk = file.slice(offset, offset + chunkSize);
			const chunkNumber = Math.floor(offset / chunkSize) + 1;

			setUploadStatus('Uploading chunk ' + chunkNumber + ' / ' + totalChunks);
			await checkedFetch('/upload', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/octet-stream',
					'X-Upload-Offset': offset.toString(),
					'X-Upload-Chunk': chunkNumber.toString(),
					'X-Upload-Chunks': totalChunks.toString()
				},
				body: chunk,
				signal: signal
			}, 'Chunk upload failed at offset ' + offset);

			const uploaded = Math.min(offset + chunk.size, file.size);
			setUploadProgress(uploaded, file.size);
			logUpload('Uploaded chunk ' + chunkNumber + ' / ' + totalChunks);
		}

		setUploadStatus('Completing');
		await checkedFetch('/upload_complete', { method: 'POST', signal: signal }, 'Upload complete failed');
		setUploadProgress(file.size, file.size);
		setUploadStatus('Complete');
		logUpload('Upload complete.');
	} catch (err) {
		if (err.name === 'AbortError') {
			setUploadStatus('Cancelled');
			logUpload('Upload cancelled.');
		} else {
			setUploadStatus('Error');
			logUpload('Error: ' + err.message);
		}
	} finally {
		uploadAbortController = null;
		setUploadBusy(false);
	}
}

function initUploadPage() {
	const input = uploadEl('firmwareInput');
	const dropZone = uploadEl('dropZone');

	if (input) {
		input.addEventListener('change', refreshFirmwareInfo);
	}

	if (dropZone && input) {
		['dragenter', 'dragover'].forEach(function(name) {
			dropZone.addEventListener(name, function(event) {
				event.preventDefault();
				dropZone.classList.add('dragover');
			});
		});

		['dragleave', 'drop'].forEach(function(name) {
			dropZone.addEventListener(name, function(event) {
				event.preventDefault();
				dropZone.classList.remove('dragover');
			});
		});

		dropZone.addEventListener('drop', function(event) {
			if (!event.dataTransfer || event.dataTransfer.files.length === 0) return;
			input.files = event.dataTransfer.files;
			refreshFirmwareInfo();
		});
	}

	refreshFirmwareInfo();
}
