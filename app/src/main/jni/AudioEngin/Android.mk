#cmake_minimum_required(VERSION 2.8)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ( $(TARGET_ARCH_ABI),armeabi)
LOCAL_CFLAGS := -DHAVE_PTHREADS -mfpu=neon  -mfloat-abi=softfp -DANDROID
else ifeq ( $(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS := -DHAVE_PTHREADS -mfpu=neon  -mfloat-abi=softfp -DANDROID
else
LOCAL_CFLAGS := -DHAVE_PTHREADS -DANDROID

endif

LOCAL_CXXFLAGS := -DHAVE_PTHREADS

LOCAL_C_INCLUDES += \
					-I ./ \
					-I $(LOCAL_PATH)/../include/ \
					-I $(LOCAL_PATH)/../include/opus \
					-I $(LOCAL_PATH)/../include/XNet \
					-I $(LOCAL_PATH)/../include/webrtc/voice_engine/include \
					-I $(LOCAL_PATH)/../include/nativehelper \
					-I $(LOCAL_PATH)/util \
					-I $(LOCAL_PATH)/src_base \
					-I $(LOCAL_PATH)/class \
					
MAIN_FILE_LIST  := \
					$(wildcard $(LOCAL_PATH)/*.cpp) \
					$(wildcard $(LOCAL_PATH)/util/*.cpp) \
					$(wildcard $(LOCAL_PATH)/util/*.c) \
					$(wildcard $(LOCAL_PATH)/src_base/*.cpp) \
					$(wildcard $(LOCAL_PATH)/class/*.cpp)
	
LOCAL_SRC_FILES := \
					 $(MAIN_FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_LDLIBS :=	-Llib/		
LOCAL_LDLIBS += -llog -landroid -ldl -lOpenSLES -lm -ldl  -lz

LOCAL_LDFLAGS += $(LOCAL_PATH)/../libs/$(TARGET_ARCH_ABI)/libopus.so
LOCAL_LDFLAGS += $(LOCAL_PATH)/../libs/$(TARGET_ARCH_ABI)/libAudioCapPlay.so
LOCAL_LDFLAGS += $(LOCAL_PATH)/../libs/$(TARGET_ARCH_ABI)/libXNet.so
LOCAL_MODULE := MultvoiceEngine

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
