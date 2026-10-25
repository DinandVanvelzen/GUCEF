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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#include "authpluginSTEAM_ISteamHostView.h"
#define GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H
#include "authpluginSTEAM_CSteamKeyValues.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H ? */

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

/** A user the Steam client on this machine has had logged in, as its files say; nothing of it is verified */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamLocalUser
{
    public:

    UInt64 steamId;                 /**< SteamID64 */
    CORE::CString accountName;      /**< the name of the login */
    CORE::CString personaName;      /**< the name shown to friends */
    bool mostRecent;                /**< the client marks this user as the last one that logged in */
    UInt64 timestamp;               /**< seconds since 1970 UTC of the last login, 0 when unknown */

    CSteamLocalUser( void );

    void Clear( void );
};

typedef GUCEF::vector< CSteamLocalUser > TSteamLocalUserVector;

/*-------------------------------------------------------------------------*/

/** What CSteamLocalDetector found */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamLocalInfo
{
    public:

    bool installed;                 /**< a Steam client is installed for this user */
    bool running;                   /**< and it is running (best effort) */
    CORE::CString installPath;      /**< the folder of the installation */
    bool hasUser;                   /**< 'user' is the user to name in the offer */
    CSteamLocalUser user;           /**< the user the client has logged in now, else the one that logged in last */

    CSteamLocalInfo( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Finds out, without the Steam SDK and without asking Steam anything, whether a Steam client is installed on this
 *  machine, whether it is running and who used it last. It only reads files (and on Windows the registry, and the
 *  process table), through an ISteamHostView, and changes nothing.
 *
 *  What it gives is a HINT and nothing else. It is not an identity: anyone who can write a file in the home directory of the user
 *  can say anything in it. It is shown to make an offer concrete ("Steam: <name> detected"), never used as the account
 *  of a login, never stored as a linked account and never sent anywhere. The login with Steam is the OpenID login in the
 *  browser, which proves who the person is.
 *
 *  Where the client is, per platform:
 *      Linux       ~/.steam/steam, ~/.local/share/Steam, the Flatpak one ~/.var/app/com.valvesoftware.Steam/.local/share/Steam
 *      Windows     the registry value SteamPath of HKCU\Software\Valve\Steam, else "%ProgramFiles(x86)%\Steam" and "%ProgramFiles%\Steam"
 *      macOS       ~/Library/Application Support/Steam
 *  Who: config/loginusers.vdf, the user with the key MostRecent set to 1, else the one with the latest Timestamp; while the client
 *  is running the user that is logged in now (ActiveUser) when the file knows it.
 *  Running: Linux and macOS, the process id of ~/.steam/steam.pid or of the ActiveProcess entry of registry.vdf, which is
 *  looked for in the process table; Windows, the registry values ActiveProcess\pid and SteamPID. The Flatpak client runs in
 *  a process space of its own: it is found installed and never running.
 *
 *  Files are read with a size limit and parsed with CSteamKeyValues; whatever is wrong with them (missing, cut short,
 *  huge, nonsense) gives less information, never an error.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamLocalDetector
{
    public:

    /** The largest loginusers.vdf and registry.vdf that is read */
    static const UInt32 MaxFileBytes = 1024 * 1024;

    /** The longest name that is given back, in code points; longer names are cut off */
    static const UInt32 MaxNameLength = 64;

    /** SteamID64 of the account with account id 0: the id of an individual account of the public universe is this plus the account id */
    static const UInt64 SteamIdBase = 76561197960265728ULL;

    /** Looks at the machine through 'view', not owned; without one at the real machine (CSystemSteamHostView) */
    explicit CSteamLocalDetector( const ISteamHostView* view = GUCEF_NULL );

    ~CSteamLocalDetector();

    void SetHostView( const ISteamHostView* view );

    const ISteamHostView& GetHostView( void ) const;

    /** Looks. True when a Steam client is installed; the info is cleared first and has what was found */
    bool Detect( CSteamLocalInfo& info ) const;

    /** The folders a Steam client could be installed in on this machine, in the order they are tried */
    void GetCandidateInstallFolders( CORE::CString::StringVector& folders ) const;

    /**
     *  The users of a loginusers.vdf: the keys of the block "users" that are a SteamID64 (an id that is not one is
     *  left out). False when the text could not be read as a whole or has no users block; users read before that are
     *  in the list. Names are cleaned up (MakeSafeName()).
     */
    static bool ParseLoginUsers( const CORE::CString& text, TSteamLocalUserVector& users );

    /**
     *  The process id and the user (an account id, 0 for nobody) of the ActiveProcess entry of a registry.vdf; false
     *  when the text has no such entry. Zero for what the entry does not have.
     */
    static bool ParseActiveProcess( const CORE::CString& text, UInt32& processId, UInt32& activeAccountId );

    /** The user to name: the one with this account id when there is one (not 0), else the one marked MostRecent (the latest of them), else the one with the latest Timestamp. False for no users */
    static bool ChooseUser( const TSteamLocalUserVector& users, UInt32 activeAccountId, CSteamLocalUser& user );

    /** A SteamID64 of an individual account: 17 digits' worth of a number from SteamIdBase up to SteamIdBase + 2^32 - 1 */
    static bool IsIndividualSteamId( UInt64 steamId );

    /** The text a name shown to the player is made of: control characters removed, trimmed, at most maxLength code points */
    static CORE::CString MakeSafeName( const CORE::CString& name, UInt32 maxLength );

    private:

    CSteamLocalDetector( const CSteamLocalDetector& src );
    CSteamLocalDetector& operator=( const CSteamLocalDetector& src );

    bool LooksLikeSteam( const CORE::CString& folder ) const;
    bool FindInstallFolder( CORE::CString& folder, CORE::CString::StringVector& allFolders ) const;
    bool IsClientRunning( const CORE::CString& installFolder, UInt32& activeAccountId ) const;

    private:

    const ISteamHostView* m_view;
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

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMLOCALDETECTOR_H ? */
