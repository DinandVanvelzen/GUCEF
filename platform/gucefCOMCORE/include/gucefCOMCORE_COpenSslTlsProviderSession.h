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

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H
#define GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_ITLSSESSION_H
#include "gucefCOMCORE_ITlsSession.h"
#define GUCEF_COMCORE_ITLSSESSION_H
#endif /* GUCEF_COMCORE_ITLSSESSION_H ? */

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H
#include "gucefCOMCORE_COpenSslTlsProviderAbi.h"
#define GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H
#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the sessions of one OpenSSL provider share: the loaded entry points, the contexts that hold the trust
 *  anchors and options per distinct settings, and the sessions kept for resumption. Shared (reference counted)
 *  by the provider and its sessions so a session never outlives what it needs.
 *
 *  Thread safety: contexts are made and looked up under a lock and used without one: OpenSSL 1.1.0 and later
 *  allow that and the sessions made from a context are used by one thread each. The error queue of OpenSSL is
 *  per thread: every call into the library is preceded by ERR_clear_error() and the queue is read on the same
 *  thread by the session.
 */
class GUCEF_COMCORE_EXPORT_CPP COpenSslTlsProviderState
{
    public:

    SOpenSslApi api;                /**< valid once the provider probed successfully, never changes after that */
    CORE::CString providerName;
    CORE::CString versionInfo;
    Int64 verifyTimeOverride;       /**< test hook: unix time the certificates are verified at, 0 for the clock */

    COpenSslTlsProviderState( void );
    ~COpenSslTlsProviderState();

    /**
     *  The context for the settings, made when it is first needed and kept for the life of the provider: making
     *  one costs milliseconds (the trust store is read) and the sessions for resumption belong to it.
     *  @return NULL with the error set when the settings cannot be used
     */
    void* AcquireContext( const CTlsSettings& settings, CTlsError& error );

    /** What identifies the context for the settings: the key of the settings and what the trust of the system takes from the environment (SSL_CERT_FILE, SSL_CERT_DIR) */
    static CORE::CString GetContextKey( const CTlsSettings& settings );

    /** Takes the session kept under the key out of the cache, NULL when there is none. The caller frees it or hands it to a connection */
    void* TakeResumableSession( const CORE::CString& key );

    /** Keeps a session (a reference the caller gives away) for a later connection, the oldest ones fall out beyond maxEntries */
    void StoreResumableSession( const CORE::CString& key, void* session, UInt32 maxEntries );

    UInt32 GetContextCount( void );

    UInt32 GetResumableSessionCount( void );

    /** The text of the error queue of the calling thread (empties it), the entries joined with " | " */
    CORE::CString CollectErrors( void ) const;

    private:

    COpenSslTlsProviderState( const COpenSslTlsProviderState& src );
    COpenSslTlsProviderState& operator=( const COpenSslTlsProviderState& src );

    void* CreateContext( const CTlsSettings& settings, CTlsError& error );
    bool LoadSystemTrust( void* ctx, CTlsError& error );

    private:

    typedef GUCEF::map< CORE::CString, void* > TVoidPtrMap;
    typedef GUCEF::vector< CORE::CString > TStringVector;

    MT::CMutex m_lock;
    TVoidPtrMap m_contexts;
    TVoidPtrMap m_resumable;
    TStringVector m_resumableOrder;     /**< oldest first */
};

typedef CORE::CTSharedPtr< COpenSslTlsProviderState, MT::CMutex > TOpenSslStatePtr;

/*-------------------------------------------------------------------------*/

/**
 *  A client connection of the OpenSSL provider: the SSL object with two memory BIOs, ciphertext in and out
 *  through them, so it works for any segmentation of the stream and never touches a socket.
 */
class GUCEF_COMCORE_EXPORT_CPP COpenSslTlsProviderSession : public ITlsSession
{
    public:

    COpenSslTlsProviderSession( const TOpenSslStatePtr& state       ,
                                const CTlsSettings& settings        ,
                                const CORE::CString& serverName     );

    virtual ~COpenSslTlsProviderSession() GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the SSL object. @return false with the error set when that fails */
    bool Initialize( CTlsError& error );

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ETlsState PushCiphertext( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ETlsState PushPlaintext( const void* data, UInt32 size ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PullCiphertext( CORE::CDynamicBuffer& out ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool PullPlaintext( CORE::CDynamicBuffer& out ) GUCEF_VIRTUAL_OVERRIDE;

    virtual ETlsState GetState( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void InitiateClose( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CTlsError& GetError( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CTlsSessionInfo& GetInfo( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void Advance( void );
    void ReadApplicationData( void );
    void OnEstablished( void );
    void Fail( int sslErrorCode, const char* whileDoing );
    void FailWith( ETlsErrorClass errorClass, const CORE::CString& message );
    void ClassifyCertificateFailure( long verifyResult );
    CORE::CString GetCertificateDate( void* peerCert, bool notAfter );
    CORE::CString GetPeerSubject( void* peerCert );
    CORE::CString GetResumptionKey( void ) const;
    void KeepSessionForResumption( void );

    private:

    TOpenSslStatePtr m_shared;
    CTlsSettings m_settings;
    CORE::CString m_serverName;
    CORE::CString m_contextKey;
    bool m_isIpAddress;
    void* m_ssl;
    void* m_readBio;        /**< ciphertext from the peer goes in here, owned by m_ssl */
    void* m_writeBio;       /**< ciphertext for the peer comes out here, owned by m_ssl */
    ETlsState m_tlsState;
    CTlsError m_error;
    CTlsSessionInfo m_info;
    CORE::CDynamicBuffer m_plaintextOut;
    bool m_wasEstablished;
    bool m_shutdownSent;
    bool m_sessionKept;
    UInt64 m_startTicks;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDERSESSION_H ? */
