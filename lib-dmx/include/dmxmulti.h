/*
 * dmxmulti.h
 */

#ifndef DMXMULTI_H_
#define DMXMULTI_H_

#if defined (OUTPUT_DMX_SEND_MULTI)
# if defined (H3)
#  include "h3/4ports/dmx.h"
# elif defined (GD32)
#  include "gd32/4ports/dmx.h"
# else
#  include "linux/4ports/dmx.h"
# endif
#else
# error
#endif

#endif /* DMXMULTI_H_ */
