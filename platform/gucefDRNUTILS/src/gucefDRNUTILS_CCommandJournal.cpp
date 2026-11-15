/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CCommandJournal.h"

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSYSTEMUTCCLOCK_H
#include "gucefCORE_CSystemUtcClock.h"
#define GUCEF_CORE_CSYSTEMUTCCLOCK_H
#endif /* GUCEF_CORE_CSYSTEMUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CCommandJournal::KeySlot = "commandJournalKey";

/*-------------------------------------------------------------------------*/

namespace {

const UInt64 DayMs = 24 * 60 * 60 * 1000;

CORE::CString
EntryKey( const CORE::CString& subject, const CORE::CString& commandId )
{GUCEF_TRACE;

    return subject + "\n" + commandId;
}

/*-------------------------------------------------------------------------*/

bool
GetJsonCodec( CORE::CDStoreCodecRegistry::TDStoreCodecPtr& codec )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && !codec.IsNULL();
}

/*-------------------------------------------------------------------------*/

UInt64
NowUtcMs( void )
{GUCEF_TRACE;

    CORE::CSystemUtcClock clock;
    Int64 now = clock.GetUtcTimeInMs();
    return now > 0 ? (UInt64) now : 0;
}

}

/*-------------------------------------------------------------------------*/

CCommandJournal::SEntry::SEntry( void )
    : atUtcMs( 0 )
    , subject()
    , commandId()
    , requestHash()
    , method()
    , code( COMCORE::RPC_STATUS_OK )
    , detailCode()
    , outcome()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CCommandJournal::CCommandJournal( void )
    : m_lock()
    , m_file()
    , m_entries()
    , m_order()
    , m_nextOrder( 1 )
    , m_keepDays( DefaultKeepDays )
    , m_keepMonths( DefaultKeepMonths )
    , m_maxEntries( DefaultMaxEntries )
    , m_skippedLines( 0 )
    , m_open( false )
{GUCEF_TRACE;

    memset( m_key, 0, sizeof( m_key ) );
    if ( !CORE::CRandom::GetRandomBytes( m_key, sizeof( m_key ) ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CommandJournal: no randomness for the journal key" );
}

/*-------------------------------------------------------------------------*/

CCommandJournal::~CCommandJournal()
{GUCEF_TRACE;

    Close();
    memset( m_key, 0, sizeof( m_key ) );
}

/*-------------------------------------------------------------------------*/

void
CCommandJournal::SetRetention( UInt32 keepDays, UInt32 keepMonths, UInt32 maxEntries )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_keepDays = 0 == keepDays ? DefaultKeepDays : keepDays;
    m_keepMonths = keepMonths;
    m_maxEntries = 0 == maxEntries ? DefaultMaxEntries : maxEntries;
    m_file.SetKeepMonths( m_keepMonths );
}

/*-------------------------------------------------------------------------*/

void
CCommandJournal::Keep( const SEntry& entry )
{GUCEF_TRACE;

    CORE::CString key = EntryKey( entry.subject, entry.commandId );
    TEntryMap::iterator existing = m_entries.find( key );
    if ( existing != m_entries.end() )
    {
        // the newer entry of a command id replaces the older one
        TOrderMap::iterator o = m_order.begin();
        for ( ; o != m_order.end(); ++o )
        {
            if ( (*o).second == key )
            {
                m_order.erase( o );
                break;
            }
        }
    }
    m_entries[ key ] = entry;
    m_order[ m_nextOrder++ ] = key;
    while ( m_entries.size() > m_maxEntries && !m_order.empty() )
    {
        TOrderMap::iterator oldest = m_order.begin();
        m_entries.erase( (*oldest).second );
        m_order.erase( oldest );
    }
}

/*-------------------------------------------------------------------------*/

bool
CCommandJournal::Open( const CORE::CString& directory, CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_open = false;
    m_entries.clear();
    m_order.clear();
    m_skippedLines = 0;
    m_file.SetKeepMonths( m_keepMonths );
    if ( !m_file.SetDirectory( directory ) )
    {
        error = "the command journal directory " + directory + " cannot be used";
        return false;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }

    // the month files that can hold entries of the last keepDays days
    UInt32 months = m_keepDays / 28 + 2;
    CORE::CString::StringVector lines;
    if ( !m_file.ReadLines( months, lines ) )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CommandJournal: not every file of " << directory << " could be read" );

    const UInt64 now = NowUtcMs();
    const UInt64 cutoff = now > (UInt64) m_keepDays * DayMs ? now - (UInt64) m_keepDays * DayMs : 0;
    for ( UInt32 i=0; i<lines.size(); ++i )
    {
        const CORE::CString& line = lines[ i ];
        CORE::CDynamicBuffer buffer;
        buffer.Append( line.C_String(), line.ByteSize() > 0 ? line.ByteSize() - 1 : 0 );
        CORE::CDataNode node;
        bool parsed = false;
        {
            CORE::CDynamicBufferAccess access( buffer );
            parsed = codec->BuildDataTree( &node, &access );
        }
        const CORE::CDataNode* root = &node;
        if ( parsed && node.GetAttributeValueOrChildValueByName( "commandId" ).AsString().IsNULLOrEmpty() && 1 == node.GetNrOfChildNodes() )
            root = *node.ConstBegin();
        SEntry entry;
        if ( parsed )
        {
            entry.atUtcMs = root->GetAttributeValueOrChildValueByName( "atUtcMs" ).AsUInt64( 0 );
            entry.subject = root->GetAttributeValueOrChildValueByName( "subject" ).AsString();
            entry.commandId = root->GetAttributeValueOrChildValueByName( "commandId" ).AsString();
            entry.requestHash = root->GetAttributeValueOrChildValueByName( "requestHash" ).AsString();
            entry.method = root->GetAttributeValueOrChildValueByName( "method" ).AsString();
            entry.code = (COMCORE::TRpcStatusCode) root->GetAttributeValueOrChildValueByName( "code" ).AsUInt32( 0 );
            entry.detailCode = root->GetAttributeValueOrChildValueByName( "detailCode" ).AsString();
            const CORE::CDataNode* outcome = root->FindChild( "outcome" );
            if ( GUCEF_NULL != outcome )
                entry.outcome.Copy( *outcome );
        }
        if ( !parsed || entry.subject.IsNULLOrEmpty() || entry.commandId.IsNULLOrEmpty() )
        {
            ++m_skippedLines;
            continue;
        }
        if ( entry.atUtcMs < cutoff )
            continue;
        Keep( entry );
    }
    m_open = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CommandJournal: " << (UInt32) m_entries.size() << " commands of the last " << m_keepDays << " days in " << directory
                << ( m_skippedLines > 0 ? ", unreadable lines skipped: " : "" ) << ( m_skippedLines > 0 ? CORE::ToString( m_skippedLines ) : CORE::CString::Empty ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCommandJournal::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_open = false;
    m_entries.clear();
    m_order.clear();
}

/*-------------------------------------------------------------------------*/

bool
CCommandJournal::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CCommandJournal::UseKeyOfStore( AUTH::ICredentialStore& store, const CORE::CString& provider, CORE::CString& problem )
{GUCEF_TRACE;

    AUTH::CSecretString stored;
    UInt8 key[ 32 ];
    if ( store.Load( provider, KeySlot, stored ) && DRN::CDRNCrypto::FromHex( stored.Reveal(), key, sizeof( key ) ) )
    {
        SetKey( key, sizeof( key ) );
        memset( key, 0, sizeof( key ) );
        return true;
    }
    if ( !CORE::CRandom::GetRandomBytes( key, sizeof( key ) ) )
    {
        problem = "no randomness for a new journal key";
        return false;
    }
    AUTH::CSecretString fresh( DRN::CDRNCrypto::ToHex( key, sizeof( key ) ).Lowercase() );
    AUTH::CSecretString check;
    if ( !store.Store( provider, KeySlot, fresh ) || !store.Load( provider, KeySlot, check ) || check.Reveal() != fresh.Reveal() )
    {
        memset( key, 0, sizeof( key ) );
        problem = "the credential store did not keep the journal key; repeats after a restart are not recognised";
        return false;
    }
    SetKey( key, sizeof( key ) );
    memset( key, 0, sizeof( key ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCommandJournal::SetKey( const UInt8* key, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == key || 0 == size )
        return;
    UInt8 derived[ 32 ];
    if ( 32 == size )
        memcpy( derived, key, 32 );
    else
        CORE::CSha256::Hash( key, size, derived );
    MT::CScopeMutex lock( m_lock );
    memcpy( m_key, derived, 32 );
    memset( derived, 0, sizeof( derived ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCommandJournal::HashRequest( const COMCORE::CRpcMessage& request, UInt32 commandIdField ) const
{GUCEF_TRACE;

    COMCORE::CRpcMessage canonical( request );
    if ( 0 != commandIdField )
        canonical.Clear( commandIdField );
    CORE::CDynamicBuffer encoded;
    if ( !canonical.Encode( encoded ) )
        return CORE::CString::Empty;
    UInt8 digest[ CORE::CHmacSha256::MacSize ];
    {
        MT::CScopeMutex lock( m_lock );
        CORE::CHmacSha256::Mac( m_key, sizeof( m_key ), encoded.GetConstBufferPtr(), encoded.GetDataSize(), digest );
    }
    encoded.Clear( false );
    return DRN::CDRNCrypto::ToHex( digest, sizeof( digest ) ).Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CCommandJournal::Record( const SEntry& entry, CORE::CString& error )
{GUCEF_TRACE;

    SEntry kept = entry;
    if ( 0 == kept.atUtcMs )
        kept.atUtcMs = NowUtcMs();

    CORE::CDataNode node( "command", GUCEF_DATATYPE_OBJECT );
    node.SetAttribute( "atUtcMs", kept.atUtcMs );
    node.SetAttribute( "subject", kept.subject );
    node.SetAttribute( "commandId", kept.commandId );
    node.SetAttribute( "requestHash", kept.requestHash );
    node.SetAttribute( "method", kept.method );
    node.SetAttribute( "code", (UInt32) kept.code );
    node.SetAttribute( "detailCode", kept.detailCode );
    CORE::CDataNode* outcome = node.AddChild( "outcome", GUCEF_DATATYPE_OBJECT );
    outcome->Copy( kept.outcome );
    outcome->SetName( "outcome" );
    if ( GUCEF_DATATYPE_OBJECT != outcome->GetNodeType() && GUCEF_DATATYPE_ARRAY != outcome->GetNodeType() )
        outcome->SetNodeType( GUCEF_DATATYPE_OBJECT );

    MT::CScopeMutex lock( m_lock );
    Keep( kept );
    if ( !m_open )
    {
        error = "the command journal is not open";
        return false;
    }
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !GetJsonCodec( codec ) )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->StoreDataTree( &node, &access ) )
        {
            error = "the journal entry cannot be encoded";
            return false;
        }
    }
    // one entry is one line: line breaks between JSON tokens carry no meaning, those within strings are escaped
    CORE::CString line;
    line.Set( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
    line = line.ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' ).Trim( true ).Trim( false );
    return m_file.Append( line, error );
}

/*-------------------------------------------------------------------------*/

CCommandOutcomeCache::ELookup
CCommandJournal::Lookup( const CORE::CString& subject, const CORE::CString& commandId,
                         const CORE::CString& requestHash, SEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::const_iterator i = m_entries.find( EntryKey( subject, commandId ) );
    if ( i == m_entries.end() )
        return CCommandOutcomeCache::COMMAND_NEW;
    const SEntry& found = (*i).second;
    if ( !requestHash.IsNULLOrEmpty() && !found.requestHash.IsNULLOrEmpty() &&
         ( requestHash.Length() != found.requestHash.Length() ||
           !CORE::CSha256::ConstantTimeEquals( requestHash.C_String(), found.requestHash.C_String(), requestHash.ByteSize() - 1 ) ) )
        return CCommandOutcomeCache::COMMAND_REUSED;
    entry = found;
    return CCommandOutcomeCache::COMMAND_REPLAY;
}

/*-------------------------------------------------------------------------*/

UInt32
CCommandJournal::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CCommandJournal::GetSkippedLineCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_skippedLines;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
