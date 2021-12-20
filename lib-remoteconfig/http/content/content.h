#include "styles.css.h"
#include "index.js.h"
#include "index.html.h"

struct FilesContent {
	const char *pFileName;
	const char *pContent;
};

static constexpr struct FilesContent HttpContent[] = {
	{ "styles.css", styles_css },
	{ "index.js", index_js },
	{ "index.html", index_html },
};