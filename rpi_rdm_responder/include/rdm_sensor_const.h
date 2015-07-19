
#ifndef RDM_SENSOR_CONST_H_
#define RDM_SENSOR_CONST_H_

#include <stdint.h>

#include "rdm_sensor.h"
#include "rdm_e120.h"
#include "hardware.h"

static const struct _rdm_sensor_defintion rdm_sensor_defintions[] = { {
		(uint8_t) 0,
		(uint8_t) E120_SENS_TEMPERATURE,
		(uint8_t) E120_UNITS_CENTIGRADE,
		(uint8_t) E120_PREFIX_NONE,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) RDM_SENSOR_TEMPERATURE_RANGE_MAX,
		(int16_t) RDM_SENSOR_TEMPERATURE_ABS_ZERO,
		(int16_t) 85,
		(uint8_t) RDM_SENSOR_RECORDED_SUPPORTED | RDM_SENSOR_LOW_HIGH_DETECT,
		"CPU", 3 } };

#define RDM_SENSORS_COUNT	(sizeof(rdm_sensor_defintions) / sizeof(rdm_sensor_defintions[0]))

struct _rdm_sensor_func {
	const int32_t (*f)(void);
} static const rdm_sensor_funcs[RDM_SENSORS_COUNT] = {{ hardware_get_core_temperature }};

static struct _rdm_sensor_value rdm_sensor_values[RDM_SENSORS_COUNT];

#endif /* INCLUDE_RDM_SENSOR_CONST_H_ */
