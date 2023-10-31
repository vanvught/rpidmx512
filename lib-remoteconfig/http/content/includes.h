#include "static.js.h"
#include "styles.css.h"
#include "index.js.h"
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.js.h"
#endif /* (ENABLE_PHY_SWITCH) */
#include "default.js.h"
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
# include "rdm.js.h"
#endif /* RDM_CONTROLLER || RDM_RESPONDER */
#if defined (RDM_CONTROLLER) || defined (RDM_RESPONDER)
# include "rdm.html.h"
#endif /* RDM_CONTROLLER || RDM_RESPONDER */
#include "index.html.h"
#if defined (ENABLE_PHY_SWITCH)
# include "dsa.html.h"
#endif /* (ENABLE_PHY_SWITCH) */
