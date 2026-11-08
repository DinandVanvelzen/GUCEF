/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#include "gucefCOMCORE_CRpcBuiltInContracts.h"
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */

#include "gucefCOMCORE_CRpcContractRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRpcContractRegistry::ContractRegisteredEvent = "GUCEF::COMCORE::CRpcContractRegistry::ContractRegisteredEvent";

/*-------------------------------------------------------------------------*/

namespace {

bool
ReadWholeFile( const CORE::CString& path, CORE::CDynamicBuffer& content )
{
    content.Clear( true );
    if ( !CORE::FileExists( path ) )
        return false;
    return content.LoadContentFromFile( path );
}

struct SCacheFile
{
    CORE::CString path;
    UInt64 size;
    UInt64 timestamp;
};

void
ListCacheFiles( const CORE::CString& dir, UInt32 depth, GUCEF::vector< SCacheFile >& files )
{
    struct CORE::SDI_Data* data = CORE::DI_First_Dir_Entry( CORE::ToAsciiString( dir ).C_String() );
    if ( GUCEF_NULL == data )
        return;
    do
    {
        CORE::CString name = CORE::DI_Name( data );
        if ( name == "." || name == ".." || name.IsNULLOrEmpty() )
            continue;
        CORE::CString path = dir;
        CORE::AppendToPath( path, name );
        if ( 0 != CORE::DI_Is_It_A_File( data ) )
        {
            SCacheFile file;
            file.path = path;
            file.size = CORE::DI_Size( data );
            file.timestamp = CORE::DI_Timestamp( data );
            files.push_back( file );
        }
        else if ( depth < 3 )
            ListCacheFiles( path, depth + 1, files );
    }
    while ( 0 != CORE::DI_Next_Dir_Entry( data ) );
    CORE::DI_Cleanup( data );
}

}

/*-------------------------------------------------------------------------*/

