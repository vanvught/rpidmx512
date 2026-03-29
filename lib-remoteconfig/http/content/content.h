#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>
#include "httpd/httpd.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "dmx.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#include "rtc.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#include "date.js.h"
#if defined (NODE_SHOWFILE)
#include "showfile.html.h"
#endif /* (NODE_SHOWFILE) */
#include "default.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#include "rtc.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
#include "rdm.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
#include "time.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
#include "rdm.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#include "index.html.h"
#if defined (NODE_SHOWFILE)
#include "showfile.js.h"
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
#include "time.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "dmx.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */

struct FilesContent {
	uint32_t hash;
	const char *file_name;
	const char *content;
	uint32_t content_length;
	http::ContentTypes content_type;
};

inline constexpr struct FilesContent kHttpContent[] = {
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 449885855,"dmx.js", dmx_js, 1357, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ 4194541735,"rtc.html", rtc_html, 1013, static_cast<http::ContentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
	{ 2932864356,"static.js", static_js, 1252, static_cast<http::ContentTypes>(2) },
	{ 2557875310,"styles.css", styles_css, 409, static_cast<http::ContentTypes>(1) },
	{ 247271700,"index.js", index_js, 1168, static_cast<http::ContentTypes>(2) },
	{ 1602327546,"date.js", date_js, 716, static_cast<http::ContentTypes>(2) },
#if defined (NODE_SHOWFILE)
	{ 92830953,"showfile.html", showfile_html, 1386, static_cast<http::ContentTypes>(0) },
#endif /* (NODE_SHOWFILE) */
	{ 135667591,"default.js", default_js, 261, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ 2872131065,"rtc.js", rtc_js, 843, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ 3336539475,"rdm.js", rdm_js, 1001, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ 1797555997,"time.js", time_js, 402, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 2632765249,"pixeltype.json", pixeltype_json, 1665, static_cast<http::ContentTypes>(3) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ 3794898249,"rdm.html", rdm_html, 1142, static_cast<http::ContentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
	{ 4024653090,"index.html", index_html, 669, static_cast<http::ContentTypes>(0) },
#if defined (NODE_SHOWFILE)
	{ 4266521075,"showfile.js", showfile_js, 1425, static_cast<http::ContentTypes>(2) },
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ 3879424355,"time.html", time_html, 599, static_cast<http::ContentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_TIME) */
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 1521353325,"dmx.html", dmx_html, 538, static_cast<http::ContentTypes>(0) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
};


#endif /* CONTENT_H_ */
