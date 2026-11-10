/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAMHOST_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAMHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#include "authpluginSTEAM_ISteamHostView.h"
#define GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_ISTEAMHOSTVIEW_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A machine that lives in the test: a platform, a home, files and folders as text, processes and a registry */
class CFakeHostView : public GUCEF::AUTHPLUGIN::STEAM::ISteamHostView
{
    public:

    GUCEF::AUTHPLUGIN::STEAM::TSteamHostPlatform platform;
    GUCEF::CORE::CString home;
    GUCEF::CORE::CString::StringMap env;
    GUCEF::CORE::CString::StringMap files;
    GUCEF::CORE::CString::StringSet dirs;
    GUCEF::CORE::CString::StringMap processes;         /**< process id as text -> its name */
    GUCEF::CORE::CString::StringMap registryStrings;   /**< key|value -> text */
    GUCEF::CORE::CString::StringMap registryDwords;    /**< key|value -> number as text */
    mutable GUCEF::UInt32 reads;

    explicit CFakeHostView( GUCEF::AUTHPLUGIN::STEAM::TSteamHostPlatform hostPlatform = GUCEF::AUTHPLUGIN::STEAM::STEAMHOST_LINUX, const GUCEF::CORE::CString& homeDir = "/home/test" )
        : GUCEF::AUTHPLUGIN::STEAM::ISteamHostView(), platform( hostPlatform ), home( homeDir ), env(), files(), dirs(), processes(), registryStrings(), registryDwords(), reads( 0 )
    {}

    virtual GUCEF::AUTHPLUGIN::STEAM::TSteamHostPlatform GetPlatform( void ) const GUCEF_VIRTUAL_OVERRIDE { return platform; }
    virtual GUCEF::CORE::CString GetHomeDirectory( void ) const GUCEF_VIRTUAL_OVERRIDE { return home; }

    virtual GUCEF::CORE::CString GetEnvironmentValue( const GUCEF::CORE::CString& name ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString::StringMap::const_iterator i = env.find( name );
        return i != env.end() ? (*i).second : GUCEF::CORE::CString::Empty;
    }

    virtual bool DirectoryExists( const GUCEF::CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE { return dirs.find( path ) != dirs.end(); }
    virtual bool FileExists( const GUCEF::CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE { return files.find( path ) != files.end(); }

    virtual bool ReadTextFile( const GUCEF::CORE::CString& path, GUCEF::UInt32 maxBytes, GUCEF::CORE::CString& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        content.Clear();
        GUCEF::CORE::CString::StringMap::const_iterator i = files.find( path );
        if ( i == files.end() || strlen( (*i).second.C_String() ) > maxBytes )
            return false;
        ++reads;
        content = (*i).second;
        return true;
    }

    virtual bool IsProcessRunning( GUCEF::UInt32 processId, const GUCEF::CORE::CString& nameHint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString::StringMap::const_iterator i = processes.find( GUCEF::CORE::UInt32ToString( processId ) );
        if ( i == processes.end() )
            return false;
        return nameHint.IsNULLOrEmpty() || (*i).second.Lowercase().HasSubstr( nameHint.Lowercase(), true ) >= 0;
    }

    virtual bool ReadRegistryString( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& valueName, GUCEF::CORE::CString& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString::StringMap::const_iterator i = registryStrings.find( key + "|" + valueName );
        if ( i == registryStrings.end() )
            return false;
        value = (*i).second;
        return true;
    }

    virtual bool ReadRegistryDword( const GUCEF::CORE::CString& key, const GUCEF::CORE::CString& valueName, GUCEF::UInt32& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CString::StringMap::const_iterator i = registryDwords.find( key + "|" + valueName );
        if ( i == registryDwords.end() )
            return false;
        value = GUCEF::CORE::StringToUInt32( (*i).second, 0 );
        return true;
    }

    /** A folder and the folders above it */
    void AddDir( const GUCEF::CORE::CString& path )
    {
        GUCEF::CORE::CString current = path;
        while ( !current.IsNULLOrEmpty() && dirs.find( current ) == dirs.end() )
        {
            dirs.insert( current );
            GUCEF::Int32 slash = -1;
            for ( GUCEF::Int32 i=(GUCEF::Int32) strlen( current.C_String() ) - 1; i>=0; --i )
            {
                if ( '/' == current.C_String()[ i ] )
                {
                    slash = i;
                    break;
                }
            }
            if ( slash <= 0 )
                break;
            current = GUCEF::CORE::CString( current.C_String(), (GUCEF::UInt32) slash );
        }
    }

    void AddFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& content )
    {
        files[ path ] = content;
        GUCEF::Int32 slash = -1;
        for ( GUCEF::Int32 i=(GUCEF::Int32) strlen( path.C_String() ) - 1; i>=0; --i )
        {
            if ( '/' == path.C_String()[ i ] )
            {
                slash = i;
                break;
            }
        }
        if ( slash > 0 )
            AddDir( GUCEF::CORE::CString( path.C_String(), (GUCEF::UInt32) slash ) );
    }
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKESTEAMHOST_H ? */
