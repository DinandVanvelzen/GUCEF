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

#ifndef GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#define GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H

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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H
#include "gucefCORE_CTCommandIdempotencyCache.h"
#define GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

#ifndef GUCEF_COMCORE_CRPCSTATUS_H
#include "gucefCOMCORE_CRpcStatus.h"
#define GUCEF_COMCORE_CRPCSTATUS_H
#endif /* GUCEF_COMCORE_CRPCSTATUS_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

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
 *  The outcomes of mutations by command id (gucefASSETSTORE_RemoteNetworking_Design.md 4.4): a (subject, commandId) is
 *  carried out at most once; a repeat with the same request is answered with the outcome kept (replayed), a repeat with
 *  another request is refused (INVALID_ARGUMENT COMMAND_ID_REUSED). A request is known by a keyed hash: HMAC-SHA256
 *  with a random key of this cache, made at construction and never written anywhere, of the request's encoding without
 *  its command id; so a request that holds a secret is comparable without the secret being kept or being guessable.
 *  The bookkeeping (time to live, per subject and total bounds, oldest first) is CORE::CTCommandIdempotencyCache.
 *  Refusals that changed nothing should not be remembered. Kept in memory: CCommandJournal keeps outcomes across
 *  restarts. Thread safe.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CCommandOutcomeCache
{
    public:

    enum ELookup
    {
        COMMAND_NEW = 0 ,       /**< not seen: carry it out and Remember() the outcome */
        COMMAND_REPLAY ,        /**< seen with the same request: answer with the outcome kept */
        COMMAND_REUSED          /**< the command id was used for another request: refuse */
    };

    /** What was answered */
    struct GUCEF_DRNUTILS_PUBLIC_CPP SOutcome
    {
        COMCORE::CRpcStatus status;         /**< OK, or the failure that was answered */
        CORE::CDynamicBuffer response;      /**< the encoded response message when status is OK */
        bool hasResponse;

        SOutcome( void );
    };

    CCommandOutcomeCache( void );
    ~CCommandOutcomeCache();

    /** 0 = no bound for that dimension; defaults 24 h, 256 per subject, 4096 in total */
    void SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal );

    /** 1 to 64 characters of A-Z, a-z, 0-9, '.', '_' and '-' */
    static bool IsValidCommandId( const CORE::CString& commandId );

    /**
     *  The command id of a call: the string field of the request when it is set (commandIdField 0: none), else the RPC
     *  metadata "idempotency-key"; false when neither is there or it is not valid ('reason' says which)
     */
    static bool GetCommandId( const COMCORE::CRpcServerCall& call, UInt32 commandIdField, CORE::CString& commandId, CORE::CString& reason );

    /** The keyed hash (hexadecimal) of a request's encoding with the command id field cleared (0: nothing cleared) */
    CORE::CString HashRequest( const COMCORE::CRpcMessage& request, UInt32 commandIdField ) const;

    /** The keyed hash (hexadecimal) of canonical bytes */
    CORE::CString HashBytes( const void* data, UInt32 size ) const;

    ELookup Find( const CORE::CString& subject, const CORE::CString& commandId, const CORE::CString& requestHash,
                  UInt64 nowMs, SOutcome& outcome );

    void Remember( const CORE::CString& subject, const CORE::CString& commandId, const CORE::CString& requestHash,
                   UInt64 nowMs, const SOutcome& outcome );

    /** An outcome of an answer: the response (encoded) or the failure */
    static void MakeOutcome( const COMCORE::CRpcStatus& status, const COMCORE::CRpcMessage* response, SOutcome& outcome );

    /**
     *  Answers a call with a kept outcome: the response decoded into the call's response type with the bool field
     *  'replayedField' set true (0: none), or the failure. False when the call could not be answered
     */
    static bool AnswerReplay( COMCORE::CRpcServerCall& call, const SOutcome& outcome, UInt32 replayedField );

    /** Refuses a call with INVALID_ARGUMENT COMMAND_ID_REUSED */
    static bool RefuseReused( COMCORE::CRpcServerCall& call, const CORE::CString& commandId );

    UInt32 GetCount( void ) const;
    void Clear( void );

    private:

    CCommandOutcomeCache( const CCommandOutcomeCache& src );            /**< not implemented */
    CCommandOutcomeCache& operator=( const CCommandOutcomeCache& src ); /**< not implemented */

    CORE::CTCommandIdempotencyCache< SOutcome > m_cache;
    UInt8 m_key[ 32 ];
    bool m_hasKey;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H ? */
