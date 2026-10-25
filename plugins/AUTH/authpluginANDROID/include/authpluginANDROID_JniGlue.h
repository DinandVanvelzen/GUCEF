/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H
#define GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#include "authpluginANDROID_macros.h"
#define GUCEF_AUTHPLUGIN_ANDROID_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The JNI side of the plugin exists on Android only (GUCEF_PLATFORM_ANDROID); elsewhere these do nothing.
 *
 *  The Java class com.vanvelzensoftware.gucefauth.GucefAuthBridge loads the library and calls nativeInit() from its static
 *  initializer; that fills the table of functions of CAndroidAuthBridge with functions that call the static methods of that
 *  class, and from then on the providers are available. The Java side answers through nativeOnSignInResult() and hands
 *  redirects to onAuthRedirect(). The JNI names are:
 *    Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_nativeInit
 *    Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_nativeOnSignInResult
 *    Java_com_vanvelzensoftware_gucefauth_GucefAuthBridge_onAuthRedirect
 */

/** Whether the Java side did initialize */
GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP bool
IsJavaSideInitialized( void );

/** Takes the Java side out of the bridge and drops the references to it. The plugin does this when it unloads */
GUCEF_AUTHPLUGIN_ANDROID_EXPORT_CPP void
ReleaseJavaSide( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_ANDROID_JNIGLUE_H ? */
