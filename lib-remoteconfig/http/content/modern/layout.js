(function() {
    function navItem(href, label, active) {
        return '<li><a href="' + href + '"' + (active ? ' class="active"' : '') + '>' + label + '</a></li>';
    }

    function renderNav(activePage) {
        const el = document.getElementById('appNav');
        if (!el) return;

        el.innerHTML =
            '<nav>' +
                '<ul>' +
                    navItem('/config', 'Configuration', activePage === 'config') +
                    navItem('/status', 'Status', activePage === 'status') +
                    navItem('/time', 'Time', activePage === 'time') +
                    navItem('/rtc', 'RTC', activePage === 'rtc') +
                '</ul>' +
            '</nav>';
    }

    function renderHeader() {
        const el = document.getElementById('appHeader');
        if (!el) return;
        el.innerHTML = '<header><ul id="idList"></ul></header>';
    }

    function renderFooter() {
        const el = document.getElementById('appFooter');
        if (!el) return;
        el.innerHTML = '<footer><ul id="idVersion"></ul></footer>';
    }

    function renderActions() {
        const el = document.getElementById('appActions');
        if (!el) return;

        el.innerHTML =
            '<div>' +
                '<button id="locateButton" class="inactive" onclick="locate()">Locate Off</button>' +
                '<button onclick="reboot()">Reboot</button>' +
            '</div>';
    }

    function bootPage(activePage) {
        renderNav(activePage);
        renderHeader();
        renderFooter();
        renderActions();
    }

    window.renderNav = renderNav;
    window.renderHeader = renderHeader;
    window.renderFooter = renderFooter;
    window.renderActions = renderActions;
    window.bootPage = bootPage;
})();
