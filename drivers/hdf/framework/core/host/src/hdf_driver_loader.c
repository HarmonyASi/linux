/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_driver_loader.h"
#include "hdf_driver.h"
#include "hdf_core_log.h"
#include "hdf_object_manager.h"

#define HDF_LOG_TAG driver_loader

extern struct HdfDriverEntry *_hdf_drivers_start[];
extern struct HdfDriverEntry *_hdf_drivers_end[];

int32_t HdfDriverEntryConstruct(void)
{
    struct HdfDriverEntry **driverPtr = NULL;
    int32_t count = 0;

    for (driverPtr = _hdf_drivers_start; driverPtr < _hdf_drivers_end; driverPtr++) {
        struct HdfDriverEntry *driverEntry = *driverPtr;
        if (HdfRegisterDriverEntry(driverEntry) != HDF_SUCCESS) {
            HDF_LOGE("failed to register driver %{public}s",
                driverEntry ? driverEntry->moduleName : "");
            continue;
        }
        count++;
    }
    if (count <= 0) {
        HDF_LOGE("%{public}s: no hdf driver exist", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

struct HdfDriver *HdfDriverLoaderGetDriver(const char *moduleName)
{
    if (moduleName == NULL) {
        HDF_LOGE("%{public}s: failed to get device entry, moduleName is NULL", __func__);
        return NULL;
    }

    return HdfDriverManagerGetDriver(moduleName);
}

void HdfDriverLoaderReclaimDriver(struct HdfDriver *driver)
{
    // kernel driver do not need release
    (void)driver;
}

void HdfDriverLoaderConstruct(struct HdfDriverLoader *inst)
{
    if (inst != NULL) {
        inst->super.GetDriver = HdfDriverLoaderGetDriver;
        inst->super.ReclaimDriver = HdfDriverLoaderReclaimDriver;
    }
}

struct HdfObject *HdfDriverLoaderCreate(void)
{
    static bool isDriverLoaderInit = false;
    static struct HdfDriverLoader driverLoader;
    if (!isDriverLoaderInit) {
        if (HdfDriverEntryConstruct() != HDF_SUCCESS) {
            return NULL;
        }
        HdfDriverLoaderConstruct(&driverLoader);
        isDriverLoaderInit = true;
    }
    return (struct HdfObject *)&driverLoader;
}

struct IDriverLoader *HdfDriverLoaderGetInstance(void)
{
    static struct IDriverLoader *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDriverLoader *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DRIVER_LOADER);
    }
    return instance;
}
