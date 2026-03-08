LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ksu_susfs
LOCAL_C_INCLUDES += $(LOCAL_PATH)/includes
ALL_C_FILES := $(wildcard $(LOCAL_PATH)/*.c)
ALL_C_FILES += $(wildcard $(LOCAL_PATH)/features/*.c)
LOCAL_SRC_FILES := $(ALL_C_FILES:$(LOCAL_PATH)/%=%)
LOCAL_CFLAGS    := -O3 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIC -flto=thin -ffunction-sections -fdata-sections -fvisibility=hidden
LOCAL_LDFLAGS   := -Wl,--gc-sections -flto=thin
LOCAL_STRIP     := true

include $(BUILD_EXECUTABLE)
