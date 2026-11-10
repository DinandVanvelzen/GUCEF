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

#include <string.h>

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
const CORE::CString CLocalIdentityLinkStore::LockFileSuffix = ".lock";
const UInt32 CLocalIdentityLinkStore::DefaultLockTimeoutInMs = 10000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether two records are the same in every field */
bool
IsSameRecord( const CLinkRecord& a, const CLinkRecord& b )
{GUCEF_TRACE;

    return a.version == b.version && a.accountRef == b.accountRef && a.profileKeyId == b.profileKeyId &&
           a.linkedAtMs == b.linkedAtMs && a.proofDigest == b.proofDigest && a.publicKey == b.publicKey &&
           a.signature == b.signature && a.mergedFrom == b.mergedFrom;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What was read of the file of the store while the lock was held */
struct CLocalIdentityLinkStore::SFileImage
{
    bool present;
    CORE::CDynamicBuffer content;
    UInt8 digest[ CORE::CSha256::DigestSize ];

    SFileImage( void )
        : present( false )
        , content()
    {GUCEF_TRACE;

        memset( digest, 0, sizeof( digest ) );
    }

    /** The digest over the presence, the size and the bytes of the file */
    void ComputeDigest( void )
    {GUCEF_TRACE;

        UInt32 size = content.GetDataSize();
        UInt8 prefix[ 5 ] = { (UInt8) ( present ? 1 : 0 ), (UInt8) size, (UInt8) ( size >> 8 ), (UInt8) ( size >> 16 ), (UInt8) ( size >> 24 ) };
        CORE::CSha256 sha;
        sha.Update( prefix, sizeof( prefix ) );
        if ( size > 0 )
            sha.Update( content.GetConstBufferPtr(), size );
        sha.Final( digest );
    }

    private:

    SFileImage( const SFileImage& src );
    SFileImage& operator=( const SFileImage& src );
};

/*-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::CLocalIdentityLinkStore( void )
    : IIdentityLinkStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_path()
    , m_signer( GUCEF_NULL )
    , m_lockTimeoutInMs( DefaultLockTimeoutInMs )
    , m_links()
    , m_foreign()
    , m_refusedCount( 0 )
    , m_loaded( false )
    , m_readOnly( false )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::CLocalIdentityLinkStore( const CORE::CString& path )
    : IIdentityLinkStore()
    , CORE::CIConfigurable()
    , m_lock()
    , m_path( path )
    , m_signer( GUCEF_NULL )
    , m_lockTimeoutInMs( DefaultLockTimeoutInMs )
    , m_links()
    , m_foreign()
    , m_refusedCount( 0 )
    , m_loaded( false )
    , m_readOnly( false )
{GUCEF_TRACE;

    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

CLocalIdentityLinkStore::~CLocalIdentityLinkStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::ForgetView( void ) const
{GUCEF_TRACE;

    // The lock is held by the caller
    m_links.clear();
    m_foreign.clear();
    m_refusedCount = 0;
    m_loaded = false;
    m_readOnly = false;
    memset( m_fileDigest, 0, sizeof( m_fileDigest ) );
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::SetPath( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_path = path;
    ForgetView();
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

CORE::CString
CLocalIdentityLinkStore::GetLockPath( void ) const
{GUCEF_TRACE;

    CORE::CString path = GetPath();
    if ( path.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return path + LockFileSuffix;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::SetLockTimeoutInMs( UInt32 lockTimeoutInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_lockTimeoutInMs = lockTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalIdentityLinkStore::GetLockTimeoutInMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lockTimeoutInMs;
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
    RefreshView();
    return m_refusedCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CLocalIdentityLinkStore::GetForeignCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    RefreshView();
    return (UInt32) m_foreign.size();
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::IsAcceptable( const CLinkRecord& record, const TLinkRecordVector* verified ) const
{GUCEF_TRACE;

    // A record that is in the view as it is now was verified when it came in: the signature is not checked again
    bool known = false;
    if ( GUCEF_NULL != verified )
    {
        for ( size_t i=0; i<verified->size(); ++i )
        {
            if ( IsSameRecord( (*verified)[ i ], record ) )
            {
                known = true;
                break;
            }
        }
    }
    if ( !known && ( !record.IsKnownVersion() || !record.IsWellFormed() || !record.VerifySignature() ) )
        return false;
    // The own profile has one key
    if ( GUCEF_NULL != m_signer && record.profileKeyId == m_signer->GetKeyId() && record.publicKey != m_signer->GetPublicKeyHex() )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::ReadFile( const CORE::CString& path, SFileImage& image ) const
{GUCEF_TRACE;

    // The lock of the file is held by the caller. A file that is there but cannot be read is there with no content,
    // which no codec reads: the file is then left alone
    image.present = CORE::FileExists( path );
    if ( image.present && !image.content.LoadContentFromFile( path ) )
        image.content.Clear();
    image.ComputeDigest();
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::UpdateView( const SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller. The file is what it was at the last read or write: the view stands
    if ( m_loaded && 0 == memcmp( m_fileDigest, image.digest, sizeof( m_fileDigest ) ) )
        return;
    ParseImage( image );
}

/*-------------------------------------------------------------------------*/

void
CLocalIdentityLinkStore::ParseImage( const SFileImage& image ) const
{GUCEF_TRACE;

    // The lock is held by the caller. The records of the view as it was are verified already
    TLinkRecordVector verified;
    verified.swap( m_links );
    m_foreign.clear();
    m_refusedCount = 0;
    m_readOnly = false;
    m_loaded = true;
    memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
    if ( !image.present )
        return;

    CORE::CString path = GetPath();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    CORE::CString codecName = CORE::ExtractFileExtention( path ).Lowercase();
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( codecName, codec, false ) || codec.IsNULL() )
    {
        // Without the codec nothing can be read: that says nothing about the file, it is looked at again next time
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: There is no \"" << codecName << "\" codec, the link file " << path << " cannot be read" );
        m_readOnly = true;
        m_loaded = false;
        return;
    }
    CORE::CDataNode root;
    CORE::CDynamicBufferAccess access( image.content );
    if ( 0 == image.content.GetDataSize() || !codec->BuildDataTree( &root, &access ) )
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
        if ( !IsAcceptable( record, &verified ) )
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
CLocalIdentityLinkStore::RefreshView( void ) const
{GUCEF_TRACE;

    // The lock is held by the caller. Nothing is made for a read: without the file there is nothing to lock
    CORE::CString path = GetPath();
    if ( path.IsNULLOrEmpty() )
        return true;
    if ( !CORE::FileExists( path ) )
    {
        SFileImage none;
        none.ComputeDigest();
        UpdateView( none );
        return true;
    }

    CFileLock fileLock;
    CFileLock::TLockOutcome outcome = fileLock.Acquire( GetLockPath(), false, m_lockTimeoutInMs );
    if ( CFileLock::LOCKOUTCOME_TIMED_OUT == outcome )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: The link file " << path << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, it is not read" );
        return false;
    }
    // LOCKOUTCOME_UNAVAILABLE: a directory in which the lock file cannot be made is not written by anyone, it is read as it is

    SFileImage image;
    ReadFile( path, image );
    UpdateView( image );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::PrepareChange( CFileLock& fileLock, bool makeDirectory )
{GUCEF_TRACE;

    // The lock is held by the caller. On success the view is what is in the file now and, for a store with a file,
    // fileLock holds the exclusive lock of it until the change is written
    CORE::CString path = GetPath();
    if ( path.IsNULLOrEmpty() )
        return true;

    CORE::CString dir = CORE::StripFilename( path );
    if ( !dir.IsNULLOrEmpty() && !CORE::DirExists( dir ) )
    {
        if ( !makeDirectory )
        {
            // nothing to change, and nothing is made for it
            SFileImage none;
            none.ComputeDigest();
            UpdateView( none );
            return true;
        }
        if ( !CORE::CreateDirs( dir ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Cannot create the directory " << dir );
            return false;
        }
    }

    CORE::CString lockPath = GetLockPath();
    bool lockFileExisted = CORE::FileExists( lockPath );
    CFileLock::TLockOutcome outcome = fileLock.Acquire( lockPath, true, m_lockTimeoutInMs );
    if ( CFileLock::LOCKOUTCOME_TAKEN != outcome && CFileLock::IsSupported() )
    {
        if ( CFileLock::LOCKOUTCOME_TIMED_OUT == outcome )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: The link file " << path << " stayed locked by another instance for " << m_lockTimeoutInMs << " ms, nothing is changed" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Cannot lock the link file " << path << " (" << lockPath << "), nothing is changed" );
        }
        return false;
    }
    // a lock file made now is restricted like the file itself
    if ( CFileLock::LOCKOUTCOME_TAKEN == outcome && !lockFileExisted && !CORE::SetOwnerOnlyAccess( lockPath ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Cannot restrict " << lockPath << " to the user" );

    // the change applies to what is in the file now, not to what this instance read before
    SFileImage image;
    ReadFile( path, image );
    UpdateView( image );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::Save( void )
{GUCEF_TRACE;

    // The lock is held by the caller, and the exclusive lock of the file: the view is what was read under it
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

    SFileImage image;
    CORE::CDynamicBufferAccess access( image.content );
    if ( !codec->StoreDataTree( &root, &access ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Unable to serialize the links" );
        return false;
    }

    CORE::CString dir = CORE::StripFilename( path );
    if ( !dir.IsNULLOrEmpty() && !CORE::DirExists( dir ) && !CORE::CreateDirs( dir ) )
        return false;
    if ( !CORE::WriteFileAtomically( path, image.content ) )
        return false;
    if ( !CORE::SetOwnerOnlyAccess( path ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LocalIdentityLinkStore: Cannot restrict " << path << " to the user" );

    // the file as it is now; records that were refused when it was read are no longer in it
    image.present = true;
    image.ComputeDigest();
    memcpy( m_fileDigest, image.digest, sizeof( m_fileDigest ) );
    m_refusedCount = 0;
    m_loaded = true;
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
    if ( !IsAcceptable( record ) )
        return LINKRESULT_INVALID_RECORD;
    CFileLock fileLock;
    if ( !PrepareChange( fileLock, true ) )
        return LINKRESULT_STORE_ERROR;

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
    CFileLock fileLock;
    if ( !PrepareChange( fileLock, false ) )
        return false;
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
    if ( !RefreshView() )
        return false;
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
    if ( !RefreshView() )
        return false;
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
    if ( fromProfileKeyId.IsNULLOrEmpty() || fromProfileKeyId == intoSigner.GetKeyId() )
        return LINKRESULT_INVALID_RECORD;
    CFileLock fileLock;
    if ( !PrepareChange( fileLock, false ) )
        return LINKRESULT_STORE_ERROR;

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

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "path", m_path );
    config.SetAttribute( "lockTimeoutInMs", m_lockTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLocalIdentityLinkStore::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString path = config.GetAttributeValueOrChildValueByName( "path" ).AsString( m_path, false );
    m_lockTimeoutInMs = config.GetAttributeValueOrChildValueByName( "lockTimeoutInMs" ).AsUInt32( m_lockTimeoutInMs, true );
    if ( path != m_path )
    {
        m_path = path;
        ForgetView();
    }
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
