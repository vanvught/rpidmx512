/*
 * host.h
 */

#ifndef DEVICE_USB_HOSTMSC_H_
#define DEVICE_USB_HOSTMSC_H_

namespace usb {
namespace host {

enum class Status {
	NOT_AVAILABLE,
	DISCONNECTED,
	RESET,
	SPEED_AVAILALBE,
	ATTACHED,
	CONNECTED,
	ENUMERATION_COMPLETED,
	READY,
	DEVICE_NOT_SUPPORTED,
	UNRECOVERABLE_ERROR
};

enum class Speed {
	FAULT,
	LOW,
	FULL,
	HIGH
};

enum class Class {
	NOT_SUPPORTED,
	MSC,
	HID
};

Status get_status();
Speed get_speed();
Class get_class();
}  // namespace host
}  // namespace usb

#endif /* DEVICE_USB_HOSTMSC_H_ */
