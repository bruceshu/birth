LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := test.c
LOCAL_ARM_MODE := arm
LOCAL_SHARED_LIBRARIES := libcutils_prebuilt
LOCAL_MODULE := test

include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)
LOCAL_MODULE := libcutils_prebuilt
LOCAL_SRC_FILES := ./libcutils.so
include $(PREBUILT_SHARED_LIBRARY)
