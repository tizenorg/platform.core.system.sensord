/*------------------------------------------------------------------------------
 * Copyright Sensirion AG 2015

 * The copyright to the computer program(s) herein is the property of
 * Sensirion AG, Switzerland. The program(s) may be used and/or copied only
 * with the written permission of Sensirion AG or in accordance with the terms
 * and conditions stipulated in the agreement/contract under which the
 * program(s) have been supplied.
------------------------------------------------------------------------------*/

#ifndef _ENGINEINTERFACE_H_
#define _ENGINEINTERFACE_H_

#include <stdint.h>



class EngineInterface {
public:
    /**
     * @return the version of the loaded engine
     */
    virtual const char* getEngineVersion() const = 0;

    /**
     * @return the build number of the loaded engine
     */
    virtual int getEngineBuild() const = 0;

    /**
     * Compensate temperature and relative humidity
     * 
     * @param[in] Time_Device timestamp of the sensor data in nanoseconds
     * @param[in] T_SHT Sensor Temperature
     * @param[out] T_AMB Ambient Temperature
     * @return Zero if compensation was successful,
     *         nonzero error code otherwise.
     */
    virtual int compensate(int64_t Time_Device, double T_SHT, double* T_AMB) = 0;

protected:
    virtual ~EngineInterface() {};
};

#define SENSIRION_COMP_ENGINE_LIBRARY "libsensirion_tizen.so"
#define SENSIRION_COMP_ENGINE_INTERFACE_VERSION "tizen-0"

#define FP_GET_ENGINE_INTERFACE_VERSION_NAME "getEngineInterfaceVersion"
#define FP_GET_ENGINE_INTERFACE_VERSION_FUNC  getEngineInterfaceVersion  // internal use only
typedef const char* (*FP_GET_ENGINE_INTERFACE_VERSION)();

#define FP_GET_ENGINE_LIBRARY_VERSION_NAME "getEngineLibraryVersion"
#define FP_GET_ENGINE_LIBRARY_VERSION_FUNC  getEngineLibraryVersion  // internal use only
typedef const char* (*FP_GET_ENGINE_LIBRARY_VERSION)();

#define FP_LOAD_COMP_ENGINE_NAME "loadCompEngine"
#define FP_LOAD_COMP_ENGINE_FUNC  loadCompEngine  // internal use only
typedef EngineInterface* (*FP_LOAD_COMP_ENGINE)();

#endif /* _ENGINEINTERFACE_H_ */
