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
#if defined (NODE_LTC_SMPTE)
#include "config_ltcdisplays.js.h"
#endif // (NODE_LTC_SMPTE)
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#include "rtc_index.html.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "config_dmxmonitor.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "config_oscclient.js.h"
#include "index.html.h"
#include "layout.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#include "config_dmxpca9685.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#if defined (NODE_LTC_SMPTE)
#include "config_gps.js.h"
#endif // (NODE_LTC_SMPTE)
#if defined (DISPLAY_UDF)
#include "config_display.js.h"
#endif // (DISPLAY_UDF)
#if defined (NODE_LTC_SMPTE)
#include "config_ltc.js.h"
#endif // (NODE_LTC_SMPTE)
#if defined (CONFIG_HTTPD_ENABLE_UPLOAD)
#include "upload_index.js.h"
#endif // (CONFIG_HTTPD_ENABLE_UPLOAD)
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
#if defined (CONFIG_HTTPD_ENABLE_UPLOAD)
#include "upload_index.html.h"
#endif // (CONFIG_HTTPD_ENABLE_UPLOAD)

struct FilesContent {
	uint32_t hash;
	const char *file_name;
	const uint8_t *content;
	uint32_t content_length;
	http::ContentTypes content_type;
	bool gzip;
};

inline constexpr struct FilesContent kHttpContent[] = {
#if defined (NODE_SHOWFILE)
	{ 1021537480,"status_showfile.js", status_showfile_js_gz, 570, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 3583512354,"status_dmx.js", status_dmx_js_gz, 868, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 1436064737,"status_pixel.js", status_pixel_js_gz, 350, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined (NODE_LTC_SMPTE)
	{ 109007079,"config_etc.js", config_etc_js_gz, 593, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_LTC_SMPTE)
	{ 2557875310,"styles.css", styles_css_gz, 969, static_cast<http::ContentTypes>(1), true },
#if defined (NODE_OSC_SERVER)
	{ 3728864745,"config_oscserver.js", config_oscserver_js_gz, 587, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_OSC_SERVER)
	{ 247271700,"index.js", index_js_gz, 1045, static_cast<http::ContentTypes>(2), true },
	{ 1602327546,"date.js", date_js_gz, 325, static_cast<http::ContentTypes>(2), true },
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 4089447436,"config_dmxpixel.js", config_dmxpixel_js_gz, 828, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 2110753961,"config_artnet.js", config_artnet_js_gz, 1034, static_cast<http::ContentTypes>(2), true },
#endif // defined(NODE_ARTNET) || defined(NODE_ARTNET_MULTI)
	{ 4041167307,"status_index.html", status_index_html_gz, 604, static_cast<http::ContentTypes>(0), true },
#if defined (NODE_LTC_SMPTE)
	{ 1194496967,"config_ltcdisplays.js", config_ltcdisplays_js_gz, 565, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_LTC_SMPTE)
#if !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
	{ 1233896314,"rtc_index.html", rtc_index_html_gz, 239, static_cast<http::ContentTypes>(0), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_RTC) && !defined (DISABLE_RTC)
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
	{ 2632765249,"pixeltype.json", pixeltype_json_gz, 277, static_cast<http::ContentTypes>(3), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 1186793998,"config_dmxmonitor.js", config_dmxmonitor_js_gz, 538, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
	{ 3282278349,"config_oscclient.js", config_oscclient_js_gz, 718, static_cast<http::ContentTypes>(2), true },
	{ 4024653090,"index.html", index_html_gz, 725, static_cast<http::ContentTypes>(0), true },
	{ 2226674728,"layout.js", layout_js_gz, 570, static_cast<http::ContentTypes>(2), true },
#if !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
	{ 1666579856,"config_dmxpca9685.js", config_dmxpca9685_js_gz, 704, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX_PCA9685) && defined(OUTPUT_DMX_PCA9685)
#if defined (NODE_LTC_SMPTE)
	{ 610379173,"config_gps.js", config_gps_js_gz, 559, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_LTC_SMPTE)
#if defined (DISPLAY_UDF)
	{ 2902032383,"config_display.js", config_display_js_gz, 1422, static_cast<http::ContentTypes>(2), true },
#endif // (DISPLAY_UDF)
#if defined (NODE_LTC_SMPTE)
	{ 1947358610,"config_ltc.js", config_ltc_js_gz, 1913, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_LTC_SMPTE)
#if defined (CONFIG_HTTPD_ENABLE_UPLOAD)
	{ 1750204712,"upload_index.js", upload_index_js_gz, 1631, static_cast<http::ContentTypes>(2), true },
#endif // (CONFIG_HTTPD_ENABLE_UPLOAD)
#if !defined (CONFIG_HTTP_HTML_NO_TIME)
	{ 2057353734,"time_index.html", time_index_html_gz, 520, static_cast<http::ContentTypes>(0), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_TIME)
#if defined (DMXNODE_PORTS)
	{ 3906326706,"config_dmxnode.js", config_dmxnode_js_gz, 1014, static_cast<http::ContentTypes>(2), true },
#endif // (DMXNODE_PORTS)
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER) || defined (RDM_RESPONDER))
	{ 1557939568,"config_rdmdevice.js", config_rdmdevice_js_gz, 433, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_RDM) && (defined (RDM_CONTROLLER || defined (RDM_RESPONDER))
#if defined (NODE_SHOWFILE)
	{ 1821390800,"config_showfile.js", config_showfile_js_gz, 535, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
	{ 381868932,"config_dmxsend.js", config_dmxsend_js_gz, 662, static_cast<http::ContentTypes>(2), true },
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if defined (NODE_LTC_SMPTE)
	{ 3844231377,"config_tcnet.js", config_tcnet_js_gz, 618, static_cast<http::ContentTypes>(2), true },
#endif // (NODE_LTC_SMPTE)
	{ 3116920429,"common.js", common_js_gz, 1448, static_cast<http::ContentTypes>(2), true },
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4064453935,"config_e131.js", config_e131_js_gz, 641, static_cast<http::ContentTypes>(2), true },
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
	{ 4156522053,"status_index.js", status_index_js_gz, 273, static_cast<http::ContentTypes>(2), true },
#if defined (CONFIG_HTTPD_ENABLE_UPLOAD)
	{ 137651886,"upload_index.html", upload_index_html_gz, 563, static_cast<http::ContentTypes>(0), true },
#endif // (CONFIG_HTTPD_ENABLE_UPLOAD)
};


#endif /* CONTENT_H_ */
