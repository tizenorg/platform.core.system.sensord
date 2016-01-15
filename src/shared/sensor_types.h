#ifndef _SENSOR_TYPES_H_
#define _SENSOR_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum {
	ALL_SENSOR = -1,
	UNKNOWN_SENSOR = 0,
	ACCELEROMETER_SENSOR,
	GEOMAGNETIC_SENSOR,
	LIGHT_SENSOR,
	PROXIMITY_SENSOR,
	THERMOMETER_SENSOR,
	GYROSCOPE_SENSOR,
	PRESSURE_SENSOR,
	MOTION_SENSOR,
	FUSION_SENSOR,
	PEDOMETER_SENSOR,
	CONTEXT_SENSOR,
	FLAT_SENSOR,
	BIO_SENSOR,
	BIO_HRM_SENSOR,
	AUTO_ROTATION_SENSOR,
	GRAVITY_SENSOR,
	LINEAR_ACCEL_SENSOR,
	ROTATION_VECTOR_SENSOR,
	GEOMAGNETIC_RV_SENSOR,
	GAMING_RV_SENSOR,
	ORIENTATION_SENSOR,
	TILT_SENSOR,
	PIR_SENSOR,
	PIR_LONG_SENSOR,
	TEMPERATURE_SENSOR,
	HUMIDITY_SENSOR,
	ULTRAVIOLET_SENSOR,
	DUST_SENSOR,
	BIO_LED_GREEN_SENSOR,
	BIO_LED_IR_SENSOR,
	BIO_LED_RED_SENSOR,
	RV_RAW_SENSOR,
	GYROSCOPE_UNCAL_SENSOR,
	UNCAL_GEOMAGNETIC_SENSOR,
	WRIST_UP_SENSOR,
} sensor_type_t;

// Sensor Event Types
enum event_types_t {
	ACCELEROMETER_RAW_DATA_EVENT					= (ACCELEROMETER_SENSOR << 16) | 0x0001,
	ACCELEROMETER_UNPROCESSED_DATA_EVENT			= (ACCELEROMETER_SENSOR << 16) | 0x0002,

	GYROSCOPE_RAW_DATA_EVENT	= (GYROSCOPE_SENSOR << 16) | 0x0001,
	GYROSCOPE_UNPROCESSED_DATA_EVENT	= (GYROSCOPE_SENSOR << 16) | 0x0002,

	GEOMAGNETIC_RAW_DATA_EVENT	= (GEOMAGNETIC_SENSOR << 16) | 0x0001,
	GEOMAGNETIC_UNPROCESSED_DATA_EVENT	= (GEOMAGNETIC_SENSOR << 16) | 0x0002,

	PROXIMITY_CHANGE_STATE_EVENT	= (PROXIMITY_SENSOR << 16) | 0x0001,
	PROXIMITY_STATE_EVENT	= (PROXIMITY_SENSOR << 16) | 0x0002,
	PROXIMITY_DISTANCE_DATA_EVENT	= (PROXIMITY_SENSOR << 16) | 0x0003,

	PRESSURE_RAW_DATA_EVENT 	= (PRESSURE_SENSOR << 16) | 0x0001,

	TEMPERATURE_RAW_DATA_EVENT	= (TEMPERATURE_SENSOR << 16) | 0x0001,

	LIGHT_LUX_DATA_EVENT	= (LIGHT_SENSOR << 16) | 0x0001,
	LIGHT_LEVEL_DATA_EVENT	= (LIGHT_SENSOR << 16) | 0x0002,
	LIGHT_CHANGE_LEVEL_EVENT	= (LIGHT_SENSOR << 16) | 0x0003,

	ROTATION_VECTOR_RAW_DATA_EVENT	= (ROTATION_VECTOR_SENSOR << 16) | 0x0001,

	RV_RAW_RAW_DATA_EVENT	= (RV_RAW_SENSOR << 16) | 0x0001,

	ULTRAVIOLET_RAW_DATA_EVENT 	= (ULTRAVIOLET_SENSOR << 16) | 0x0001,

