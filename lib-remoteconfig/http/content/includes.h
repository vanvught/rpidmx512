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
