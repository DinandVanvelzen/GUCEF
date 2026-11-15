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

#ifndef GUCEF_DRNUTILS_CCOMMANDJOURNAL_H
#define GUCEF_DRNUTILS_CCOMMANDJOURNAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#include "gucefCORE_CMonthlyJsonLinesFile.h"
#define GUCEF_CORE_CMONTHLYJSONLINESFILE_H
#endif /* GUCEF_CORE_CMONTHLYJSONLINESFILE_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#include "gucefDRNUTILS_CCommandOutcomeCache.h"
#define GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcomes of mutations on storage, so that a repeat after a restart of the service is answered with what was
 *  done instead of being done again: "<directory>/<yyyy-mm>.jsonl" (CORE::CMonthlyJsonLinesFile), one line per command
 *  that may have changed data: { atUtcMs, subject, commandId, requestHash, method, code, detailCode, outcome }. The
 *  outcome is a small document the service chooses (ids, revisions); it never holds a payload or a secret.
 *  The request hash is HMAC-SHA256 with the journal key, a persistent secret (UseKeyOfStore(): a slot of the service's
 *  credential store, made at the first start), so the hashes are the same after a restart and tell nothing without the
 *  key. Without a persistent key the key is random per process (a repeat after a restart is then not recognised).
 *  Open() reads the entries of the last keepDays days into memory (bounded by maxEntries, newest kept). Needs the JSON
 *  codec (dstorepluginJSONPARSER). Thread safe.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CCommandJournal
{
    public:

    static const UInt32 DefaultKeepDays = 7;
    static const UInt32 DefaultKeepMonths = 3;
    static const UInt32 DefaultMaxEntries = 100000;
    static const char* const KeySlot;           /**< "commandJournalKey" */

    struct GUCEF_DRNUTILS_PUBLIC_CPP SEntry
    {
        UInt64 atUtcMs;
        CORE::CString subject;
        CORE::CString commandId;
        CORE::CString requestHash;
        CORE::CString method;               /**< "contract.Service/Method" */
        COMCORE::TRpcStatusCode code;       /**< what was answered */
        CORE::CString detailCode;
        CORE::CDataNode outcome;

        SEntry( void );
    };

    CCommandJournal( void );
    ~CCommandJournal();

    /** 'keepDays' 0: DefaultKeepDays; 'keepMonths' of files 0: every file */
    void SetRetention( UInt32 keepDays, UInt32 keepMonths, UInt32 maxEntries );

    /** Uses the directory (made when missing) and reads the entries of the last keepDays days; false when it cannot be used */
    bool Open( const CORE::CString& directory, CORE::CString& error );
    void Close( void );
    bool IsOpen( void ) const;

    /** The journal key from the store's slot (provider, KeySlot), made and stored when missing; false with the reason (the key is then random) */
    bool UseKeyOfStore( AUTH::ICredentialStore& store, const CORE::CString& provider, CORE::CString& problem );

    /** A given key (32 bytes) */
    void SetKey( const UInt8* key, UInt32 size );

    /** The keyed hash (hexadecimal) of a request's encoding with the command id field cleared (0: nothing cleared) */
    CORE::CString HashRequest( const COMCORE::CRpcMessage& request, UInt32 commandIdField ) const;

    /** Appends the entry and keeps it in memory; false when the line could not be written (it is kept in memory then) */
    bool Record( const SEntry& entry, CORE::CString& error );

    /** NEW: no entry; REPLAY: an entry with the same hash (in 'entry'); REUSED: an entry with another hash */
    CCommandOutcomeCache::ELookup Lookup( const CORE::CString& subject, const CORE::CString& commandId,
                                          const CORE::CString& requestHash, SEntry& entry ) const;

    UInt32 GetCount( void ) const;

    /** Lines that could not be read at Open() (skipped) */
    UInt32 GetSkippedLineCount( void ) const;

    private:

    typedef GUCEF::map< CORE::CString, SEntry > TEntryMap;     /**< key: subject, line feed, command id */
    typedef GUCEF::map< UInt64, CORE::CString > TOrderMap;

    CCommandJournal( const CCommandJournal& src );              /**< not implemented */
    CCommandJournal& operator=( const CCommandJournal& src );   /**< not implemented */

    void Keep( const SEntry& entry );

    mutable MT::CMutex m_lock;
    CORE::CMonthlyJsonLinesFile m_file;
    TEntryMap m_entries;
    TOrderMap m_order;
    UInt64 m_nextOrder;
    UInt8 m_key[ 32 ];
    UInt32 m_keepDays;
    UInt32 m_keepMonths;
    UInt32 m_maxEntries;
    UInt32 m_skippedLines;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CCOMMANDJOURNAL_H ? */
