#-------------------------------------------------------------------
# This file has been automatically generated by ProjectGenerator    
# which is part of a build system designed for GUCEF                
#     (Galaxy Unlimited Framework)                                  
# For the latest info, see http://www.VanvelzenSoftware.com/        
#                                                                   
# The contents of this file are placed in the public domain. Feel   
# free to make use of it in any way you like.                       
#-------------------------------------------------------------------


ifndef MY_MODULE_PATH
  MY_MODULE_PATH := $(call my-dir)
endif
LOCAL_PATH := $(MY_MODULE_PATH)

include $(CLEAR_VARS)

@echo Module path: $(MY_MODULE_PATH)
LOCAL_MODULE := CEGUI.RendererModule.OpenGLES
LOCAL_MODULE_FILENAME := libCEGUI.RendererModule.OpenGLES
@echo Module name: $(LOCAL_MODULE)

LOCAL_SRC_FILES := \
  FBOTextureTarget.cpp \
  GeometryBuffer.cpp \
  Renderer.cpp \
  Texture.cpp \
  ViewportTarget.cpp

LOCAL_C_INCLUDES := \
  $(MY_MODULE_PATH)/../../../include

LOCAL_CFLAGS := -DCEGUIOPENGLESRENDERER_EXPORTS


LOCAL_SHARED_LIBRARIES := \
  CEGUI

include $(BUILD_SHARED_LIBRARY)

