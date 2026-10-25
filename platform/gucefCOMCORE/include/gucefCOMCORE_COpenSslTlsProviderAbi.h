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

/*
 *  The part of the OpenSSL interface that the TLS provider of gucefCOMCORE uses, declared here by hand so that
 *  the build needs no OpenSSL development package and no OpenSSL header: the library of the system is loaded when
 *  TLS is first wanted (see gucefCOMCORE_COpenSslTlsProvider.h) and the entry points are looked up by name in
 *  a table, the way CX11Display and the Wayland runtime do it.
 *
 *  Only functions that are real symbols in OpenSSL 1.1.1 and in 3.x are used. The macros of the OpenSSL headers
 *  (SSL_CTX_set_min_proto_version, SSL_set_tlsext_host_name, BIO_pending, ...) expand to calls of SSL_ctrl,
 *  SSL_CTX_ctrl and BIO_ctrl with a command number: those numbers are below. The numbers and flags are part of
 *  the public, stable interface of the library; declaring them is not copying the library.
 *
 *  Two things differ between the versions and are handled by the table: SSL_get1_peer_certificate (3.x) is
 *  SSL_get_peer_certificate in 1.1.1, and SSL_CTX_set_options takes a 64 bit value in 3.x and an unsigned long
 *  in 1.1.1 (the same on LP64, not on 32 bit systems): SetOptions() calls it the way the loaded version declares it.
 */

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H
#define GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stddef.h>
#include <time.h>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* SSL_CTX_ctrl and SSL_ctrl commands */
static const int OSSL_SSL_CTRL_MODE = 33;
static const int OSSL_SSL_CTRL_SET_SESS_CACHE_SIZE = 42;
static const int OSSL_SSL_CTRL_SET_SESS_CACHE_MODE = 44;
static const int OSSL_SSL_CTRL_SET_TLSEXT_HOSTNAME = 55;
static const int OSSL_SSL_CTRL_SET_MIN_PROTO_VERSION = 123;
static const int OSSL_SSL_CTRL_SET_MAX_PROTO_VERSION = 124;
static const int OSSL_TLSEXT_NAMETYPE_host_name = 0;

/* BIO_ctrl commands */
static const int OSSL_BIO_CTRL_PENDING = 10;

/* SSL_CTX_set_mode */
static const long OSSL_SSL_MODE_ENABLE_PARTIAL_WRITE = 0x00000001L;
static const long OSSL_SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER = 0x00000002L;

/* SSL_CTX_set_session_cache_mode */
static const long OSSL_SSL_SESS_CACHE_OFF = 0x0000L;
static const long OSSL_SSL_SESS_CACHE_CLIENT = 0x0001L;
static const long OSSL_SSL_SESS_CACHE_NO_INTERNAL_STORE = 0x0200L;

/* SSL_CTX_set_options */
static const UInt64 OSSL_SSL_OP_NO_COMPRESSION = 0x00020000ULL;
static const UInt64 OSSL_SSL_OP_NO_RENEGOTIATION = 0x40000000ULL;

/* SSL_CTX_set_verify */
static const int OSSL_SSL_VERIFY_NONE = 0;
static const int OSSL_SSL_VERIFY_PEER = 1;

/* SSL_get_error */
static const int OSSL_SSL_ERROR_NONE = 0;
static const int OSSL_SSL_ERROR_SSL = 1;
static const int OSSL_SSL_ERROR_WANT_READ = 2;
static const int OSSL_SSL_ERROR_WANT_WRITE = 3;
static const int OSSL_SSL_ERROR_SYSCALL = 5;
static const int OSSL_SSL_ERROR_ZERO_RETURN = 6;

/* SSL_set_hostflags */
static const unsigned int OSSL_X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS = 0x4;
static const unsigned int OSSL_X509_CHECK_FLAG_NEVER_CHECK_SUBJECT = 0x20;

/* the protocol versions on the wire, the same as ETlsVersion */
static const int OSSL_TLS1_2_VERSION = 0x0303;
static const int OSSL_TLS1_3_VERSION = 0x0304;

/* NID of the subject alternative name extension */
static const int OSSL_NID_subject_alt_name = 85;

/* OpenSSL_version() */
static const int OSSL_OPENSSL_VERSION = 0;

/* results of the verification of a certificate chain (X509_V_ERR_*) */
static const long OSSL_X509_V_OK = 0;
static const long OSSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT = 2;
static const long OSSL_X509_V_ERR_CERT_NOT_YET_VALID = 9;
static const long OSSL_X509_V_ERR_CERT_HAS_EXPIRED = 10;
static const long OSSL_X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT = 18;
static const long OSSL_X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN = 19;
static const long OSSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY = 20;
static const long OSSL_X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE = 21;
static const long OSSL_X509_V_ERR_CERT_REVOKED = 23;
static const long OSSL_X509_V_ERR_HOSTNAME_MISMATCH = 62;
static const long OSSL_X509_V_ERR_EMAIL_MISMATCH = 63;
static const long OSSL_X509_V_ERR_IP_ADDRESS_MISMATCH = 64;

/** the oldest version of the library that is used: OpenSSL 1.1.1, which is the first with TLS 1.3 */
static const unsigned long OSSL_MIN_VERSION_NUMBER = 0x10101000UL;

/** from this version on the library is OpenSSL 3 */
static const unsigned long OSSL_VERSION_NUMBER_3 = 0x30000000UL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The entry points. Every object is an opaque pointer, the types of the library are never needed.
 *  Members are NULL until Resolve() found them, see the list of required and optional ones there.
 */
