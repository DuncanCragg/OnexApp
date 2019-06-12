
LOCAL_PATH := $(call my-dir)/../../../..

######################################################
include $(CLEAR_VARS)

LOCAL_MODULE := OnexAndroidKernel

LOCAL_SRC_FILES := ../OnexKernel/libOnexAndroidKernel.a

include $(PREBUILT_STATIC_LIBRARY)

######################################################
include $(CLEAR_VARS)

LOCAL_MODULE := OnexAndroidLang

LOCAL_SRC_FILES := ../OnexLang/libOnexAndroidLang.a

include $(PREBUILT_STATIC_LIBRARY)

######################################################
include $(CLEAR_VARS)

LOCAL_MODULE := onexapp

LOCAL_C_INCLUDES :=
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/glm
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../external/imgui
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../OnexKernel/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../OnexLang/include

LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/../sascha/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/../src/*.cpp) \
                   $(wildcard $(LOCAL_PATH)/../external/imgui/*.cpp) \

LOCAL_CPPFLAGS := -std=c++11
LOCAL_CPPFLAGS += -D__STDC_LIMIT_MACROS
LOCAL_CPPFLAGS += -DVK_NO_PROTOTYPES
LOCAL_CPPFLAGS += -DVK_USE_PLATFORM_ANDROID_KHR

LOCAL_LDLIBS := -landroid -llog -lz

LOCAL_STATIC_LIBRARIES += android_native_app_glue
LOCAL_STATIC_LIBRARIES += cpufeatures
LOCAL_STATIC_LIBRARIES += OnexAndroidKernel
LOCAL_STATIC_LIBRARIES += OnexAndroidLang

LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, android/cpufeatures)
