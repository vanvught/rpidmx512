
#ifndef RDM_SEND_H_
#define RDM_SEND_H_

#include <stdint.h>

extern void rdm_send_discovery_respond_message(const uint8_t *, const uint16_t);
extern void rdm_send_respond_message_ack(uint8_t *);
extern void rdm_send_respond_message_nack(uint8_t *, const uint16_t);
extern void rdm_send_respond_message_ack_timer(uint8_t *, const uint16_t);
extern void rdm_send_increment_message_count();
extern void rdm_send_decrement_message_count();

#endif /* RDM_SEND_H_ */
