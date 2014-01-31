#include <bcm2835.h>

struct vc_msg_tag_clock_rate {
	uint32_t tag_id; /* the message id */
	uint32_t buffer_size; /* size of the buffer (which in this case is always 8 bytes) */
	uint32_t data_size; /* amount of data being sent or received */
	uint32_t dev_id; /* the ID of the clock/voltage to get or set */
	uint32_t val; /* the value (e.g. rate (in Hz)) to set */
};

struct vc_msg_get_clock_rate {
	uint32_t msg_size; /* simply, sizeof(struct vc_msg) */
	uint32_t request_code; /* holds various information like the success and number of bytes returned (refer to mailboxes wiki) */
	struct vc_msg_tag_clock_rate tag; /* the tag structure above to make */
	uint32_t end_tag; /* an end identifier, should be set to NULL */
};

uint32_t bcm2835_vc_get_clock_rate(int clock_id) {

	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	struct  vc_msg_get_clock_rate *vc_msg = (struct vc_msg_get_clock_rate *)mb_addr;

	vc_msg->msg_size = sizeof(struct vc_msg_get_clock_rate);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = BCM2835_MAILBOX_TAG_GET_CLOCK_RATE;
	vc_msg->tag.buffer_size = 8;
	vc_msg->tag.data_size = 4; /* we're just sending the clock ID which is one word long */
	vc_msg->tag.dev_id = clock_id;
	vc_msg->tag.val = 0;
	vc_msg->end_tag = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, mb_addr);
	bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return 0;
	}

	return vc_msg->tag.val;

}

uint32_t bcm2835_vc_set_clock_rate(int clock_id, uint32_t clock_rate) {
	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	struct  vc_msg_get_clock_rate *vc_msg = (struct vc_msg_get_clock_rate *)mb_addr;

	vc_msg->msg_size = sizeof(struct vc_msg_get_clock_rate);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = BCM2835_MAILBOX_TAG_SET_CLOCK_RATE;
	vc_msg->tag.buffer_size = 8;
	vc_msg->tag.data_size = 8; /* we're sending the clock ID and the new rates which is a total of 2 words */
	vc_msg->tag.dev_id = clock_id;
	vc_msg->tag.val = clock_rate;
	vc_msg->end_tag = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, mb_addr);
	bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return 0;
	}

	return vc_msg->tag.val;
}
