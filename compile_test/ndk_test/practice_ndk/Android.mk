LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SRC_PATH := $(LOCAL_PATH)/
#$(warning $(SRC_PATH))

ALL_FILES := $(shell find $(SRC_PATH))
#$(warning $(ALL_FILES))

#ALL_FILES := $(ALL_FILES:$(SRC_PATH)/%=$(SRC_PATH)%)
#$(warning $(ALL_FILES))

SRC_FILES := $(filter %.cpp %.c,$(ALL_FILES))
$(warning $(SRC_FILES))

LOCAL_SRC_FILES += $(SRC_FILES:$(LOCAL_PATH)/%=%)
$(warning $(LOCAL_SRC_FILES))

LOCAL_MODULE := test
include $(BUILD_SHARED_LIBRARY)
