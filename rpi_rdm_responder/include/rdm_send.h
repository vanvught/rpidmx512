
#ifndef RDM_SEND_H_
#define RDM_SEND_H_

#include <stdint.h>

extern void rdm_send_discovery_msg(const uint8_t *, const uint16_t);
extern void rdm_send_respond_message(const uint8_t *, uint8_t );

#endif /* RDM_SEND_H_ */