void
CRpcContractRegistry::RegisterEvents( void )
{GUCEF_TRACE;

    ContractRegisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRpcContractRegistry::CRpcContractRegistry( void )
    : CORE::CTSGNotifier()
    , CORE::CIConfigurable()
    , m_lock()
    , m_entries()
    , m_cacheDir( "$CACHEDIR$/gucef/rpc-contracts" )
    , m_cacheMaxBytes( DefaultCacheMaxBytes )
{GUCEF_TRACE;

    RegisterEvents();
    CRpcContractPtr rpc = CRpcBuiltInContracts::GetRpcContract();
    if ( !rpc.IsNULL() )
        RegisterLocked( rpc, CONTRACTSOURCE_BUILTIN, false );
}

/*-------------------------------------------------------------------------*/

CRpcContractRegistry::~CRpcContractRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::RegisterLocked( const CRpcContractPtr& contract, TContractSource source, bool notify )
{GUCEF_TRACE;

    if ( contract.IsNULL() )
        return false;
    {
        MT::CScopeMutex lock( m_lock );
        TEntryMap::iterator i = m_entries.find( contract->GetHash() );
        if ( i != m_entries.end() )
        {
            // a contract known from the cache or a peer that the application registers itself gets the stronger source
            if ( source < (*i).second.source )
                (*i).second.source = source;
            return true;
        }
        SEntry entry;
        entry.contract = contract;
        entry.source = source;
        m_entries[ contract->GetHash() ] = entry;
    }
    if ( CONTRACTSOURCE_PEER == source || CONTRACTSOURCE_FILE == source )
        StoreInCache( contract, contract->HasDocs() ? &contract->GetDocs() : GUCEF_NULL );
    if ( notify )
    {
        // the mailbox takes a copy
        CRpcContractEventData data( contract, source );
        NotifyObserversFromThread( ContractRegisteredEvent, &data );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::Register( const CRpcContractPtr& contract, TContractSource source )
{GUCEF_TRACE;

    return RegisterLocked( contract, source, true );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::RegisterFromDataNode( const CORE::CDataNode& source, CRpcContractPtr* registered, CORE::CString& error )
{GUCEF_TRACE;

    CRpcContractPtr contract = CRpcContract::LoadFromDataNode( source, error );
    if ( contract.IsNULL() )
        return false;
    if ( GUCEF_NULL != registered )
        *registered = contract;
    return Register( contract, CONTRACTSOURCE_FILE );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::RegisterFromJsonText( const CORE::CString& jsonText, CRpcContractPtr* registered, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
    {
        error = "the json dstore codec is not available";
        return false;
    }
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( jsonText, 0, false );
    CORE::CDynamicBufferAccess access( buffer );
    CORE::CDataNode tree;
    if ( !codec->BuildDataTree( &tree, &access ) )
    {
        error = "the contract is not valid JSON";
        return false;
    }
    return RegisterFromDataNode( tree, registered, error );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::RegisterFromJsonFile( const CORE::CString& path, CRpcContractPtr* registered, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDynamicBuffer content;
    CORE::CString resolved = CORE::ResolveVars( path );
    if ( !ReadWholeFile( resolved, content ) )
    {
        error = "cannot read \"" + resolved + "\"";
        return false;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
    {
        error = "the json dstore codec is not available";
        return false;
    }
    CORE::CDynamicBufferAccess access( content );
    CORE::CDataNode tree;
    if ( !codec->BuildDataTree( &tree, &access ) )
    {
        error = "\"" + resolved + "\" is not valid JSON";
        return false;
    }
    if ( !RegisterFromDataNode( tree, registered, error ) )
    {
        error = resolved + ": " + error;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::RegisterFromSchema( const void* schema, UInt32 size, TContractSource source, CRpcContractPtr* registered,
                                          CORE::CString& error, const void* docs, UInt32 docsSize )
{GUCEF_TRACE;

    CRpcContractPtr contract = CRpcContract::FromSchema( schema, size, error, docs, docsSize );
    if ( contract.IsNULL() )
        return false;
    if ( GUCEF_NULL != registered )
        *registered = contract;
    return Register( contract, source );
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContractRegistry::FindByHash( const CRpcContractHash& hash ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TEntryMap::const_iterator i = m_entries.find( hash );
        if ( i != m_entries.end() )
            return (*i).second.contract;
    }
    return LoadFromCache( hash );
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContractRegistry::FindLatest( const CORE::CString& name, UInt32 major ) const
{GUCEF_TRACE;

    return FindAtLeast( name, major, 0 );
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContractRegistry::FindAtLeast( const CORE::CString& name, UInt32 major, UInt32 minMinor ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CRpcContractPtr best;
    TEntryMap::const_iterator i = m_entries.begin();
    for ( ; i != m_entries.end(); ++i )
    {
        const SEntry& entry = (*i).second;
        if ( CONTRACTSOURCE_BUILTIN != entry.source && CONTRACTSOURCE_FILE != entry.source )
            continue;
        const CRpcContract& c = *entry.contract;
        if ( c.GetName() != name || c.GetMajor() != major || c.GetMinor() < minMinor )
            continue;
        if ( best.IsNULL() || c.GetMinor() > best->GetMinor() )
            best = entry.contract;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

void
CRpcContractRegistry::GetContracts( TRpcContractPtrVector& contracts ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    contracts.clear();
    TEntryMap::const_iterator i = m_entries.begin();
    for ( ; i != m_entries.end(); ++i )
        contracts.push_back( (*i).second.contract );
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::GetSource( const CRpcContractHash& hash, TContractSource& source ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( hash );
    if ( i == m_entries.end() )
        return false;
    source = (*i).second.source;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::Unregister( const CRpcContractHash& hash )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( hash );
    if ( i == m_entries.end() || CONTRACTSOURCE_BUILTIN == (*i).second.source )
        return false;
    m_entries.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcContractRegistry::GetCacheFilePath( const CRpcContractPtr& contract ) const
{GUCEF_TRACE;

    CORE::CString path = GetCacheDirectory();
    CORE::AppendToPath( path, contract->GetName() );
    CORE::AppendToPath( path, CORE::ToString( contract->GetMajor() ) + "." + CORE::ToString( contract->GetMinor() ) );
    CORE::AppendToPath( path, contract->GetHash().ToHexString() + ".bin" );
    return path;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::StoreInCache( const CRpcContractPtr& contract, const CORE::CDynamicBuffer* docs )
{GUCEF_TRACE;

    if ( contract.IsNULL() )
        return false;
    CORE::CString dir = GetCacheDirectory();
    if ( dir.IsNULLOrEmpty() )
        return false;
    CORE::CString path = GetCacheFilePath( contract );
    if ( !CORE::CreatePathDirectories( path ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RpcContractRegistry: cannot create the cache directory of " << path << ", the cache is not used" );
        return false;
    }
    if ( !CORE::FileExists( path ) )
    {
        if ( !CORE::WriteFileAtomically( path, contract->GetSchema() ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RpcContractRegistry: cannot write " << path );
            return false;
        }
    }
    if ( GUCEF_NULL != docs && docs->GetDataSize() > 0 )
    {
        CORE::CString docsPath = path.CutChars( 4, false ) + ".docs.bin";
        if ( !CORE::FileExists( docsPath ) )
            CORE::WriteFileAtomically( docsPath, *docs );
    }
    PruneCache();
    return true;
}

/*-------------------------------------------------------------------------*/

CRpcContractPtr
CRpcContractRegistry::LoadFromCache( const CRpcContractHash& hash ) const
{GUCEF_TRACE;

    CORE::CString dir = GetCacheDirectory();
    if ( dir.IsNULLOrEmpty() || !CORE::DirExists( dir ) )
        return CRpcContractPtr();

    CORE::CString wanted = hash.ToHexString() + ".bin";
    GUCEF::vector< SCacheFile > files;
    ListCacheFiles( dir, 0, files );
    for ( UInt32 i=0; i<files.size(); ++i )
    {
        CORE::CString fileName = CORE::ExtractFilename( files[ i ].path );
        if ( fileName != wanted )
            continue;

        CORE::CDynamicBuffer content;
        if ( !ReadWholeFile( files[ i ].path, content ) )
            continue;
        CRpcContractHash actual = CRpcContractHash::Compute( content.GetConstBufferPtr(), content.GetDataSize() );
        CORE::CString error;
        CRpcContractPtr contract;
        if ( actual == hash )
        {
            CORE::CDynamicBuffer docs;
            CORE::CString docsPath = files[ i ].path.CutChars( 4, false ) + ".docs.bin";
            ReadWholeFile( docsPath, docs );
            contract = CRpcContract::FromSchema( content.GetConstBufferPtr(), content.GetDataSize(), error,
                                                 docs.GetDataSize() > 0 ? docs.GetConstBufferPtr() : GUCEF_NULL, docs.GetDataSize() );
        }
        else
            error = "the content does not match its hash";
        if ( contract.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RpcContractRegistry: deleting cache file " << files[ i ].path << ": " << error );
            CORE::DeleteFile( files[ i ].path );
            return CRpcContractPtr();
        }

        // a hit counts as a use for the pruning order
        CORE::CResourceMetaData metaData;
        metaData.modifiedDateTime = CORE::CDateTime::NowUTCDateTime();
        metaData.hasModifiedDateTime = true;
        CORE::SetFileMetaData( files[ i ].path, metaData );

        MT::CScopeMutex lock( m_lock );
        TEntryMap::iterator e = m_entries.find( hash );
        if ( e != m_entries.end() )
            return (*e).second.contract;
        SEntry entry;
        entry.contract = contract;
        entry.source = CONTRACTSOURCE_CACHE;
        m_entries[ hash ] = entry;
        return contract;
    }
    return CRpcContractPtr();
}

/*-------------------------------------------------------------------------*/

void
CRpcContractRegistry::PruneCache( void ) const
{GUCEF_TRACE;

    if ( 0 == m_cacheMaxBytes )
        return;
    CORE::CString dir = GetCacheDirectory();
    GUCEF::vector< SCacheFile > files;
    ListCacheFiles( dir, 0, files );
    UInt64 total = 0;
    for ( UInt32 i=0; i<files.size(); ++i )
        total += files[ i ].size;
    while ( total > m_cacheMaxBytes && !files.empty() )
    {
        UInt32 oldest = 0;
        for ( UInt32 i=1; i<files.size(); ++i )
            if ( files[ i ].timestamp < files[ oldest ].timestamp )
                oldest = i;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RpcContractRegistry: pruning cache file " << files[ oldest ].path );
        CORE::DeleteFile( files[ oldest ].path );
        total -= files[ oldest ].size;
        files.erase( files.begin() + oldest );
    }
}

/*-------------------------------------------------------------------------*/

void
CRpcContractRegistry::SetCacheDirectory( const CORE::CString& dir )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cacheDir = dir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRpcContractRegistry::GetCacheDirectory( void ) const
{GUCEF_TRACE;

    CORE::CString dir;
    {
        MT::CScopeMutex lock( m_lock );
        dir = m_cacheDir;
    }
    return CORE::ResolveVars( dir );
}

/*-------------------------------------------------------------------------*/

void
CRpcContractRegistry::SetCacheMaxBytes( UInt64 maxBytes )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_cacheMaxBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CRpcContractRegistry::GetCacheMaxBytes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_cacheMaxBytes;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "cacheDirectory", m_cacheDir );
    config.SetAttribute( "cacheMaxBytes", m_cacheMaxBytes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcContractRegistry::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CVariant dir = config.GetAttributeValue( "cacheDirectory" );
    if ( !dir.IsNULLOrEmpty() )
        m_cacheDir = dir.AsString();
    CORE::CVariant maxBytes = config.GetAttributeValue( "cacheMaxBytes" );
    if ( !maxBytes.IsNULLOrEmpty() )
        m_cacheMaxBytes = maxBytes.AsUInt64( m_cacheMaxBytes );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcContractRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcContractRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CRpcContractEventData::CRpcContractEventData( const CRpcContractPtr& registered, CRpcContractRegistry::TContractSource from )
    : CORE::CICloneable()
    , contract( registered )
    , source( from )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcContractEventData::CRpcContractEventData( const CRpcContractEventData& src )
    : CORE::CICloneable( src )
    , contract( src.contract )
    , source( src.source )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRpcContractEventData::~CRpcContractEventData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CRpcContractEventData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CRpcContractEventData( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
