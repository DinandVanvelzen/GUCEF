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
LOCAL_MODULE := gucefCOMCORE
LOCAL_MODULE_FILENAME := libgucefCOMCORE
@echo Module name: $(LOCAL_MODULE)

LOCAL_SRC_FILES := \
  src/CCom.cpp \
  src/CGUCEFCOMCOREModule.cpp \
  src/CHostAddress.cpp \
  src/CPing.cpp \
  src/CSocket.cpp \
  src/CTCPClientSocket.cpp \
  src/CTCPConnection.cpp \
  src/CTCPServerConnection.cpp \
  src/CTCPServerSocket.cpp \
  src/CUDPChannel.cpp \
  src/CUDPMasterSocket.cpp \
  src/CUDPSocket.cpp \
  src/dvwinsock.c \
  src/gucefCOMCORE.cpp \
  src/gucefCOMCORE_CComCoreGlobal.cpp \
  src/gucefCOMCORE_CDiscoveryManager.cpp \
  src/gucefCOMCORE_CDnsCache.cpp \
  src/gucefCOMCORE_CDnsCacheEntry.cpp \
  src/gucefCOMCORE_CDnsCacheRefreshTaskConsumer.cpp \
  src/gucefCOMCORE_CDnsResolver.cpp \
  src/gucefCOMCORE_CGeoLocationLookupService.cpp \
  src/gucefCOMCORE_CICommunicationInterface.cpp \
  src/gucefCOMCORE_CIDiscovery.cpp \
  src/gucefCOMCORE_CIDiscoveryCapability.cpp \
  src/gucefCOMCORE_CIDiscoveryClient.cpp \
  src/gucefCOMCORE_CIDiscoveryEntry.cpp \
  src/gucefCOMCORE_CIGeoLocationLookupServiceProvider.cpp \
  src/gucefCOMCORE_CIMessage.cpp \
  src/gucefCOMCORE_CIMessageTransformer.cpp \
  src/gucefCOMCORE_CINetworkInterface.cpp \
  src/gucefCOMCORE_CIPv4Address.cpp \
  src/gucefCOMCORE_CIPv4Info.cpp \
  src/gucefCOMCORE_CIPv6Address.cpp \
  src/gucefCOMCORE_CIPv6Info.cpp \
  src/gucefCOMCORE_CMessageSerializer.cpp \
  src/gucefCOMCORE_CPingTaskConsumer.cpp \
  src/gucefCOMCORE_CSerialPort.cpp \
  src/socketutils.cpp

LOCAL_C_INCLUDES := \
  $(MY_MODULE_PATH)/include \
  $(MY_MODULE_PATH)/../../common/include \
  $(MY_MODULE_PATH)/../gucefCORE/include \
  $(MY_MODULE_PATH)/../gucefCORE/include/android \
  $(MY_MODULE_PATH)/../gucefCORE/include/c_api \
  $(MY_MODULE_PATH)/../gucefMT/include

LOCAL_CFLAGS := -DGUCEF_COMCORE_BUILD_MODULE


LOCAL_SHARED_LIBRARIES := \
  gucefCORE \
  gucefMT

include $(BUILD_SHARED_LIBRARY)

