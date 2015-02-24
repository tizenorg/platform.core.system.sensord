
#ifndef __SENSOR_GAMING_RV_H__
#define __SENSOR_GAMING_RV_H__

//! Pre-defined events for the gaming rotation vector sensor
//! Sensor Plugin developer can add more event to their own headers

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @defgroup SENSOR_GEOMAGNETIC_RV Rotation Vector Sensor
 * @ingroup SENSOR_FRAMEWORK
 *
 * These APIs are used to control the Gaming Rotation Vector sensor.
 * @{
 */

enum gaming_rv_event_type {
	GAMING_RV_RAW_DATA_EVENT	= (GAMING_RV_SENSOR << 16) | 0x0001,
};

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
//! End of a file

