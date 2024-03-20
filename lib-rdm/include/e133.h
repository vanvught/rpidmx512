/**
 * @file e133.h
 *
 */
/* Copyright (C) 2019-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef E133_H_
#define E133_H_

/**
 * A.3 Root Layer PDU Vector
 */

#define VECTOR_ROOT_LLRP 			0x0000000A	/* Section 5.4 */

/**
 * Table A-4: Vector Defines for LLRP PDU
 */
#define VECTOR_LLRP_PROBE_REQUEST 0x00000001 /* Section 5.4.2.1 */
#define VECTOR_LLRP_PROBE_REPLY   0x00000002 /* Section 5.4.2.2 */
#define VECTOR_LLRP_RDM_CMD       0x00000003 /* Section 5.4.2.3 */

/**
 * Table A-5: Vector Defines for LLRP Probe Request PDU
 */
#define VECTOR_PROBE_REQUEST_DATA 0x01

/**
 * Table A-6: Vector Defines for LLRP Probe Reply PDU
 */
#define VECTOR_PROBE_REPLY_DATA 0x01

/**
 * Table A.12 RDM Command PDU Vector
 */
#define VECTOR_RDM_CMD_RDM_DATA	0xCC

/**
 * Table A.15 RDM Parameter ID
 */

#define E133_COMPONENT_SCOPE	0x0800
#define E133_SEARCH_DOMAIN		0x0801
#define E133_TCP_COMMS_STATUS	0x0802
#define E133_BROKER_STATUS		0x0803

/**
 * Table A-23: LLRP Component Type Codes
 */
#define LLRP_COMPONENT_TYPE_RPT_DEVICE     0x00	///< The LLRP Target is a Device
#define LLRP_COMPONENT_TYPE_RPT_CONTROLLER 0x01	///< The LLRP Target is a Controller
#define LLRP_COMPONENT_TYPE_BROKER         0x02	///< The LLRP Target is a Broker
#define LLRP_COMPONENT_TYPE_NON_RDMNET     0xFF	///< The LLRP Target does not implement any RDMnet protocol other than LLRP

#endif /* E133_H_ */