	AUTO_ROTATION_CHANGE_STATE_EVENT = (AUTO_ROTATION_SENSOR << 16) | 0x0001,

	BIO_LED_RED_RAW_DATA_EVENT	= (BIO_LED_RED_SENSOR << 16) | 0x0001,

	GAMING_RV_RAW_DATA_EVENT	= (GAMING_RV_SENSOR << 16) | 0x0001,

	GEOMAGNETIC_RV_RAW_DATA_EVENT	= (GEOMAGNETIC_RV_SENSOR << 16) | 0x0001,

	GRAVITY_RAW_DATA_EVENT	= (GRAVITY_SENSOR << 16) | 0x0001,

	LINEAR_ACCEL_RAW_DATA_EVENT	= (LINEAR_ACCEL_SENSOR << 16) | 0x0001,

	MOTION_ENGINE_EVENT_SNAP				= (MOTION_SENSOR << 16) | 0x0001,
	MOTION_ENGINE_EVENT_SHAKE				= (MOTION_SENSOR << 16) | 0x0002,
	MOTION_ENGINE_EVENT_DOUBLETAP			= (MOTION_SENSOR << 16) | 0x0004,
	MOTION_ENGINE_EVENT_PANNING				= (MOTION_SENSOR << 16) | 0x0008,
	MOTION_ENGINE_EVENT_TOP_TO_BOTTOM		= (MOTION_SENSOR << 16) | 0x0010,
	MOTION_ENGINE_EVENT_DIRECT_CALL			= (MOTION_SENSOR << 16) | 0x0020,
	MOTION_ENGINE_EVENT_TILT_TO_UNLOCK		= (MOTION_SENSOR << 16) | 0x0040,
	MOTION_ENGINE_EVENT_LOCK_EXECUTE_CAMERA = (MOTION_SENSOR << 16) | 0x0080,
	MOTION_ENGINE_EVENT_SMART_ALERT			= (MOTION_SENSOR << 16) | 0x0100,
	MOTION_ENGINE_EVENT_TILT				= (MOTION_SENSOR << 16) | 0x0200,
	MOTION_ENGINE_EVENT_PANNING_BROWSE		= (MOTION_SENSOR << 16) | 0x0400,
	MOTION_ENGINE_EVENT_NO_MOVE				= (MOTION_SENSOR << 16) | 0x0800,
	MOTION_ENGINE_EVENT_SHAKE_ALWAYS_ON     = (MOTION_SENSOR << 16) | 0x1000,
	MOTION_ENGINE_EVENT_SMART_RELAY         = (MOTION_SENSOR << 16) | 0x2000,

	ORIENTATION_RAW_DATA_EVENT	= (ORIENTATION_SENSOR << 16) | 0x0001,

	TILT_RAW_DATA_EVENT 	= (TILT_SENSOR << 16) | 0x0001,

	GYROSCOPE_UNCAL_RAW_DATA_EVENT	= (GYROSCOPE_UNCAL_SENSOR << 16) | 0x0001,

	FUSION_EVENT = (FUSION_SENSOR << 16) | 0x0001,
	FUSION_GYROSCOPE_UNCAL_EVENT = (FUSION_SENSOR << 16) | 0x0002,
	FUSION_CALIBRATION_NEEDED_EVENT = (FUSION_SENSOR << 16) | 0x0003,
	FUSION_ORIENTATION_ENABLED = (FUSION_SENSOR << 16) | 0x0004,
	FUSION_ROTATION_VECTOR_ENABLED = (FUSION_SENSOR << 16) | 0x0005,
	FUSION_GAMING_ROTATION_VECTOR_ENABLED = (FUSION_SENSOR << 16) | 0x0006,
	FUSION_GEOMAGNETIC_ROTATION_VECTOR_ENABLED = (FUSION_SENSOR << 16) | 0x0007,
	FUSION_TILT_ENABLED = (FUSION_SENSOR << 16) | 0x0008,
	FUSION_GYROSCOPE_UNCAL_ENABLED = (FUSION_SENSOR << 16) | 0x0009,

