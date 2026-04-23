#ifndef CONTENT_H_
#define CONTENT_H_

#include <cstdint>
#include "httpd/httpd.h"
#if defined (NODE_SHOWFILE)
#include "status_showfile.js.h"
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "status_dmx.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "status_pixel.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined (NODE_LTC_SMPTE)
#include "config_etc.js.h"
#endif // (NODE_LTC_SMPTE)
#include "styles.css.h"
#if defined (NODE_OSC_SERVER)
#include "config_oscserver.js.h"
#endif // (NODE_OSC_SERVER)
#include "index.js.h"
#include "date.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "config_dmxpixel.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "config_artnet.js.h"
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
#include "status_index.html.h"
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#include "rtc_index.html.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "config_dmxmonitor.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "index.html.h"
#include "layout.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#include "config_dmxpca9685.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#if defined (DISPLAY_UDF)
#include "config_display.js.h"
#endif // (DISPLAY_UDF)
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
#include "time_index.html.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_TIME)
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
#if defined (NODE_LTC_SMPTE)
#include "config_tcnet.js.h"
#endif // (NODE_LTC_SMPTE)
#include "common.js.h"
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
#if defined (NODE_SHOWFILE)
	{ 1021537480,"status_showfile.js", status_showfile_js, 944, static_cast<http::ContentTypes>(2) },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 3583512354,"status_dmx.js", status_dmx_js, 1532, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 1436064737,"status_pixel.js", status_pixel_js, 570, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined (NODE_LTC_SMPTE)
	{ 109007079,"config_etc.js", config_etc_js, 1789, static_cast<http::ContentTypes>(2) },
#endif // (NODE_LTC_SMPTE)
	{ 2557875310,"styles.css", styles_css, 2170, static_cast<http::ContentTypes>(1) },
#if defined (NODE_OSC_SERVER)
	{ 3728864745,"config_oscserver.js", config_oscserver_js, 1395, static_cast<http::ContentTypes>(2) },
#endif // (NODE_OSC_SERVER)
	{ 247271700,"index.js", index_js, 5044, static_cast<http::ContentTypes>(2) },
	{ 1602327546,"date.js", date_js, 716, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 4089447436,"config_dmxpixel.js", config_dmxpixel_js, 1966, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 2110753961,"config_artnet.js", config_artnet_js, 2816, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 4041167307,"status_index.html", status_index_html, 1698, static_cast<http::ContentTypes>(0) },
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ 1233896314,"rtc_index.html", rtc_index_html, 408, static_cast<http::ContentTypes>(0) },
#endif // !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 2632765249,"pixeltype.json", pixeltype_json, 1665, static_cast<http::ContentTypes>(3) },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 1186793998,"config_dmxmonitor.js", config_dmxmonitor_js, 1071, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 4024653090,"index.html", index_html, 1873, static_cast<http::ContentTypes>(0) },
	{ 2226674728,"layout.js", layout_js, 1374, static_cast<http::ContentTypes>(2) },
#if !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
	{ 1666579856,"config_dmxpca9685.js", config_dmxpca9685_js, 2758, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#if defined (DISPLAY_UDF)
	{ 2902032383,"config_display.js", config_display_js, 3757, static_cast<http::ContentTypes>(2) },
#endif // (DISPLAY_UDF)
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ 2057353734,"time_index.html", time_index_html, 1188, static_cast<http::ContentTypes>(0) },
#endif // !defined (CONFIG_HTTP_HTML_NO_TIME)
#if defined (DMXNODE_PORTS)
	{ 3906326706,"config_dmxnode.js", config_dmxnode_js, 2659, static_cast<http::ContentTypes>(2) },
#endif // (DMXNODE_PORTS)
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
	{ 1557939568,"config_rdmdevice.js", config_rdmdevice_js, 615, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER || defined (RDM_RESPONDER))
#if defined (NODE_SHOWFILE)
	{ 1821390800,"config_showfile.js", config_showfile_js, 1466, static_cast<http::ContentTypes>(2) },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 381868932,"config_dmxsend.js", config_dmxsend_js, 1617, static_cast<http::ContentTypes>(2) },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if defined (NODE_LTC_SMPTE)
	{ 3844231377,"config_tcnet.js", config_tcnet_js, 1634, static_cast<http::ContentTypes>(2) },
#endif // (NODE_LTC_SMPTE)
	{ 3116920429,"common.js", common_js, 2411, static_cast<http::ContentTypes>(2) },
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4064453935,"config_e131.js", config_e131_js, 1077, static_cast<http::ContentTypes>(2) },
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4156522053,"status_index.js", status_index_js, 3168, static_cast<http::ContentTypes>(2) },
};


#endif /* CONTENT_H_ */
