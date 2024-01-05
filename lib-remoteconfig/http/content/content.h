#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>

#include "httpd/httpd.h"

#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.js.h"
#endif /* (ENABLE_PHY_SWITCH) */
#include "default.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#include "index.html.h"
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.html.h"
#endif /* (ENABLE_PHY_SWITCH) */

struct FilesContent {
	const char *pFileName;
	const char *pContent;
	const uint32_t nContentLength;
	const http::contentTypes contentType;
};

static constexpr struct FilesContent HttpContent[] = {
	{ "static.js", static_js, 1072, static_cast<http::contentTypes>(2) },
	{ "styles.css", styles_css, 409, static_cast<http::contentTypes>(1) },
	{ "index.js", index_js, 1140, static_cast<http::contentTypes>(2) },
#if defined (ENABLE_PHY_SWITCH)
	{ "dsa.js", dsa_js, 613, static_cast<http::contentTypes>(2) },
#endif /* (ENABLE_PHY_SWITCH) */
	{ "default.js", default_js, 254, static_cast<http::contentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ "rdm.js", rdm_js, 1025, static_cast<http::contentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ "rdm.html", rdm_html, 1142, static_cast<http::contentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
	{ "index.html", index_html, 669, static_cast<http::contentTypes>(0) },
#if defined (ENABLE_PHY_SWITCH)
	{ "dsa.html", dsa_html, 447, static_cast<http::contentTypes>(0) },
#endif /* (ENABLE_PHY_SWITCH) */
};

#endif /* CONTENT_H_ */
