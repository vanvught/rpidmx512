#include <cstdint>

#include "httpd/httpd.h"

#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#include "index.html.h"

struct FilesContent {
	const char *pFileName;
	const char *pContent;
	const uint32_t nContentLength;
	const http::contentTypes contentType;
};

static constexpr struct FilesContent HttpContent[] = {
	{ "static.js", static_js, 1065, static_cast<http::contentTypes>(2) },
	{ "styles.css", styles_css, 411, static_cast<http::contentTypes>(1) },
	{ "index.js", index_js, 1063, static_cast<http::contentTypes>(2) },
	{ "index.html", index_html, 611, static_cast<http::contentTypes>(0) },
};