#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>
#include "httpd/httpd.h"
#include "e131.js.h"
#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "dmxmonitor.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#include "default.js.h"
#include "index.html.h"
#if defined (NODE_SHOWFILE)
#include "showfile.js.h"
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
#include "rdmdevice.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */

struct FilesContent {
	uint32_t hash;
	const char *file_name;
	const char *content;
	uint32_t content_length;
	http::ContentTypes content_type;
};

inline constexpr struct FilesContent kHttpContent[] = {
	{ 1936665420,"e131.js", e131_js, 1466, static_cast<http::ContentTypes>(2) },
	{ 2932864356,"static.js", static_js, 696, static_cast<http::ContentTypes>(2) },
	{ 2557875310,"styles.css", styles_css, 1509, static_cast<http::ContentTypes>(1) },
	{ 247271700,"index.js", index_js, 4028, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 3515266689,"dmxmonitor.js", dmxmonitor_js, 1731, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
	{ 135667591,"default.js", default_js, 262, static_cast<http::ContentTypes>(2) },
	{ 4024653090,"index.html", index_html, 1706, static_cast<http::ContentTypes>(0) },
#if defined (NODE_SHOWFILE)
	{ 4266521075,"showfile.js", showfile_js, 2201, static_cast<http::ContentTypes>(2) },
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ 2747196229,"rdmdevice.js", rdmdevice_js, 511, static_cast<http::ContentTypes>(2) },
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
};


#endif /* CONTENT_H_ */
