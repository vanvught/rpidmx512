/*
 * rdm_device_const.h
 *
 *  Created on: Mar 15, 2015
 *      Author: pi
 */

#ifndef RDM_DEVICE_CONST_H_
#define RDM_DEVICE_CONST_H_

#if RDM_DEVICE_INFO_H_
# error Invalid include
#endif

static const uint8_t  DEVICE_LABEL[] = "Raspberry Pi DMX USB Pro";
static const uint8_t  DEVICE_ID[] = {1, 0};
static const uint8_t  DEVICE_MANUFACTURER_NAME[] = "AvV";
static const uint8_t  DEVICE_MANUFACTURER_ID[] = {0xF0, 0x7F};
static const uint8_t  DEVICE_SUPPORTED_LANGUAGE[] = "en";
static const uint8_t  DEVICE_SOFTWARE_VERSION[] = "1.0";
static const uint32_t DEVICE_SOFTWARE_VERSION_ID = 0x20150315;

#endif /* RDM_DEVICE_CONST_H_ */
