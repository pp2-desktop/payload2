LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)

LOCAL_MODULE := cocos2dcpp_shared

LOCAL_MODULE_FILENAME := libcocos2dcpp

LOCAL_SRC_FILES := hellocpp/main.cpp \
                   ../../Classes/AppDelegate.cpp \
                   ../../Classes/json11.cpp \
                   ../../Classes/connection.cpp \
                   ../../Classes/user_info.cpp \
                   ../../Classes/assets_scene.cpp \
                   ../../Classes/lobby_scene.cpp \
                   ../../Classes/single_lobby_scene.cpp \
                   ../../Classes/single_play_info.cpp \
                   ../../Classes/single_play_scene.cpp \
                   ../../Classes/resource_md.cpp \
                   ../../Classes/multi_lobby_scene.cpp \
                   ../../Classes/multi_room_scene.cpp \
                   ../../Classes/multi_play_scene.cpp \
                   ../../Classes/ranking_scene.cpp \
                   ../../Classes/setting_scene.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Classes

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END


LOCAL_STATIC_LIBRARIES := cocos2dx_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
