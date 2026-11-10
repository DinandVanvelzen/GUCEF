/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The binding of the platform and of the P2P interface to the EOS SDK. The code that includes the headers of the SDK
 *  is compiled when the compiler is given GUCEF_AUTH_HAS_EOS and the include directory of the SDK, and the SDK library is
 *  linked (the flag is the one of the auth plugin: one SDK, one flag):
 *
 *      export GUCEF_AUTH_EOS_SDK_DIR=/path/to/EOS-SDK/SDK
 *      CXXFLAGS="-DGUCEF_AUTH_HAS_EOS=1 -I$GUCEF_AUTH_EOS_SDK_DIR/Include" \
 *      LDFLAGS="-L$GUCEF_AUTH_EOS_SDK_DIR/Bin -lEOSSDK-Linux-Shipping" cmake ...
 *
 *  Without the flag the functions give nothing and the glue has no default platform.
 *
 *  NEVER COMPILED OR RUN WHERE THIS WAS WRITTEN: there is no SDK on that machine. The names of the SDK are written
 *  from its documentation and are marked "to confirm" where they could not be checked.
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosPlatformSdk
{
    public:

    /** Whether the build has the SDK compiled in (GUCEF_AUTH_HAS_EOS) */
    static bool IsCompiledIn( void );

    /** Fills the table of the platform with functions that run on the SDK. False when the SDK is not compiled in */
    static bool FillPlatformShim( SEosPlatformShim& shim );

    /** Fills the table of the P2P interface with functions that run on the SDK. False when the SDK is not compiled in */
    static bool FillP2PShim( SEosP2PShim& shim );

    /**
     *  Uses the platform handle (EOS_HPlatform) that the application made instead of creating one. The platform is
     *  borrowed: it is not released by the glue, and it is not ticked by it when tickedByApplication is set: the application keeps ticking
     *  it itself. False without the SDK or when a platform exists already
     */
    static bool AdoptPlatform( void* platformHandle, bool tickedByApplication );

    /** The EOS_HPlatform of the SDK binding, for the bindings of the interfaces of other plugins; NULL when there is none */
    static void* GetPlatformHandle( void );

    private:

    CEosPlatformSdk( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMSDK_H ? */
