
#include "rdmsoftwareversion.h"

#include "software_version.h"
#include "sofware_version_id.h"

const char *RDMSoftwareVersion::GetVersion(void) {
	return SOFTWARE_VERSION;
}

const uint8_t RDMSoftwareVersion::GetVersionLength(void) {
	return (uint8_t) sizeof(SOFTWARE_VERSION) / sizeof(SOFTWARE_VERSION[0]) - 1;
}

const uint32_t RDMSoftwareVersion::GetVersionId(void) {
	return DEVICE_SOFTWARE_VERSION_ID;
}