struct GUCEF_COMCORE_EXPORT_CPP SOpenSslApi
{
    // libcrypto
    unsigned long ( *OpenSSL_version_num )( void );
    const char* ( *OpenSSL_version )( int type );
    unsigned long ( *ERR_get_error )( void );
    void ( *ERR_error_string_n )( unsigned long e, char* buf, size_t len );
    void ( *ERR_clear_error )( void );
    const void* ( *BIO_s_mem )( void );
    void* ( *BIO_new )( const void* method );
    int ( *BIO_free )( void* bio );
    int ( *BIO_read )( void* bio, void* data, int dlen );
    int ( *BIO_write )( void* bio, const void* data, int dlen );
    long ( *BIO_ctrl )( void* bio, int cmd, long larg, void* parg );
    void* ( *BIO_new_mem_buf )( const void* buf, int len );
    void* ( *PEM_read_bio_X509 )( void* bio, void** x, void* cb, void* u );
    int ( *X509_STORE_add_cert )( void* store, void* x509 );
    void ( *X509_free )( void* x509 );
    void* ( *X509_get_subject_name )( const void* x509 );
    char* ( *X509_NAME_oneline )( const void* name, char* buf, int size );
    const char* ( *X509_verify_cert_error_string )( long n );
    int ( *X509_VERIFY_PARAM_set1_ip_asc )( void* param, const char* ipasc );

    // libcrypto, optional
    int ( *OPENSSL_sk_num )( const void* stack );
    void* ( *OPENSSL_sk_value )( const void* stack, int index );
    void ( *X509_VERIFY_PARAM_set_time )( void* param, time_t t );
    const void* ( *X509_get0_notAfter )( const void* x509 );
    const void* ( *X509_get0_notBefore )( const void* x509 );
    int ( *ASN1_TIME_print )( void* bio, const void* time );
    int ( *X509_get_ext_by_NID )( const void* x509, int nid, int lastpos );
    const char* ( *X509_get_default_cert_file )( void );
    const char* ( *X509_get_default_cert_dir )( void );

    // libssl
    int ( *OPENSSL_init_ssl )( UInt64 opts, const void* settings );
    const void* ( *TLS_client_method )( void );
    void* ( *SSL_CTX_new )( const void* method );
    void ( *SSL_CTX_free )( void* ctx );
    long ( *SSL_CTX_ctrl )( void* ctx, int cmd, long larg, void* parg );
    void ( *SSL_CTX_set_verify )( void* ctx, int mode, void* callback );
    int ( *SSL_CTX_set_default_verify_paths )( void* ctx );
    int ( *SSL_CTX_load_verify_locations )( void* ctx, const char* caFile, const char* caPath );
    void* ( *SSL_CTX_get_cert_store )( const void* ctx );
    int ( *SSL_CTX_set_alpn_protos )( void* ctx, const unsigned char* protos, unsigned int protosLen );
    void ( *SSL_CTX_set_security_level )( void* ctx, int level );
    void* ( *SSL_new )( void* ctx );
    void ( *SSL_free )( void* ssl );
    long ( *SSL_ctrl )( void* ssl, int cmd, long larg, void* parg );
    void ( *SSL_set_bio )( void* ssl, void* rbio, void* wbio );
    void ( *SSL_set_connect_state )( void* ssl );
    int ( *SSL_do_handshake )( void* ssl );
    int ( *SSL_read )( void* ssl, void* buf, int num );
    int ( *SSL_write )( void* ssl, const void* buf, int num );
    int ( *SSL_shutdown )( void* ssl );
    int ( *SSL_get_error )( const void* ssl, int ret );
    int ( *SSL_set1_host )( void* ssl, const char* hostname );
    void ( *SSL_set_hostflags )( void* ssl, unsigned int flags );
    void* ( *SSL_get0_param )( void* ssl );
    long ( *SSL_get_verify_result )( const void* ssl );
    const char* ( *SSL_get_version )( const void* ssl );
    const void* ( *SSL_get_current_cipher )( const void* ssl );
    const char* ( *SSL_CIPHER_get_name )( const void* cipher );
    void ( *SSL_get0_alpn_selected )( const void* ssl, const unsigned char** data, unsigned int* len );
    void* ( *SSL_get_peer_cert_chain )( const void* ssl );
    void* ( *SSL_get1_session )( void* ssl );
    int ( *SSL_set_session )( void* ssl, void* session );
    int ( *SSL_session_reused )( const void* ssl );
    void ( *SSL_SESSION_free )( void* session );

    // libssl, optional
    int ( *SSL_SESSION_is_resumable )( const void* session );

    /**
     *  The peer certificate: SSL_get1_peer_certificate of 3.x, or SSL_get_peer_certificate of 1.1.1 which returns
     *  the same, a certificate the caller has to X509_free()
     */
    void* ( *PeerCertificate )( const void* ssl );

    /** SSL_CTX_set_options in the form of the loaded version, kept raw: use SetOptions() */
    void* SSL_CTX_set_options_raw;

    /** OpenSSL_version_num() of the loaded library, 0 when not loaded */
    unsigned long versionNumber;

    /** the libraries, kept for the life of the process: they are not unloaded */
    void* sslHandle;
    void* cryptoHandle;

    SOpenSslApi( void );

    /** Sets the options of a context the way the loaded version declares the function. @return the options now set */
    UInt64 SetOptions( void* ctx, UInt64 options ) const;

    /**
     *  Looks the entry points up in the two library handles. A missing required entry point makes it return false
     *  and missing lists the names. Optional entry points that are not there stay NULL.
     */
    bool Resolve( void* sslLib, void* cryptoLib, CORE::CString& missing );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDERABI_H ? */
