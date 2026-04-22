(function() {
    async function getJSON(path) {
        try {
            const r = await fetch('/json/' + path);
            return r.ok ? await r.json() : null;
        } catch {
            return null;
        }
    }

    async function postJSON(path, obj) {
        try {
            const r = await fetch(path, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(obj)
            });
            return r.ok;
        } catch {
            return false;
        }
    }

    function setFieldValue(field, value) {
        if (field.type === 'checkbox') {
            field.checked = !!value;
        } else {
            field.value = value ?? '';
        }
    }

    function fillDataKeys(root, json) {
        const fields = root.querySelectorAll('[data-key]');

        for (let i = 0; i < fields.length; i++) {
            const field = fields[i];
            const key = field.dataset.key;

            if (json[key] !== undefined) {
                setFieldValue(field, json[key]);
            }
        }
    }

    function validateFields(fields) {
        for (let i = 0; i < fields.length; i++) {
            const field = fields[i];
            field.setCustomValidity('');

            if (!field.checkValidity()) {
                field.reportValidity();
                return false;
            }
        }

        return true;
    }

    function collectDataKeys(root) {
        const fields = root.querySelectorAll('[data-key]');
        if (!validateFields(fields)) {
            return null;
        }

        const out = {};

        for (let i = 0; i < fields.length; i++) {
            const field = fields[i];
            const key = field.dataset.key;

            if (field.type === 'checkbox') {
                out[key] = field.checked ? 1 : 0;
            } else if (field.type === 'number') {
                out[key] = +field.value;
            } else {
                out[key] = field.value.trim();
            }
        }

        return out;
    }

    async function saveDataKeyForm(path, card, options) {
        const out = collectDataKeys(card);
        if (!out) return false;

        if (options && options.beforePost && options.beforePost(out, card) === false) {
            return false;
        }

        const btn = card.querySelector("button[type='submit']");
        if (btn) {
            btn.disabled = true;
            btn.textContent = 'Saving...';
        }

        try {
            const ok = await postJSON('json/' + path, out);
            if (!ok) {
                console.log('Save failed');
                return false;
            }

            const json = await getJSON(path);
            if (!json) return true;

            if (options && options.afterLoad) {
                options.afterLoad(card, json);
            } else {
                fillDataKeys(card, json);
            }

            return true;
        } catch (e) {
            console.log('Error:', e);
            return false;
        } finally {
            if (btn) {
                btn.disabled = false;
                btn.textContent = 'Save';
            }
        }
    }

    window.getJSON = getJSON;
    window.postJSON = postJSON;
    window.setFieldValue = setFieldValue;
    window.fillDataKeys = fillDataKeys;
    window.validateFields = validateFields;
    window.collectDataKeys = collectDataKeys;
    window.saveDataKeyForm = saveDataKeyForm;
})();
