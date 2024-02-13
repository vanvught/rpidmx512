#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.js.h"
#endif /* (ENABLE_PHY_SWITCH) */
#if defined (NODE_SHOWFILE)
#include "showfile.html.h"
#endif /* (NODE_SHOWFILE) */
#include "default.js.h"
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.js.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#if !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER)
# include "rdm.html.h"
#endif /* !defined (CONFIG_HTTP_HTML_NO_RDM) && defined (RDM_CONTROLLER) */
#include "index.html.h"
#if defined (NODE_SHOWFILE)
#include "showfile.js.h"
#endif /* (NODE_SHOWFILE) */
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.html.h"
#endif /* (ENABLE_PHY_SWITCH) */
