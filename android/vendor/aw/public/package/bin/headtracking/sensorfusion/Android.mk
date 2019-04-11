LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)			# clean everything up to prepare for a module

# copy files to ./

dependingIncludes := \
					OVR_SensorFusion.h \
					OVR_SensorFilter.h \
					OVR_Math.h \
					OVR_Types.h \
					OVR_Log.h \
					OVR_Std.h \
					OVR_Deque.h \
					OVR_Allocator.h \
					OVR_Alg.h \
					OVR_Lockless.h \
					OVR_Atomic.h \
					AwVRSensorHal.h \
					OVR_Array.h \
					OVR_ContainerAllocator.h

LOCAL_MODULE    := libsensorfusion

LOCAL_SRC_FILES	:= OVR_Math.cpp \
				   OVR_SensorFilter.cpp \
	               OVR_SensorFusion.cpp \
				   OVR_Log.cpp \
				   OVR_Std.cpp \
				   OVR_Allocator.cpp \
				   OVR_Alg.cpp \
				   OVR_Lockless.cpp \
				   OVR_Atomic.cpp \
				   AwVRSensorHal.cpp

copyFiles := $(dependingIncludes)
copyFiles += $(LOCAL_SRC_FILES)

define copy-files-to-pwd
$(foreach file, $(1), $(shell find ../../ -name $(file) | xargs -i cp -f {} ./))
endef

#$(call copy-files-to-pwd, $(copyFiles))

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

LOCAL_CLANG := false

#LOCAL_LDLIBS := -llog -landroid
LOCAL_CFLAGS := -Wno-non-virtual-dtor
LOCAL_CPPFLAGS := -std=c++11

LOCAL_SHARED_LIBRARY := libutils liblog libandroid

#include $(BUILD_EXECUTABLE)
#include $(BUILD_SHARED_LIBRARY)		# start building based on everything since CLEAR_VARS
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/ndk_helper)
