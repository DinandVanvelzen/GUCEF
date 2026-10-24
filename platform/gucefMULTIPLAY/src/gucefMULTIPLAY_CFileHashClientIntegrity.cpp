/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefMULTIPLAY_CFileHashClientIntegrity.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CFileHashClientIntegrity::MissingFileValue = "missing";

/*-------------------------------------------------------------------------*/

CFileHashClientIntegrity::CFileHashClientIntegrity( void )
    : CIClientIntegrity()
    , CORE::CIConfigurable()
    , m_paths()
    , m_names()
    , m_report()
    , m_hashed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFileHashClientIntegrity::~CFileHashClientIntegrity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CFileHashClientIntegrity::AddFile( const CORE::CString& path, const CORE::CString& name )
{GUCEF_TRACE;

    if ( path.IsNULLOrEmpty() )
        return;
    m_paths.push_back( path );
    m_names.push_back( name.IsNULLOrEmpty() ? path : name );
    m_hashed = false;
}

/*-------------------------------------------------------------------------*/

void
CFileHashClientIntegrity::ClearFiles( void )
{GUCEF_TRACE;

    m_paths.clear();
    m_names.clear();
    m_report.Clear();
    m_hashed = false;
}

/*-------------------------------------------------------------------------*/

bool
CFileHashClientIntegrity::HashFile( const CORE::CString& path, CORE::CString& hash )
{GUCEF_TRACE;

    CORE::CFileAccess file( CORE::RelativePath( path ), "rb" );
    if ( !file.Opened() )
        return false;
    CORE::CSha256 sha;
    UInt8 block[ 64 * 1024 ];
    UInt32 read = 0;
    while ( ( read = file.Read( block, 1, sizeof( block ) ) ) > 0 )
        sha.Update( block, read );
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    hash = DRN::CDRNCrypto::ToHex( digest, CORE::CSha256::DigestSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileHashClientIntegrity::GetIntegrityReport( CORE::CValueList& report )
{GUCEF_TRACE;

    if ( m_paths.empty() )
        return false;
    if ( !m_hashed )
    {
        m_report.Clear();
        m_report.SetAllowMultipleValues( false );
        for ( size_t i=0; i<m_paths.size(); ++i )
        {
            CORE::CString hash;
            if ( !HashFile( m_paths[ i ], hash ) )
                hash = MissingFileValue;
            m_report.Set( m_names[ i ], hash );
        }
        m_hashed = true;
    }
    report = m_report;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileHashClientIntegrity::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "files", CORE::CString::Empty.Combine( m_paths, ';' ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFileHashClientIntegrity::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString files = config.GetAttributeValueOrChildValueByName( "files" ).AsString( CORE::CString::Empty, true );
    if ( files.IsNULLOrEmpty() )
        return true;
    ClearFiles();
    CORE::CString::StringVector paths = files.ParseElements( ';', false );
    for ( size_t i=0; i<paths.size(); ++i )
        AddFile( paths[ i ].Trim( true ).Trim( false ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFileHashClientIntegrity::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CFileHashClientIntegrity";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
