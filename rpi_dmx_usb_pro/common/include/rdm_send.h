
#ifndef RDM_SEND_H_
#define RDM_SEND_H_

#include <stdint.h>

extern void rdm_send_discovery_msg(const uint8_t *, const uint16_t);
extern void rdm_send_respond_message_ack();
extern void rdm_send_respond_message_nack(const uint16_t);
extern void rdm_send_increment_message_count();
extern void rdm_send_decrement_message_count();

#endif /* RDM_SEND_H_ */
