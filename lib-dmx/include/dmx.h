/*
 * dmx.h
 */

#ifndef DMX_H_
#define DMX_H_

#if defined (OUTPUT_DMX_SEND_MULTI)
# error
#else
# if defined (H3)
#  include "h3/1port/dmx.h"
# elif defined(RPI1) || defined (RPI2)
#  include "rpi/1port/dmx.h"
# else
#  include "linux/1port/dmx.h"
# endif
#endif

#endif /* DMX_H_ */