	CONTEXT_EVENT_REPORT = (CONTEXT_SENSOR << 16) | 0x0001,
	CONTEXT_WRIST_UP_EVENT_REPORT = (WRIST_UP_SENSOR << 16) | 0x0001,
};

enum proxi_change_state {
	PROXIMITY_STATE_NEAR	= 0,
	PROXIMITY_STATE_FAR	= 1,
};

enum auto_rotation_state {
	AUTO_ROTATION_DEGREE_UNKNOWN = 0,
	AUTO_ROTATION_DEGREE_0,
	AUTO_ROTATION_DEGREE_90,
	AUTO_ROTATION_DEGREE_180,
	AUTO_ROTATION_DEGREE_270,
};

enum motion_snap_event {
	MOTION_ENGIEN_SNAP_NONE			= 0,
	MOTION_ENGIEN_NEGATIVE_SNAP_X	= 1,
	MOTION_ENGIEN_POSITIVE_SNAP_X	= 2,
	MOTION_ENGIEN_NEGATIVE_SNAP_Y	= 3,
	MOTION_ENGIEN_POSITIVE_SNAP_Y	= 4,
	MOTION_ENGIEN_NEGATIVE_SNAP_Z	= 5,
	MOTION_ENGIEN_POSITIVE_SNAP_Z	= 6,
	MOTION_ENGIEN_SNAP_LEFT			= MOTION_ENGIEN_NEGATIVE_SNAP_X,
	MOTION_ENGIEN_SNAP_RIGHT		= MOTION_ENGIEN_POSITIVE_SNAP_X,
	MOTION_ENGINE_SNAP_NONE			= 0,
	MOTION_ENGINE_NEGATIVE_SNAP_X	= 1,
	MOTION_ENGINE_POSITIVE_SNAP_X	= 2,
	MOTION_ENGINE_NEGATIVE_SNAP_Y	= 3,
	MOTION_ENGINE_POSITIVE_SNAP_Y	= 4,
	MOTION_ENGINE_NEGATIVE_SNAP_Z	= 5,
	MOTION_ENGINE_POSITIVE_SNAP_Z	= 6,
	MOTION_ENGINE_SNAP_LEFT			= MOTION_ENGINE_NEGATIVE_SNAP_X,
	MOTION_ENGINE_SNAP_RIGHT		= MOTION_ENGINE_POSITIVE_SNAP_X,
};

enum motion_shake_event {
	MOTION_ENGIEN_SHAKE_NONE		= 0,
	MOTION_ENGIEN_SHAKE_DETECTION	= 1,
	MOTION_ENGIEN_SHAKE_CONTINUING	= 2,
	MOTION_ENGIEN_SHAKE_FINISH		= 3,
	MOTION_ENGINE_SHAKE_BREAK		= 4,
	MOTION_ENGINE_SHAKE_NONE		= 0,
	MOTION_ENGINE_SHAKE_DETECTION	= 1,
	MOTION_ENGINE_SHAKE_CONTINUING	= 2,
	MOTION_ENGINE_SHAKE_FINISH		= 3,
};

enum motion_doubletap_event {
	MOTION_ENGIEN_DOUBLTAP_NONE			= 0,
	MOTION_ENGIEN_DOUBLTAP_DETECTION	= 1,
	MOTION_ENGINE_DOUBLTAP_NONE			= 0,
	MOTION_ENGINE_DOUBLTAP_DETECTION	= 1,
};

enum motion_top_to_bottom_event {
	MOTION_ENGIEN_TOP_TO_BOTTOM_NONE		= 0,
	MOTION_ENGIEN_TOP_TO_BOTTOM_WAIT		= 1,
	MOTION_ENGIEN_TOP_TO_BOTTOM_DETECTION	= 2,
	MOTION_ENGINE_TOP_TO_BOTTOM_NONE		= 0,
	MOTION_ENGINE_TOP_TO_BOTTOM_WAIT		= 1,
	MOTION_ENGINE_TOP_TO_BOTTOM_DETECTION	= 2,
};

enum motion_direct_call_event_t {
	MOTION_ENGINE_DIRECT_CALL_NONE,
	MOTION_ENGINE_DIRECT_CALL_DETECTION,
};

