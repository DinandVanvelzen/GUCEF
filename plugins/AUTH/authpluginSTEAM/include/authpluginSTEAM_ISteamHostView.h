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

#ifndef GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#define GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The kind of machine CSteamLocalDetector looks at, which decides where a Steam installation can be */
enum ESteamHostPlatform
{
    STEAMHOST_LINUX = 0     ,
    STEAMHOST_WINDOWS = 1   ,
    STEAMHOST_MACOS = 2     ,
    STEAMHOST_OTHER = 3         /**< a platform without a desktop Steam client: nothing is detected */
};

typedef enum ESteamHostPlatform TSteamHostPlatform;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What CSteamLocalDetector sees of the machine it runs on: the operating system, the home directory, the files, the
 *  processes and on Windows the registry. The detector reads nothing else, so a test gives it a view of a made up
 *  Steam installation in a temporary directory (CSystemSteamHostView is the view of the real machine).
 *  Everything here is a read: a view never changes the machine.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP ISteamHostView
{
    public:

    ISteamHostView( void );

    virtual ~ISteamHostView();

    virtual TSteamHostPlatform GetPlatform( void ) const = 0;

    /** The home directory of the user, no trailing separator; empty when there is none */
    virtual CORE::CString GetHomeDirectory( void ) const = 0;

    /** The value of an environment variable, empty when there is none */
    virtual CORE::CString GetEnvironmentValue( const CORE::CString& name ) const = 0;

    virtual bool DirectoryExists( const CORE::CString& path ) const = 0;

    virtual bool FileExists( const CORE::CString& path ) const = 0;

    /** The file as text. False when it is not there, cannot be read or is larger than maxBytes (then nothing is read) */
    virtual bool ReadTextFile( const CORE::CString& path, UInt32 maxBytes, CORE::CString& content ) const = 0;

    /**
     *  Whether a process with that id is running and, when the view can tell its name, that its name contains
     *  nameHint (case insensitive). A view that cannot see the name goes by the id alone.
     */
    virtual bool IsProcessRunning( UInt32 processId, const CORE::CString& nameHint ) const = 0;

    /** A string value of the registry of the current user, for example key "Software\Valve\Steam" and value "SteamPath"; false without a registry or a value */
    virtual bool ReadRegistryString( const CORE::CString& key, const CORE::CString& valueName, CORE::CString& value ) const = 0;

    /** A DWORD value of the registry of the current user; false without a registry or a value */
    virtual bool ReadRegistryDword( const CORE::CString& key, const CORE::CString& valueName, UInt32& value ) const = 0;

    private:

    ISteamHostView( const ISteamHostView& src );
    ISteamHostView& operator=( const ISteamHostView& src );
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

#endif /* GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H ? */
