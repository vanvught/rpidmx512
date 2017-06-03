/*
 * artnet_output.h
 *
 *  Created on: May 31, 2017
 *      Author: pi
 */

#ifndef ARTNET_OUTPUT_H_
#define ARTNET_OUTPUT_H_

#include "artnetnode.h"

#include "midi.h"
#include "ltc_reader.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void artnet_output_set_node(const ArtNetNode *);
extern void artnet_output(const struct _midi_send_tc *, const timecode_types);

#ifdef __cplusplus
}
#endif

#endif /* ARTNET_OUTPUT_H_ */
