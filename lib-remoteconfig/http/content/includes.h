#if defined (NODE_SHOWFILE)
#include "status_showfile.js.h"
#endif // (NODE_SHOWFILE)
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "status_dmx.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "status_pixel.js.h"
#endif // !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "styles.css.h"
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
#include "common.js.h"
#if defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#include "config_e131.js.h"
#endif // defined(NODE_E131) || defined(NODE_E131_MULTI) || (ARTNET_VERSION == 4)
#include "status_index.js.h"
