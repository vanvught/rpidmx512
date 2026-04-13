#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>
#include "httpd/httpd.h"
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#include "e131.js.h"
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "dmxsend.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "styles.css.h"
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "artnet.js.h"
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "index.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "dmxmonitor.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined (DMXNODE_PORTS)
#include "dmxnode.js.h"
#endif // (DMXNODE_PORTS)
#if defined (DISPLAY_UDF)
#include "display.js.h"
#endif // (DISPLAY_UDF)
#include "index.html.h"
#if defined (NODE_SHOWFILE)
#include "showfile.js.h"
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "dmxpixel.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
#include "rdmdevice.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)

struct FilesContent {
	uint32_t hash;
	const char *file_name;
	const char *content;
	uint32_t content_length;
	http::ContentTypes content_type;
};

inline constexpr struct FilesContent kHttpContent[] = {
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 1936665420,"e131.js", e131_js, 1460, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 1292863077,"dmxsend.js", dmxsend_js, 2573, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 2557875310,"styles.css", styles_css, 1509, static_cast<http::ContentTypes>(1) },
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 2310954254,"artnet.js", artnet_js, 3163, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 247271700,"index.js", index_js, 5394, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 3515266689,"dmxmonitor.js", dmxmonitor_js, 1731, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 2632765249,"pixeltype.json", pixeltype_json, 1665, static_cast<http::ContentTypes>(3) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined (DMXNODE_PORTS)
	{ 1113597067,"dmxnode.js", dmxnode_js, 3597, static_cast<http::ContentTypes>(2) },
#endif // (DMXNODE_PORTS)
#if defined (DISPLAY_UDF)
	{ 3809887750,"display.js", display_js, 4477, static_cast<http::ContentTypes>(2) },
#endif // (DISPLAY_UDF)
	{ 4024653090,"index.html", index_html, 1810, static_cast<http::ContentTypes>(0) },
#if defined (NODE_SHOWFILE)
	{ 4266521075,"showfile.js", showfile_js, 2201, static_cast<http::ContentTypes>(2) },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 434885195,"dmxpixel.js", dmxpixel_js, 3108, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
	{ 2747196229,"rdmdevice.js", rdmdevice_js, 511, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
};


#endif /* CONTENT_H_ */
