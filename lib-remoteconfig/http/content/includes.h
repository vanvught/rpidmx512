#include "e131.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI))
#include "dmxsend.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && (defined(OUTPUT_DMX_SEND) || defined(OUTPUT_DMX_SEND_MULTI)) */
#include "styles.css.h"
#include "index.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR)
#include "dmxmonitor.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_DMX) && defined(OUTPUT_DMX_MONITOR) */
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "pixeltype.json.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)) */
#include "index.html.h"
#if defined (NODE_SHOWFILE)
#include "showfile.js.h"
#endif /* (NODE_SHOWFILE) */
#if !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI))
#include "dmxpixel.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_PIXEL) && (defined(OUTPUT_DMX_PIXEL) || defined(OUTPUT_DMX_PIXEL_MULTI)) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
#include "rdmdevice.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
