function post(u, j) {
    return fetch('/json/' + u, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(j)
    })
}

function delet(s) {
    return fetch('/json/action', {
        method: 'DELETE',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(s)
    })
}

function reboot() {
    post('action', { reboot: 1 })
}

function locate() {
    var b = document.getElementById('locateButton');
    if (b.classList.contains('inactive')) {
        b.classList.remove('inactive')
        b.classList.add('active')
        b.innerHTML = 'Locate On'
        post('action', { identify: 1 })
    } else {
        b.classList.remove('active')
        b.classList.add('inactive')
        b.innerHTML = 'Locate Off'
        post('action', { identify: 0 })
    }
}