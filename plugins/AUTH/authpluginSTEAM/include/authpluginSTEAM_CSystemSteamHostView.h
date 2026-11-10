/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H
#define GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#include "authpluginSTEAM_ISteamHostView.h"
#define GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The view of the machine the process runs on. Files are read through gucefCORE; the home directory is the one of the
 *  user ($HOME, the profile folder). A process is looked for with the process table (/proc on Linux, kill( id, 0 ) on
 *  macOS, OpenProcess on Windows); the registry exists on Windows only (HKEY_CURRENT_USER).
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSystemSteamHostView : public ISteamHostView
{
    public:

    CSystemSteamHostView( void );

    virtual ~CSystemSteamHostView();

    /** The one view of the real machine; it keeps no state */
    static const CSystemSteamHostView& Instance( void );

    virtual TSteamHostPlatform GetPlatform( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetHomeDirectory( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetEnvironmentValue( const CORE::CString& name ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DirectoryExists( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool FileExists( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReadTextFile( const CORE::CString& path, UInt32 maxBytes, CORE::CString& content ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsProcessRunning( UInt32 processId, const CORE::CString& nameHint ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReadRegistryString( const CORE::CString& key, const CORE::CString& valueName, CORE::CString& value ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ReadRegistryDword( const CORE::CString& key, const CORE::CString& valueName, UInt32& value ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSYSTEMSTEAMHOSTVIEW_H ? */
