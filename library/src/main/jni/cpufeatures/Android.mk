LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
include $(CLEAR_ABI)

LOCAL_MODULE := cpufeatures
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_SRC_FILES := cpu-features.c

include $(APPLY_ABI)
include $(BUILD_STATIC_LIBRARY)