enum motion_smart_relay_event_t {
	MOTION_ENGINE_SMART_RELAY_NONE,
	MOTION_ENGINE_SMART_RELAY_DETECTION,
};

enum motion_tilt_to_unlock_event_t {
	MOTION_ENGINE_TILT_TO_UNLOCK_NONE,
	MOTION_ENGINE_TILT_TO_UNLOCK_DETECTION,
};

enum motion_lock_execute_camera_event_t {
	MOTION_ENGINE_LOCK_EXECUTE_CAMERA_NONE,
	MOTION_ENGINE_LOCK_EXECUTE_CAMERA_L_DETECTION,
	MOTION_ENGINE_LOCK_EXECUTE_CAMERA_R_DETECTION,
};

enum motion_smart_alert_t {
	MOTION_ENGINE_SMART_ALERT_NONE,
	MOTION_ENGINE_SMART_ALERT_DETECTION,
};

enum motion_no_move_t {
	MOTION_ENGINE_NO_MOVE_NONE,
    MOTION_ENGINE_NO_MOVE_DETECTION,
};

enum motion_property_id {
	MOTION_PROPERTY_UNKNOWN = 0,
	MOTION_PROPERTY_CHECK_ACCEL_SENSOR,
	MOTION_PROPERTY_CHECK_GYRO_SENSOR,
	MOTION_PROPERTY_CHECK_GEO_SENSOR,
	MOTION_PROPERTY_CHECK_PRIXI_SENSOR,
	MOTION_PROPERTY_CHECK_LIGHT_SENSOR,
	MOTION_PROPERTY_CHECK_BARO_SENSOR,
	MOTION_PROPERTY_LCD_TOUCH_ON,
	MOTION_PROPERTY_LCD_TOUCH_OFF,
	MOTION_PROPERTY_CHECK_GYRO_CAL_STATUS,
};

/**
 * @defgroup SENSOR_PEDO Pedometer Sensor
 * @ingroup SENSOR_FRAMEWORK
 *
 * These APIs are used to control the Pedometer sensor.
 * @{
 */

enum pedo_data_id {
	PEDOMETER_BASE_DATA_SET	= (PEDOMETER_SENSOR << 16) | 0x0001,
};

enum pedo_event_type {
	PEDOMETER_EVENT_STEP_COUNT	= (PEDOMETER_SENSOR << 16) | 0x0001,
};

enum pedo_property_id {
	PEDOMETER_PROPERTY_UNKNOWN	= 0,
};

enum pedo_status {
	PEDOMETER_STATUS_UNKNOWN = 0,
	PEDOMETER_STATUS_STOP,
	PEDOMETER_STATUS_WALK,
	PEDOMETER_STATUS_RUN,
};

#define SENSOR_PEDOMETER_DATA_DIFFS_SIZE	20

/**
 * sensor_data_t extension for Pedometer sensor
 * The internal structure should be aligned with sensor_data_t.
 */
typedef struct {
	int accuracy;
	unsigned long long timestamp;
	/* In case of Pedometer, value_count = 8 */
	int value_count;
	/* In case of Pedometer, values = {
	 *		step count,
	 *		walk step count,
	 *		run step count,
	 *		moving distance,
	 *		calorie burned,
	 *		last speed
	 *		last stepping frequency (steps per sec)
	 *		last step status (walking, running, ...),
	 * }
	 */
	float values[SENSOR_DATA_VALUE_SIZE];
	/* Additional data attributes (not in sensor_data_t)*/
	int diffs_count;
	struct differences {
		int timestamp;
		int total_step;
		int walk_step;
		int run_step;
		float distance;
		float calorie;
		float speed;
	} diffs[SENSOR_PEDOMETER_DATA_DIFFS_SIZE];
} pedo_data_t;

/**
 * sensor_event_t extension for Pedometer sensor.
 * The internal structure should be aligned with sensor_event_t.
 */
typedef struct {
	unsigned int event_type;
	sensor_id_t sensor_id;
	pedo_data_t data;
} pedo_event_t;

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* _SENSOR_TYPES_H_ */
