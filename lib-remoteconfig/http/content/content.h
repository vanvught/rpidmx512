#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>

#include "httpd/httpd.h"

#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
# include "dmx.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
# include "rtc.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#include "date.js.h"
#if defined (NODE_SHOWFILE)
# include "showfile.html.h"
#endif /* (NODE_SHOWFILE) */
#include "default.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
# include "rtc.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
# include "time.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#include "index.html.h"
#if defined (NODE_SHOWFILE)
# include "showfile.js.h"
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
# include "time.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
# include "dmx.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */

struct FilesContent {
	const char *pFileName;
	const char *pContent;
	const uint32_t nContentLength;
	const http::contentTypes contentType;
};

static constexpr struct FilesContent HttpContent[] = {
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ "dmx.js", dmx_js, 1357, static_cast<http::contentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ "rtc.html", rtc_html, 1013, static_cast<http::contentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
	{ "static.js", static_js, 1252, static_cast<http::contentTypes>(2) },
	{ "styles.css", styles_css, 409, static_cast<http::contentTypes>(1) },
	{ "index.js", index_js, 1168, static_cast<http::contentTypes>(2) },
	{ "date.js", date_js, 716, static_cast<http::contentTypes>(2) },
#if defined (NODE_SHOWFILE)
	{ "showfile.html", showfile_html, 1386, static_cast<http::contentTypes>(0) },
#endif /* (NODE_SHOWFILE) */
	{ "default.js", default_js, 261, static_cast<http::contentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ "rtc.js", rtc_js, 843, static_cast<http::contentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ "rdm.js", rdm_js, 1001, static_cast<http::contentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ "time.js", time_js, 402, static_cast<http::contentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ "rdm.html", rdm_html, 1142, static_cast<http::contentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
	{ "index.html", index_html, 669, static_cast<http::contentTypes>(0) },
#if defined (NODE_SHOWFILE)
	{ "showfile.js", showfile_js, 1425, static_cast<http::contentTypes>(2) },
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ "time.html", time_html, 599, static_cast<http::contentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ "dmx.html", dmx_html, 538, static_cast<http::contentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
};

#endif /* CONTENT_H_ */
