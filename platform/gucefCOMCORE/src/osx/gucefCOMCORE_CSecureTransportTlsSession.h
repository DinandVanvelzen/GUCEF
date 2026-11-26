/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_CSECURETRANSPORTTLSSESSION_H
#define GUCEF_COMCORE_CSECURETRANSPORTTLSSESSION_H

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

#ifndef GUCEF_COMCORE_CTLSSERVERSETTINGS_H
#include "gucefCOMCORE_CTlsServerSettings.h"
#define GUCEF_COMCORE_CTLSSERVERSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSERVERSETTINGS_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

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
 *  One TLS connection on Secure Transport: an SSLContext whose I/O functions read from and write to memory
 *  buffers, so that the session never touches a socket. See CSecureTransportTlsProvider.
 */
class CSecureTransportTlsSession : public ITlsSession
{
    public:

    /** Client side. anchors is a CFArrayRef of SecCertificateRef or NULL, retained by the session */
    CSecureTransportTlsSession( const CTlsSettings& settings    ,
                                const CORE::CString& serverName ,
                                void* anchors                   ,
                                Int64 verifyTimeOverride        );

    /** Server side. chain is a CFArrayRef: identity followed by the intermediate certificates, retained by the session */
    CSecureTransportTlsSession( const CTlsServerSettings& settings ,
                                void* chain                        );

    virtual ~CSecureTransportTlsSession() GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the SSLContext. @return false with the error filled in when the settings cannot be used */
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

    /** For the I/O functions of Secure Transport */
    Int32 OnRead( void* data, size_t* length );
    Int32 OnWrite( const void* data, size_t* length );

    private:

    CSecureTransportTlsSession( const CSecureTransportTlsSession& src );
    CSecureTransportTlsSession& operator=( const CSecureTransportTlsSession& src );

    void Advance( void );
    void ReadApplicationData( void );
    void OnEstablished( void );
    bool EvaluateServerTrust( void );
    void ClassifyTrustFailure( void* trust, const CORE::CString& reason );
    void FailWithStatus( Int32 status, const char* whileDoing );
    void FailWith( ETlsErrorClass errorClass, const CORE::CString& message );
    static CORE::CString DescribeStatus( Int32 status );

    private:

    static const UInt32 PlaintextReadBlockSize = 16 * 1024;

    CTlsSettings m_settings;
    CTlsServerSettings m_serverSettings;
    bool m_isServer;
    CORE::CString m_serverName;
    bool m_isIpAddress;
    void* m_anchors;                    /**< CFArrayRef or NULL */
    void* m_chain;                      /**< CFArrayRef or NULL */
    Int64 m_verifyTimeOverride;
    void* m_context;                    /**< SSLContextRef */
    CORE::CDynamicBuffer m_ciphertextIn;
    UInt32 m_ciphertextInPos;
    CORE::CDynamicBuffer m_ciphertextOut;
    CORE::CDynamicBuffer m_plaintextOut;
    ETlsState m_tlsState;
    CTlsError m_error;
    CTlsSessionInfo m_info;
    bool m_wasEstablished;
    bool m_closeSent;
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

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSECURETRANSPORTTLSSESSION_H ? */
