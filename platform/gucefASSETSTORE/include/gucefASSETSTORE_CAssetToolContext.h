/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#include "gucefCORE_CCommandIdempotencyCache.h"
#define GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H
#endif /* GUCEF_CORE_CCOMMANDIDEMPOTENCYCACHE_H ? */

#ifndef GUCEF_COM_CMCPCALLGATE_H
#include "gucefCOM_CMcpCallGate.h"
#define GUCEF_COM_CMCPCALLGATE_H
#endif /* GUCEF_COM_CMCPCALLGATE_H ? */

#ifndef GUCEF_COM_CMCPJOBBRIDGE_H
#include "gucefCOM_CMcpJobBridge.h"
#define GUCEF_COM_CMCPJOBBRIDGE_H
#endif /* GUCEF_COM_CMCPJOBBRIDGE_H ? */

#ifndef GUCEF_COM_CMCPTOOLRESULT_H
#include "gucefCOM_CMcpToolResult.h"
#define GUCEF_COM_CMCPTOOLRESULT_H
#endif /* GUCEF_COM_CMCPTOOLRESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H
#include "gucefASSETSTORE_CAssetToolSetSettings.h"
#define GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLSETSETTINGS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H
#include "gucefASSETSTORE_CAssetToolsExposureControl.h"
#define GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLSEXPOSURECONTROL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H
#include "gucefASSETSTORE_CAssetToolsRedaction.h"
#define GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H
#include "gucefASSETSTORE_CAssetContentExposureGate.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTEXPOSUREGATE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM { class CMcpServer; }
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the tools, resources and the policy of one asset tool set share (CAssetToolSet owns it): the service they
 *  work on (CIAssetStoreService, nothing else), the settings, the exposure in force, the output filter, the content
 *  gate, the call gate, the job bridge, the outcomes of writes by request key, the quotas of writes and jobs per
 *  principal, the previews the tool layer keeps itself (store_classify, store_edit_rules), and the server for the
 *  resource update notifications. Thread safe.
 *
 *  FinishResult() is the fixed end of every result (section 10.5): the content blocks (images, audio, embedded
 *  resources) without a tag of the pack they show are dropped and logged as a defect, tagged blocks pass the gate
 *  again, every text is redacted, the structured result is bounded at maxResultBytes (lists cut with truncated) and
 *  its serialized JSON is put in front as the text block.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolContext
{
    public:

    /** The pack a content block shows, for the final filter */
    struct GUCEF_ASSETSTORE_PUBLIC_CPP SContentTag
    {
        UInt32 contentIndex;
        CORE::CString converterPackId;
        CORE::CString storePackId;
        CORE::CString storeAssetId;
        CORE::CString stage;

        SContentTag( void );
    };
    typedef GUCEF::vector< SContentTag > TContentTagVector;

    CAssetToolContext( const TAssetStoreServicePtr& service, const CAssetToolSetSettings& settings );

    ~CAssetToolContext();

    CIAssetStoreService& GetService( void );

    const TAssetStoreServicePtr& GetServicePtr( void ) const;

    const CAssetToolSetSettings& GetSettings( void ) const;

    CAssetToolsExposureControl& GetExposureControl( void );

    CAssetToolsExposure GetExposure( void ) const;

    CAssetToolsRedaction& GetRedaction( void );

    const CAssetContentExposureGate& GetGate( void ) const;

    COM::CMcpCallGate& GetCallGate( void );

    void SetJobBridge( COM::CMcpJobBridge* bridge );

    COM::CMcpJobBridge* GetJobBridge( void ) const;

    void SetServer( COM::CMcpServer* server );

    /** Tells subscribers that a resource changed (manifest, report, job result); nothing without a server */
    void NotifyResourceUpdated( const CORE::CString& uri );

    /** The entries of a page: the limit asked for, the default when none, capped by the settings and the exposure */
    UInt32 GetPageLimit( UInt32 asked ) const;

    UInt32 GetUntrustedTextChars( void ) const;

    /** The command id of the service for a request key of a principal and a tool: stable, does not show the subject */
    static CORE::CString MakeCommandId( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey );

    /** A digest of the arguments of a call, without the members that do not change what it does */
    static CORE::CString DigestArguments( const CORE::CDataNode& arguments );

    /** The hex SHA-256 of a text */
    static CORE::CString Sha256Hex( const CORE::CString& text );

    /** An opaque handle: 22 characters of base64url over 128 random bits; empty when no random bytes are there */
    static CORE::CString MakeHandle( void );

    /** The outcome of an earlier write with the request key: HIT gives it, CONFLICT when the key came with other arguments */
    CORE::TCommandIdempotencyLookup FindWrite( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey,
                                               const CORE::CString& digest, CORE::CDataNode& outcome );

    void StoreWrite( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& requestKey,
                     const CORE::CString& digest, const CORE::CDataNode& outcome );

    /** Takes one applied write of the hourly quota of the principal; false with the time the next one is allowed */
    bool TakeWriteQuota( const CORE::CString& subject, CORE::CString& nextAllowed );

    /** Takes one job of the daily quota of the principal */
    bool TakeJobQuota( const CORE::CString& subject, CORE::CString& nextAllowed );

    /** A preview the tool layer keeps (single use, bound to the principal, the tool and the arguments digest) */
    CORE::CString AddPendingChange( const CORE::CString& subject, const CORE::CString& tool, const CORE::CString& digest,
                                    const CORE::CDataNode& payload, Int64& expiresUtcMs );

    /** Takes a preview for its apply; false (and the preview stays, unless it expired) with why */
    bool TakePendingChange( const CORE::CString& token, const CORE::CString& subject, const CORE::CString& tool,
                            const CORE::CString& digest, CORE::CDataNode& payload, CORE::CString& problem );

    /** The pack of a job of the bridge, for the job objects (kept in memory, bounded) */
    void RememberJobPack( const CORE::CString& jobId, const CORE::CString& packId );

    CORE::CString GetJobPack( const CORE::CString& jobId ) const;

    /** A nonce of a confirmation form may be used once; false when it was used */
    bool UseConfirmationNonce( const CORE::CString& nonce );

    /** The fixed end of a result, see the class description */
    void FinishResult( COM::CMcpToolResult& result, const TContentTagVector& tags );

    /** A tool execution error { "error": { code, message, retryable } } with the message redacted */
    void SetError( COM::CMcpToolResult& result, const CORE::CString& code, const CORE::CString& message, bool retryable );

    /**
     *  The error of a service answer: the errorCode of the document when it has one, else one made from the status
     *  (NOT_FOUND: not-found, INVALID_ARGUMENT: invalid-arguments, UNSUPPORTED: unsupported, UNAVAILABLE and the
     *  network statuses: backend-unavailable, REFUSED: licence-refused, NO_SPACE: no-space, NOT_SIGNED_IN: not-signed-in, ...)
     */
    void SetServiceError( COM::CMcpToolResult& result, TAssetStoreStatus status, const CORE::CString& message,
                          const CORE::CDataNode* document );

    static CORE::CString GetErrorCodeOfStatus( TAssetStoreStatus status );

    static Int64 NowUtcMs( void );

    private:

    CAssetToolContext( const CAssetToolContext& src );              /**< not implemented */
    CAssetToolContext& operator=( const CAssetToolContext& src );   /**< not implemented */

    struct SPending
    {
        CORE::CString subject;
        CORE::CString tool;
        CORE::CString digest;
        CORE::CDataNode payload;
        Int64 expiresUtcMs;
    };
    typedef GUCEF::map< CORE::CString, SPending > TPendingMap;
    typedef GUCEF::vector< Int64 > TTimeVector;
    typedef GUCEF::map< CORE::CString, TTimeVector > TQuotaMap;

    bool TakeQuota( TQuotaMap& quotas, const CORE::CString& subject, UInt32 maximum, Int64 windowMs, CORE::CString& nextAllowed );

    void BoundResult( CORE::CDataNode& structured ) const;

    TAssetStoreServicePtr m_service;
    CAssetToolSetSettings m_settings;
    CAssetToolsExposureControl m_exposure;
    CAssetToolsRedaction m_redaction;
    CAssetContentExposureGate m_gate;
    COM::CMcpCallGate m_callGate;
    COM::CMcpJobBridge* m_bridge;
    COM::CMcpServer* m_server;
    mutable MT::CMutex m_lock;
    CORE::CCommandIdempotencyCache m_writes;
    TQuotaMap m_writeQuotas;
    TQuotaMap m_jobQuotas;
    TPendingMap m_pending;
    CORE::CString::StringMap m_jobPacks;
    CORE::CString::StringVector m_jobPackOrder;
    CORE::CString::StringSet m_usedNonces;
    CORE::CString::StringVector m_nonceOrder;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */
