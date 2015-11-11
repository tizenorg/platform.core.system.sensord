#define ACCEL strcmp("@ACCEL@", "ON")
#define AUTO_ROTATION strcmp("@AUTO_ROTATION@", "ON")
#define GYRO strcmp("@GYRO@", "ON")
#define PROXI strcmp("@PROXI@", "ON")
#define LIGHT strcmp("@LIGHT@", "ON")
#define GEO strcmp("@GEO@", "ON")
#define PRESSURE strcmp("@PRESSURE@", "ON")
#define TEMPERATURE strcmp("@TEMPERATURE@", "ON")
#define ULTRAVIOLET strcmp("@ULTRAVIOLET@", "ON")
#define ORIENTATION strcmp("@ORIENTATION@", "ON")
#define GRAVITY strcmp("@GRAVITY@", "ON")
#define LINEAR_ACCEL strcmp("@LINEAR_ACCEL@", "ON")
#define GEOMAGNETIC_RV strcmp("@GEOMAGNETIC_RV@", "ON")
#define GAMING_RV strcmp("@GAMING_RV@", "ON")
#define RV strcmp("@RV@", "ON")
#define TILT strcmp("@TILT@", "ON")
#define UNCAL_GYRO strcmp("@UNCAL_GYRO@", "ON")
#define BIO_LED_RED strcmp("@BIO_LED_RED@", "ON")
#define RV_RAW strcmp("@RV_RAW@", "ON")

#if $ACCEL == 1
#include <accel_sensor.h>
#endif
#if $GYRO == 1
#include <gyro_sensor.h>
#endif
#if $PROXI == 1
#include <proxi_sensor.h>
#endif
#if $LIGHT == 1
#include <light_sensor.h>
#endif
#if $GEO == 1
#include <geo_sensor.h>
#endif
#if $AUTO_ROTATION == 1
#include <auto_rotation_sensor.h>
#endif
#if $PRESSURE == 1
#include <pressure_sensor.h>
#endif
#if $TEMPERATURE == 1
#include <temperature_sensor.h>
#endif
#if $HUMIDITY == 1
#include <humidity_sensor.h>
#endif
#if $ULTRAVIOLET == 1
#include <ultraviolet_sensor.h>
#endif
#if $BIO_LED_RED == 1
#include <bio_led_red_sensor.h>
#endif
#if $ORIENTATION == 1
#include <orientation_sensor.h>
#endif
#if $GEOMAGNETIC_RV == 1
#include <geomagnetic_rv_sensor.h>
#endif
#if $GAMING_RV == 1
#include <gaming_rv_sensor.h>
#endif
#if $TILT == 1
#include <tilt_sensor.h>
#endif
#if $UNCAL_GYRO == 1
#include <uncal_gyro_sensor.h>
#endif
#if $GRAVITY == 1
#include <gravity_sensor.h>
#endif
#if $LINEAR_ACCEL == 1
#include <linear_accel_sensor.h>
#endif
#if $RV == 1
#include <rv_sensor.h>
#endif
#if $RV_RAW == 1
#include <rv_raw_sensor.h>
#endif

#include <sf_common.h>

extern "C" sensor_module* create(void)
{
	sensor_module *module = new(std::nothrow) sensor_module;
	retvm_if(!module, NULL, "Failed to allocate memory");

#if $ACCEL == 1
	accel_sensor *accel_sensor = NULL;
	try {
		accel_sensor = new(std::nothrow) accel_sensor;
	} catch (int err) {
		ERR("Failed to create accel_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!accel_sensor)
		module->sensors.push_back(accel_sensor);
#endif
#if $AUTO_ROTATION == 1
	auto_rotation_sensor *auto_rotation_sensor = NULL;
	try {
		auto_rotation_sensor = new(std::nothrow) auto_rotation_sensor;
	} catch (int err) {
		ERR("Failed to create auto_rotation_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!auto_rotation_sensor)
		module->sensors.push_back(auto_rotation_sensor);
#endif
#if $GYRO == 1
	gyro_sensor *gyro_sensor = NULL;
	try {
		gyro_sensor = new(std::nothrow) gyro_sensor;
	} catch (int err) {
		ERR("Failed to create gyro_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!gyro_sensor)
		module->sensors.push_back(gyro_sensor);
#endif
#if $PROXI == 1
	proxi_sensor *proxi_sensor = NULL;
	try {
		proxi_sensor = new(std::nothrow) proxi_sensor;
	} catch (int err) {
		ERR("Failed to create proxi_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!proxi_sensor)
		module->sensors.push_back(proxi_sensor);
#endif
#if $LIGHT == 1
	light_sensor *light_sensor = NULL;
	try {
		light_sensor = new(std::nothrow) light_sensor;
	} catch (int err) {
		ERR("Failed to create light_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!light_sensor)
		module->sensors.push_back(light_sensor);
#endif
#if $GEO == 1
	geo_sensor *geo_sensor = NULL;
	try {
		geo_sensor = new(std::nothrow) geo_sensor;
	} catch (int err) {
		ERR("Failed to create geo_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!geo_sensor)
		module->sensors.push_back(geo_sensor);
#endif
#if $PRESSURE == 1
	pressure_sensor *pressure_sensor = NULL;
	try {
		pressure_sensor = new(std::nothrow) pressure_sensor;
	} catch (int err) {
		ERR("Failed to create pressure_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!pressure_sensor)
		module->sensors.push_back(pressure_sensor);
#endif
#if $TEMPERATURE == 1

	temperature_sensor *temperature_sensor = NULL;
	try {
		temperature_sensor = new(std::nothrow) temperature_sensor;
	} catch (int err) {
		ERR("Failed to create temperature_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!temperature_sensor)
		module->sensors.push_back(temperature_sensor);

#endif
#if $ULTRAVIOLET == 1
	ultraviolet_sensor *ultraviolet_sensor = NULL;
	try {
		ultraviolet_sensor = new(std::nothrow) ultraviolet_sensor;
	} catch (int err) {
		ERR("Failed to create ultraviolet_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!ultraviolet_sensor)
		module->sensors.push_back(ultraviolet_sensor);
#endif
#if $ORIENTATION == 1
	orientation_sensor *orientation_sensor = NULL;
	try {
		orientation_sensor = new(std::nothrow) orientation_sensor;
	} catch (int err) {
		ERR("Failed to create orientation_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!orientation_sensor)
		module->sensors.push_back(orientation_sensor);
#endif
#if $GRAVITY == 1
	gravity_sensor *gravity_sensor = NULL;
	try {
		gravity_sensor = new(std::nothrow) gravity_sensor;
	} catch (int err) {
		ERR("Failed to create gravity_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!gravity_sensor)
		module->sensors.push_back(gravity_sensor);
#endif
#if $LINEAR_ACCEL == 1
	linear_accel_sensor *linear_accel_sensor = NULL;
	try {
		linear_accel_sensor = new(std::nothrow) linear_accel_sensor;
	} catch (int err) {
		ERR("Failed to create linear_accel_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!linear_accel_sensor)
		module->sensors.push_back(linear_accel_sensor);
#endif
#if $GEOMAGNETIC_RV == 1
	geomagnetic_rv_sensor *geomagnetic_rv_sensor = NULL;
	try {
		geomagnetic_rv_sensor = new(std::nothrow) geomagnetic_rv_sensor;
	} catch (int err) {
		ERR("Failed to create geomagnetic_rv_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!geomagnetic_rv_sensor)
		module->sensors.push_back(geomagnetic_rv_sensor);
#endif
#if $GAMING_RV == 1
	gaming_rv_sensor *gaming_rv_sensor = NULL;
	try {
		gaming_rv_sensor = new(std::nothrow) gaming_rv_sensor;
	} catch (int err) {
		ERR("Failed to create gaming_rv_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!gaming_rv_sensor)
		module->sensors.push_back(gaming_rv_sensor);
#endif
#if $RV == 1
	rv_sensor *rv_sensor = NULL;
	try {
		rv_sensor = new(std::nothrow) rv_sensor;
	} catch (int err) {
		ERR("Failed to create rv_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!rv_sensor)
		module->sensors.push_back(rv_sensor);
#endif
#if $TILT == 1
	tilt_sensor *tilt_sensor = NULL;
	try {
		tilt_sensor = new(std::nothrow) tilt_sensor;
	} catch (int err) {
		ERR("Failed to create tilt_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!tilt_sensor)
		module->sensors.push_back(tilt_sensor);
#endif
#if $UNCAL_GYRO == 1
	uncal_gyro_sensor *uncal_gyro_sensor = NULL;
	try {
		uncal_gyro_sensor = new(std::nothrow) uncal_gyro_sensor;
	} catch (int err) {
		ERR("Failed to create uncal_gyro_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!uncal_gyro_sensor)
		module->sensors.push_back(uncal_gyro_sensor);
#endif
#if $BIO_LED_RED == 1
	bio_led_red_sensor *bio_led_red_sensor = NULL;
	try {
		bio_led_red_sensor = new(std::nothrow) bio_led_red_sensor;
	} catch (int err) {
		ERR("Failed to create bio_led_red_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!bio_led_red_sensor)
		module->sensors.push_back(bio_led_red_sensor);
#endif
#if $RV_RAW == 1
	rv_raw_sensor *rv_raw_sensor = NULL;
	try {
		rv_raw_sensor = new(std::nothrow) rv_raw_sensor;
	} catch (int err) {
		ERR("Failed to create rv_raw_sensor module, err: %d, cause: %s", err, strerror(err));
	}
	if (!rv_raw_sensor)
		module->sensors.push_back(rv_raw_sensor);

#endif

	return module;
}
