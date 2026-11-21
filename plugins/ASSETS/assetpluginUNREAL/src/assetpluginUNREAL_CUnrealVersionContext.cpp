/*
 *  assetpluginUNREAL: The file versions and custom versions of the package being read
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnrealVersionContext::CUnrealVersionContext( void )
    : m_legacyFileVersion( 0 )
    , m_legacyUe3Version( 0 )
    , m_ue4Version( 0 )
    , m_ue5Version( 0 )
    , m_licenseeVersion( 0 )
    , m_customVersions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CUnrealVersionContext::Clear( void )
{GUCEF_TRACE;

    m_legacyFileVersion = m_legacyUe3Version = m_ue4Version = m_ue5Version = m_licenseeVersion = 0;
    m_customVersions.clear();
}

/*-------------------------------------------------------------------------*/

void
CUnrealVersionContext::SetFileVersions( Int32 legacyFileVersion, Int32 legacyUe3Version, Int32 ue4Version, Int32 ue5Version, Int32 licenseeVersion )
{GUCEF_TRACE;

    m_legacyFileVersion = legacyFileVersion;
    m_legacyUe3Version = legacyUe3Version;
    m_ue4Version = ue4Version;
    m_ue5Version = ue5Version;
    m_licenseeVersion = licenseeVersion;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetLegacyFileVersion( void ) const
{GUCEF_TRACE;

    return m_legacyFileVersion;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetLegacyUe3Version( void ) const
{GUCEF_TRACE;

    return m_legacyUe3Version;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetUe4Version( void ) const
{GUCEF_TRACE;

    return m_ue4Version;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetUe5Version( void ) const
{GUCEF_TRACE;

    return m_ue5Version;
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetLicenseeVersion( void ) const
{GUCEF_TRACE;

    return m_licenseeVersion;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealVersionContext::IsAtLeast( EUnrealUe4Version version ) const
{GUCEF_TRACE;

    return m_ue4Version >= (Int32) version;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealVersionContext::IsAtLeast( EUnrealUe5Version version ) const
{GUCEF_TRACE;

    return m_ue5Version >= (Int32) version;
}

/*-------------------------------------------------------------------------*/

void
CUnrealVersionContext::SetCustomVersion( const CUnrealGuid& key, Int32 version )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_customVersions.size(); ++i )
    {
        if ( m_customVersions[ i ].key == key )
        {
            m_customVersions[ i ].version = version;
            return;
        }
    }
    m_customVersions.push_back( CUnrealCustomVersion( key, version ) );
}

/*-------------------------------------------------------------------------*/

Int32
CUnrealVersionContext::GetCustomVersion( const CUnrealGuid& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_customVersions.size(); ++i )
        if ( m_customVersions[ i ].key == key )
            return m_customVersions[ i ].version;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealVersionContext::HasCustomVersion( const CUnrealGuid& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_customVersions.size(); ++i )
        if ( m_customVersions[ i ].key == key )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

const CUnrealVersionContext::TCustomVersionVector&
CUnrealVersionContext::GetCustomVersions( void ) const
{GUCEF_TRACE;

    return m_customVersions;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealVersionContext::CheckSupported( CORE::CString& text ) const
{GUCEF_TRACE;

    if ( m_legacyFileVersion < (Int32) UE_LEGACY_NEWEST_SUPPORTED )
    {
        text = "Legacy file version " + CORE::Int32ToString( m_legacyFileVersion ) + " is newer than the newest supported (" + CORE::Int32ToString( (Int32) UE_LEGACY_NEWEST_SUPPORTED ) + ")";
        return false;
    }
    if ( m_legacyFileVersion > (Int32) UE_LEGACY_OLDEST_SUPPORTED )
    {
        text = "Legacy file version " + CORE::Int32ToString( m_legacyFileVersion ) + " is older than the oldest supported (" + CORE::Int32ToString( (Int32) UE_LEGACY_OLDEST_SUPPORTED ) + ")";
        return false;
    }
    if ( m_ue4Version < (Int32) UE4_MINIMUM_SUPPORTED )
    {
        text = "UE4 file version " + CORE::Int32ToString( m_ue4Version ) + " is older than the oldest supported (" + CORE::Int32ToString( (Int32) UE4_MINIMUM_SUPPORTED ) + ", engine 4.14)";
        return false;
    }
    if ( m_ue5Version > (Int32) UE5_NEWEST_SUPPORTED )
    {
        text = "UE5 file version " + CORE::Int32ToString( m_ue5Version ) + " is newer than the newest supported (" + CORE::Int32ToString( (Int32) UE5_NEWEST_SUPPORTED ) + ")";
        return false;
    }
    if ( m_ue5Version > 0 && m_ue5Version < (Int32) UE5_INITIAL_VERSION )
    {
        text = "UE5 file version " + CORE::Int32ToString( m_ue5Version ) + " is not a version of the format";
        return false;
    }
    if ( m_ue5Version > 0 && m_legacyFileVersion > -8 )
    {
        text = "A UE5 file version needs legacy file version -8 or newer, found " + CORE::Int32ToString( m_legacyFileVersion );
        return false;
    }
    if ( 0 != m_licenseeVersion )
    {
        text = "Licensee version " + CORE::Int32ToString( m_licenseeVersion ) + " is not supported (only 0)";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUnrealVersionContext::GuessEngineRelease( Int32 ue4Version, Int32 ue5Version )
{GUCEF_TRACE;

    if ( ue5Version > 0 )
    {
        if ( ue5Version >= 1018 ) return "5.7 to 5.8";
        if ( ue5Version >= 1017 ) return "5.6";
        if ( ue5Version >= 1012 ) return "5.4 to 5.5";
        if ( ue5Version >= 1009 ) return "5.3";
        if ( ue5Version >= 1004 ) return "5.1 to 5.2";
        return "5.0";
    }
    if ( ue4Version >= 522 ) return "4.27";
    if ( ue4Version >= 517 ) return "4.22 to 4.26";
    if ( ue4Version >= 508 ) return "4.14 to 4.21";
    return CORE::CString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
