/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefAUTH_CLocalIdentityLinkStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CLocalIdentityLinkStore::FileVersion;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::CLocalIdentityLinkStore( void )
    : IIdentityLinkStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_path()
    , m_signer( GUCEF_NULL )
    , m_links()
    , m_foreign()
    , m_refusedCount( 0 )
    , m_loaded( false )
    , m_readOnly( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::CLocalIdentityLinkStore( const CORE::CString& path )
    : IIdentityLinkStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_path( path )
    , m_signer( GUCEF_NULL )
    , m_links()
    , m_foreign()
    , m_refusedCount( 0 )
    , m_loaded( false )
    , m_readOnly( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::~CLocalIdentityLinkStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::SetPath( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_path = path;
    m_links.clear();
    m_foreign.clear();
    m_refusedCount = 0;
    m_loaded = false;
    m_readOnly = false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLocalIdentityLinkStore::GetPath( void ) const
{GUCEF_TRACE;

    if ( m_path.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return CORE::RelativePath( m_path );
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::SetSigner( ILinkSigner* signer )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_signer = signer;
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalIdentityLinkStore::GetRefusedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refusedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalIdentityLinkStore::GetForeignCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_foreign.size();
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::IsAcceptable( const CLinkRecord& record ) const
{GUCEF_TRACE;

    if ( !record.IsKnownVersion() || !record.IsWellFormed() || !record.VerifySignature() )
        return false;
    // The own profile has one key
    if ( GUCEF_NULL != m_signer && record.profileKeyId == m_signer->GetKeyId() && record.publicKey != m_signer->GetPublicKeyHex() )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::EnsureLoaded( void )
{GUCEF_TRACE;

    // The lock is held by the caller
    if ( m_loaded )
        return;
    m_loaded = true;
    m_links.clear();
    m_foreign.clear();
    m_refusedCount = 0;
    m_readOnly = false;

    CORE::CString path = GetPath();
    if ( path.IsNULLOrEmpty() || !CORE::FileExists( path ) )
        return;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    CORE::CDataNode root;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() ||
         !codec->BuildDataTree( &root, path ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: The link file " << path << " cannot be read" );
        // Writing over what cannot be read would lose it
        m_readOnly = true;
        return;
    }

    // A codec may give the document as the root or as a child of it
    const CORE::CDataNode* document = &root;
    if ( GUCEF_NULL == root.FindChild( "links" ) )
    {
        CORE::CDataNode::const_iterator c = root.ConstBegin();
        while ( c != root.ConstEnd() )
        {
            if ( GUCEF_NULL != (*c)->FindChild( "links" ) )
            {
                document = *c;
                break;
            }
            ++c;
        }
    }
    UInt32 fileVersion = document->GetAttributeValueOrChildValueByName( "version" ).AsUInt32( FileVersion, true );
    m_readOnly = fileVersion > FileVersion;

    const CORE::CDataNode* list = document->FindChild( "links" );
    if ( GUCEF_NULL == list )
        return;
    CORE::CDataNode::const_iterator i = list->ConstBegin();
    while ( i != list->ConstEnd() )
    {
        CLinkRecord record;
        if ( !record.Deserialize( *(*i), CORE::CDataNodeSerializableSettings() ) )
        {
            ++m_refusedCount;
        }
        else
        if ( !record.IsKnownVersion() )
        {
            m_foreign.push_back( *(*i) );
        }
        else
        if ( !IsAcceptable( record ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: A link record in " << path << " was refused: it is not well formed or its signature does not verify" );
            ++m_refusedCount;
        }
        else
        if ( FindIndex( record.accountRef ) < m_links.size() )
        {
            // an account belongs to one profile: the second record of an account is not trusted over the first
            ++m_refusedCount;
        }
        else
        {
            m_links.push_back( record );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::Save( void )
{GUCEF_TRACE;

    // The lock is held by the caller
    CORE::CString path = GetPath();
    if ( path.IsNULLOrEmpty() )
        return true;
    if ( m_readOnly )
        return false;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: There is no \"" << codecName << "\" codec to write the link file " << path );
        return false;
    }

    CORE::CDataNode root( "AuthLinks", GUCEF_DATATYPE_OBJECT );
    root.SetAttribute( "version", FileVersion );
    CORE::CDataNode* list = root.AddChild( "links", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_links.size(); ++i )
        m_links[ i ].Serialize( *list->AddChild( "link", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
    for ( size_t i=0; i<m_foreign.size(); ++i )
        list->AddChild( m_foreign[ i ] );

    CORE::CDynamicBuffer buffer;
    CORE::CDynamicBufferAccess access( buffer );
    if ( !codec->StoreDataTree( &root, &access ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Unable to serialize the links" );
        return false;
    }

    CORE::CString dir = CORE::StripFilename( path );
    if ( !dir.IsNULLOrEmpty() && !CORE::DirExists( dir ) && !CORE::CreateDirs( dir ) )
        return false;
    if ( !CORE::WriteFileAtomically( path, buffer ) )
        return false;
    if ( !CORE::SetOwnerOnlyAccess( path ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Cannot restrict " << path << " to the user" );
    return true;
}

/*-------------------------------------------------------------------------*/

size_t
CLocalIdentityLinkStore::FindIndex( const CAccountRef& account ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_links.size(); ++i )
        if ( m_links[ i ].accountRef == account )
            return i;
    return m_links.size();
}

/*-------------------------------------------------------------------------*/

TLinkResult
CLocalIdentityLinkStore::Link( const CLinkRecord& record )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    if ( !IsAcceptable( record ) )
        return LINKRESULT_INVALID_RECORD;

    TLinkRecordVector previous = m_links;
    size_t index = FindIndex( record.accountRef );
    if ( index < m_links.size() )
    {
        if ( m_links[ index ].profileKeyId != record.profileKeyId )
            return LINKRESULT_CONFLICT;
        m_links[ index ] = record;
    }
    else
    {
        m_links.push_back( record );
    }

    if ( !Save() )
    {
        m_links = previous;
        return LINKRESULT_STORE_ERROR;
    }
    return LINKRESULT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::Unlink( const CAccountRef& account, const CORE::CString& profileKeyId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    size_t index = FindIndex( account );
    if ( index >= m_links.size() || m_links[ index ].profileKeyId != profileKeyId )
        return true;

    TLinkRecordVector previous = m_links;
    m_links.erase( m_links.begin() + index );
    if ( !Save() )
    {
        m_links = previous;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::GetLinks( const CORE::CString& profileKeyId, TLinkRecordVector& links )
{GUCEF_TRACE;

    links.clear();
    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    for ( size_t i=0; i<m_links.size(); ++i )
        if ( m_links[ i ].profileKeyId == profileKeyId )
            links.push_back( m_links[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::Resolve( const CAccountRef& account, CORE::CString& profileKeyId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    size_t index = FindIndex( account );
    if ( index >= m_links.size() )
        return false;
    profileKeyId = m_links[ index ].profileKeyId;
    return true;
}

/*-------------------------------------------------------------------------*/

TLinkResult
CLocalIdentityLinkStore::Merge( const CORE::CString& fromProfileKeyId ,
                                ILinkSigner& intoSigner               ,
                                UInt32* mergedCount                   )
{GUCEF_TRACE;

    if ( GUCEF_NULL != mergedCount )
        *mergedCount = 0;

    MT::CScopeMutex lock( m_lock );
    EnsureLoaded();
    if ( fromProfileKeyId.IsNULLOrEmpty() || fromProfileKeyId == intoSigner.GetKeyId() )
        return LINKRESULT_INVALID_RECORD;

    TLinkRecordVector previous = m_links;
    UInt32 moved = 0;
    UInt64 now = GetEpochTimeInMs();
    for ( size_t i=0; i<m_links.size(); ++i )
    {
        if ( m_links[ i ].profileKeyId != fromProfileKeyId )
            continue;
        CLinkRecord merged;
        if ( !CLinkRecord::Create( intoSigner, m_links[ i ].accountRef, m_links[ i ].proofDigest, now, merged, fromProfileKeyId ) ||
             !IsAcceptable( merged ) )
        {
            m_links = previous;
            return LINKRESULT_INVALID_RECORD;
        }
        m_links[ i ] = merged;
        ++moved;
    }
    if ( 0 == moved )
        return LINKRESULT_OK;

    if ( !Save() )
    {
        m_links = previous;
        return LINKRESULT_STORE_ERROR;
    }
    if ( GUCEF_NULL != mergedCount )
        *mergedCount = moved;
    return LINKRESULT_OK;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "path", m_path );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString path = config.GetAttributeValueOrChildValueByName( "path" ).AsString( m_path, false );
    if ( path != m_path )
        SetPath( path );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLocalIdentityLinkStore::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CLocalIdentityLinkStore";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
