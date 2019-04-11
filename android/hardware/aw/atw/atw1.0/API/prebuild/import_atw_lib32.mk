#
# import_atw.mk
#
# Common settings used by all native projects who want use allwinner atw module.
#
# See the NDK documentation for more details.

# save off the local path
LOCAL_PATH_TEMP := $(LOCAL_PATH)
LOCAL_PATH := $(call my-dir)

# predefine atw module.
include $(CLEAR_VARS)
LOCAL_MODULE := atw_api
LOCAL_SRC_FILES := lib32/libatw_api.so
TARGET_PRELINK_MODULES := false
include $(PREBUILT_SHARED_LIBRARY)

# for your customer lib, use libatwclient like this:
#include $(CLEAR_VARS)
#LOCAL_MODULE := YOUR_MODULE_NAME
#LOCAL_C_INCLUDES  += API/inc/
#LOCAL_EXPORT_C_INCLUDES += API/inc/
#LOCAL_SHARED_LIBRARIES += libatw_api
#include $(BUILD_SHARED_LIBRARY)

# Restore the local path since we overwrote it when we started
LOCAL_PATH := $(LOCAL_PATH_TEMP)
