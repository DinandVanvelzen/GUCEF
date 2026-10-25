/*
 *  lobbyservicepluginEOS: gucefMULTIPLAY lobby service plugin for lobbies on Epic Online Services
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

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H
#define GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#include "pluginglueEOS_EosShim.h"
#define GUCEF_PLUGINGLUE_EOS_EOSSHIM_H
#endif /* GUCEF_PLUGINGLUE_EOS_EOSSHIM_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H
#include "lobbyservicepluginEOS_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The binding of the lobby interface to the EOS SDK, behind the function table of the glue (SEosLobbyShim). The code
 *  that includes the headers of the SDK is compiled when the compiler is given GUCEF_AUTH_HAS_EOS and the include
 *  directory of the SDK, and the SDK library is linked (one flag for the SDK: the platform and the P2P binding of
 *  pluginglueEOS use it too, see CEosPlatformSdk).
 *
 *  NEVER COMPILED OR RUN WHERE THIS WAS WRITTEN: there is no SDK on that machine. The functions, structures and enumerations
 *  of EOS_Lobby_*, EOS_LobbyModification_*, EOS_LobbySearch_* and EOS_LobbyDetails_* are written from the documentation of
 *  the SDK; every name is to confirm against the headers of the SDK in use (the sources mark the ones that are least sure).
 */
class GUCEF_MULTIPLAYPLUGIN_EOS_EXPORT_CPP CEosLobbySdk
{
    public:

    /** Whether the build has the SDK compiled in (GUCEF_AUTH_HAS_EOS) */
    static bool IsCompiledIn( void );

    /** Fills the table with functions that run on the SDK; the table of the platform is that of the glue. False without the SDK */
    static bool FillShim( PLUGINGLUE::EOS::SEosLobbyShim& shim );

    private:

    CEosLobbySdk( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_EOS_CEOSLOBBYSDK_H ? */
