/**
 * @file e120.h
 *
 */
 /* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef E120_H_
#define E120_H_

#include <cstdint>

#include "rdmconst.h"

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

struct TRdmMessage
{
    uint8_t start_code;                    ///< 1	SC_RDM
    uint8_t sub_start_code;                ///< 2	SC_SUB_MESSAGE
    uint8_t message_length;                ///< 3	Range 24 to 255
    uint8_t destination_uid[RDM_UID_SIZE]; ///< 4,5,6,7,8,9
    uint8_t source_uid[RDM_UID_SIZE];      ///< 10,11,12,13,14,15
    uint8_t transaction_number;            ///< 16
    union
    {
        uint8_t port_id;       ///< 17
        uint8_t response_type; ///< 17
    } slot16;
    uint8_t message_count;     ///< 18
    uint8_t sub_device[2];     ///< 19, 20
    uint8_t command_class;     ///< 21
    uint8_t param_id[2];       ///< 22, 23
    uint8_t param_data_length; ///< 24	PDL	Range 0 to 231
    uint8_t param_data[231];   ///< 25,,,,	PD	6.2.3 Message Length
    uint8_t checksum_filler[2];
} PACKED;

struct TRdmMessageNoSc
{
    uint8_t sub_start_code;                ///< 2	SC_SUB_MESSAGE
    uint8_t message_length;                ///< 3	Range 24 to 255
    uint8_t destination_uid[RDM_UID_SIZE]; ///< 4,5,6,7,8,9
    uint8_t source_uid[RDM_UID_SIZE];      ///< 10,11,12,13,14,15
    uint8_t transaction_number;            ///< 16
    union
    {
        uint8_t port_id;       ///< 17
        uint8_t response_type; ///< 17
    } slot16;
    uint8_t message_count;     ///< 18
    uint8_t sub_device[2];     ///< 19, 20
    uint8_t command_class;     ///< 21
    uint8_t param_id[2];       ///< 22, 23
    uint8_t param_data_length; ///< 24	PDL	Range 0 to 231
    uint8_t param_data[231];   ///< 25,,,,	PD	6.2.3 Message Length
    uint8_t checksum_filler[2];
} PACKED;

struct TRdmDiscoveryMsg
{
    uint8_t header_FE[7];         ///<
    uint8_t header_AA;            ///<
    uint8_t masked_device_id[12]; ///<
    uint8_t checksum[4];          ///<
} PACKED;

#endif  // E120_H_
