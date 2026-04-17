#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>
#include "httpd/httpd.h"
#include "styles.css.h"
#include "index.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "config_dmxpixel.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "config_artnet.js.h"
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "status_index.html.h"
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "config_dmxmonitor.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "index.html.h"
#if defined (DISPLAY_UDF)
#include "config_display.js.h"
#endif // (DISPLAY_UDF)
#if defined (DMXNODE_PORTS)
#include "config_dmxnode.js.h"
#endif // (DMXNODE_PORTS)
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
#include "config_rdmdevice.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER || defined (RDM_RESPONDER))
#if defined (NODE_SHOWFILE)
#include "config_showfile.js.h"
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "config_dmxsend.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#include "config_e131.js.h"
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#include "status_index.js.h"

struct FilesContent {
	uint32_t hash;
	const char *file_name;
	const char *content;
	uint32_t content_length;
	http::ContentTypes content_type;
};

inline constexpr struct FilesContent kHttpContent[] = {
	{ 2557875310,"styles.css", styles_css, 1509, static_cast<http::ContentTypes>(1) },
	{ 247271700,"index.js", index_js, 5394, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 4089447436,"config_dmxpixel.js", config_dmxpixel_js, 2938, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 2110753961,"config_artnet.js", config_artnet_js, 4322, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 4041167307,"status_index.html", status_index_html, 1225, static_cast<http::ContentTypes>(0) },
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 2632765249,"pixeltype.json", pixeltype_json, 1665, static_cast<http::ContentTypes>(3) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 1186793998,"config_dmxmonitor.js", config_dmxmonitor_js, 2051, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 4024653090,"index.html", index_html, 1816, static_cast<http::ContentTypes>(0) },
#if defined (DISPLAY_UDF)
	{ 2902032383,"config_display.js", config_display_js, 4477, static_cast<http::ContentTypes>(2) },
#endif // (DISPLAY_UDF)
#if defined (DMXNODE_PORTS)
	{ 3906326706,"config_dmxnode.js", config_dmxnode_js, 4332, static_cast<http::ContentTypes>(2) },
#endif // (DMXNODE_PORTS)
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
	{ 1557939568,"config_rdmdevice.js", config_rdmdevice_js, 1597, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER || defined (RDM_RESPONDER))
#if defined (NODE_SHOWFILE)
	{ 1821390800,"config_showfile.js", config_showfile_js, 2601, static_cast<http::ContentTypes>(2) },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 381868932,"config_dmxsend.js", config_dmxsend_js, 2607, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4064453935,"config_e131.js", config_e131_js, 2292, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4156522053,"status_index.js", status_index_js, 2247, static_cast<http::ContentTypes>(2) },
};


#endif /* CONTENT_H_ */
