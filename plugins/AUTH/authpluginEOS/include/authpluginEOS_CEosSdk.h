/*
 *  authpluginEOS: gucefAUTH plugin for logging in with Epic Online Services
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

#ifndef GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H
#define GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#include "authpluginEOS_EosShim.h"
#define GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H
#endif /* GUCEF_AUTHPLUGIN_EOS_EOSSHIM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The binding of the shim to the Epic Online Services SDK. It exists in two forms:
 *
 *   - Without the SDK (the default, what the repository builds): IsCompiledIn() is false and FillShim() fails, so the
 *     provider is not available. Nothing of the SDK is needed or referenced.
 *
 *   - With the SDK: the code in authpluginEOS_CEosSdk.cpp between #ifdef GUCEF_AUTH_HAS_EOS and #endif is compiled.
 *     It includes the headers of the SDK (eos_sdk.h, eos_auth.h, eos_connect.h) and calls the C interface of the SDK.
 *     The SDK is not vendored: its location is given to the compiler from outside, for a build with the usual scripts:
 *
 *         export GUCEF_AUTH_EOS_SDK_DIR=/path/to/EOS-SDK/SDK
 *         CXXFLAGS="-DGUCEF_AUTH_HAS_EOS=1 -I$GUCEF_AUTH_EOS_SDK_DIR/Include" \
 *         LDFLAGS="-L$GUCEF_AUTH_EOS_SDK_DIR/Bin -lEOSSDK-Linux-Shipping" cmake ...
 *
 *     (EOSSDK-Win64-Shipping.lib on Windows, the library of the platform elsewhere). The generated build files have no
 *     option for it and are not edited by hand.
 *
 *  UNVERIFIED AGAINST THE REAL SDK. The SDK is not available where this was written, so the guarded code has never
 *  been compiled or run. It is written from the documentation of the EOS C interface and every call and structure in
 *  it, in particular the options structures and their *_API_LATEST versions, the callback info members, the names of
 *  the EOS_EResult values used, EOS_LCT_DeviceCode and EOS_Auth_DeletePersistentAuth, is to be confirmed against the
 *  headers of the SDK version in use. Everything the provider does with the results is covered by tests through a
 *  fake table of functions.
 */
class GUCEF_AUTHPLUGIN_EOS_EXPORT_CPP CEosSdk
{
    public:

    CEosSdk( void );

    ~CEosSdk();

    /** Whether the build has the SDK compiled in (GUCEF_AUTH_HAS_EOS) */
    static bool IsCompiledIn( void );

    /** Fills the table with functions that run on the SDK. False when the SDK is not compiled in */
    bool FillShim( TEosShim& shim );

    /**
     *  Uses the platform handle (EOS_HPlatform) that the application made instead of creating one. The platform is
     *  borrowed: it is not ticked and not released by the plugin, the application keeps doing both. False without the SDK
     */
    bool AdoptPlatform( void* platformHandle );

    /** The state of the binding, defined with the SDK code */
    struct SImpl;

    private:

    CEosSdk( const CEosSdk& src );
    CEosSdk& operator=( const CEosSdk& src );

    SImpl* m_impl;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_EOS_CEOSSDK_H ? */
