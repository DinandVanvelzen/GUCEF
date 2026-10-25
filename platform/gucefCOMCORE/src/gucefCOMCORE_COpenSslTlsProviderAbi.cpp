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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "gucefCOMCORE_COpenSslTlsProviderAbi.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef unsigned long ( *TSetOptions1 )( void* ctx, unsigned long options );
typedef UInt64 ( *TSetOptions3 )( void* ctx, UInt64 options );

/*-------------------------------------------------------------------------*/

/** Looks an entry point up, the name is added to the missing list when it is not there and is required */
static void*
Lookup( void* library, const char* name, bool required, CORE::CString& missing )
{GUCEF_TRACE;

    void* address = CORE::GetFunctionAddress( library, name, 0 ).objPtr;
    if ( GUCEF_NULL == address && required )
    {
        if ( !missing.IsNULLOrEmpty() )
            missing += ", ";
        missing += name;
    }
    return address;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SOpenSslApi::SOpenSslApi( void )
{GUCEF_TRACE;

    memset( this, 0, sizeof( SOpenSslApi ) );
}

/*-------------------------------------------------------------------------*/

UInt64
SOpenSslApi::SetOptions( void* ctx, UInt64 options ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == SSL_CTX_set_options_raw )
        return 0;

    // 3.x takes and returns a 64 bit value, 1.1.1 an unsigned long: the same on LP64 but not on 32 bit systems
    if ( versionNumber >= OSSL_VERSION_NUMBER_3 )
        return ( (TSetOptions3) SSL_CTX_set_options_raw )( ctx, options );
    return ( (TSetOptions1) SSL_CTX_set_options_raw )( ctx, (unsigned long) options );
}

/*-------------------------------------------------------------------------*/

bool
SOpenSslApi::Resolve( void* sslLib, void* cryptoLib, CORE::CString& missing )
{GUCEF_TRACE;

    missing.Clear();
    sslHandle = sslLib;
    cryptoHandle = cryptoLib;

    // libcrypto, required
    OpenSSL_version_num = (unsigned long ( * )( void )) Lookup( cryptoLib, "OpenSSL_version_num", true, missing );
    OpenSSL_version = (const char* ( * )( int )) Lookup( cryptoLib, "OpenSSL_version", true, missing );
    ERR_get_error = (unsigned long ( * )( void )) Lookup( cryptoLib, "ERR_get_error", true, missing );
    ERR_error_string_n = (void ( * )( unsigned long, char*, size_t )) Lookup( cryptoLib, "ERR_error_string_n", true, missing );
    ERR_clear_error = (void ( * )( void )) Lookup( cryptoLib, "ERR_clear_error", true, missing );
    BIO_s_mem = (const void* ( * )( void )) Lookup( cryptoLib, "BIO_s_mem", true, missing );
    BIO_new = (void* ( * )( const void* )) Lookup( cryptoLib, "BIO_new", true, missing );
    BIO_free = (int ( * )( void* )) Lookup( cryptoLib, "BIO_free", true, missing );
    BIO_read = (int ( * )( void*, void*, int )) Lookup( cryptoLib, "BIO_read", true, missing );
    BIO_write = (int ( * )( void*, const void*, int )) Lookup( cryptoLib, "BIO_write", true, missing );
    BIO_ctrl = (long ( * )( void*, int, long, void* )) Lookup( cryptoLib, "BIO_ctrl", true, missing );
    BIO_new_mem_buf = (void* ( * )( const void*, int )) Lookup( cryptoLib, "BIO_new_mem_buf", true, missing );
    PEM_read_bio_X509 = (void* ( * )( void*, void**, void*, void* )) Lookup( cryptoLib, "PEM_read_bio_X509", true, missing );
    X509_STORE_add_cert = (int ( * )( void*, void* )) Lookup( cryptoLib, "X509_STORE_add_cert", true, missing );
    X509_free = (void ( * )( void* )) Lookup( cryptoLib, "X509_free", true, missing );
    X509_get_subject_name = (void* ( * )( const void* )) Lookup( cryptoLib, "X509_get_subject_name", true, missing );
    X509_NAME_oneline = (char* ( * )( const void*, char*, int )) Lookup( cryptoLib, "X509_NAME_oneline", true, missing );
    X509_verify_cert_error_string = (const char* ( * )( long )) Lookup( cryptoLib, "X509_verify_cert_error_string", true, missing );
    X509_VERIFY_PARAM_set1_ip_asc = (int ( * )( void*, const char* )) Lookup( cryptoLib, "X509_VERIFY_PARAM_set1_ip_asc", true, missing );

    // libcrypto, optional: the message is less detailed or a test hook is not there without them
    OPENSSL_sk_num = (int ( * )( const void* )) Lookup( cryptoLib, "OPENSSL_sk_num", false, missing );
    OPENSSL_sk_value = (void* ( * )( const void*, int )) Lookup( cryptoLib, "OPENSSL_sk_value", false, missing );
    X509_VERIFY_PARAM_set_time = (void ( * )( void*, time_t )) Lookup( cryptoLib, "X509_VERIFY_PARAM_set_time", false, missing );
    X509_get0_notAfter = (const void* ( * )( const void* )) Lookup( cryptoLib, "X509_get0_notAfter", false, missing );
    X509_get0_notBefore = (const void* ( * )( const void* )) Lookup( cryptoLib, "X509_get0_notBefore", false, missing );
    ASN1_TIME_print = (int ( * )( void*, const void* )) Lookup( cryptoLib, "ASN1_TIME_print", false, missing );
    X509_get_ext_by_NID = (int ( * )( const void*, int, int )) Lookup( cryptoLib, "X509_get_ext_by_NID", false, missing );
    X509_get_default_cert_file = (const char* ( * )( void )) Lookup( cryptoLib, "X509_get_default_cert_file", false, missing );
    X509_get_default_cert_dir = (const char* ( * )( void )) Lookup( cryptoLib, "X509_get_default_cert_dir", false, missing );

    // libssl, required
    OPENSSL_init_ssl = (int ( * )( UInt64, const void* )) Lookup( sslLib, "OPENSSL_init_ssl", true, missing );
    TLS_client_method = (const void* ( * )( void )) Lookup( sslLib, "TLS_client_method", true, missing );
    SSL_CTX_new = (void* ( * )( const void* )) Lookup( sslLib, "SSL_CTX_new", true, missing );
    SSL_CTX_free = (void ( * )( void* )) Lookup( sslLib, "SSL_CTX_free", true, missing );
    SSL_CTX_ctrl = (long ( * )( void*, int, long, void* )) Lookup( sslLib, "SSL_CTX_ctrl", true, missing );
    SSL_CTX_set_options_raw = Lookup( sslLib, "SSL_CTX_set_options", true, missing );
    SSL_CTX_set_verify = (void ( * )( void*, int, void* )) Lookup( sslLib, "SSL_CTX_set_verify", true, missing );
    SSL_CTX_set_default_verify_paths = (int ( * )( void* )) Lookup( sslLib, "SSL_CTX_set_default_verify_paths", true, missing );
    SSL_CTX_load_verify_locations = (int ( * )( void*, const char*, const char* )) Lookup( sslLib, "SSL_CTX_load_verify_locations", true, missing );
    SSL_CTX_get_cert_store = (void* ( * )( const void* )) Lookup( sslLib, "SSL_CTX_get_cert_store", true, missing );
    SSL_CTX_set_alpn_protos = (int ( * )( void*, const unsigned char*, unsigned int )) Lookup( sslLib, "SSL_CTX_set_alpn_protos", true, missing );
    SSL_CTX_set_security_level = (void ( * )( void*, int )) Lookup( sslLib, "SSL_CTX_set_security_level", true, missing );
    SSL_new = (void* ( * )( void* )) Lookup( sslLib, "SSL_new", true, missing );
    SSL_free = (void ( * )( void* )) Lookup( sslLib, "SSL_free", true, missing );
    SSL_ctrl = (long ( * )( void*, int, long, void* )) Lookup( sslLib, "SSL_ctrl", true, missing );
    SSL_set_bio = (void ( * )( void*, void*, void* )) Lookup( sslLib, "SSL_set_bio", true, missing );
    SSL_set_connect_state = (void ( * )( void* )) Lookup( sslLib, "SSL_set_connect_state", true, missing );
    SSL_do_handshake = (int ( * )( void* )) Lookup( sslLib, "SSL_do_handshake", true, missing );
    SSL_read = (int ( * )( void*, void*, int )) Lookup( sslLib, "SSL_read", true, missing );
    SSL_write = (int ( * )( void*, const void*, int )) Lookup( sslLib, "SSL_write", true, missing );
    SSL_shutdown = (int ( * )( void* )) Lookup( sslLib, "SSL_shutdown", true, missing );
    SSL_get_error = (int ( * )( const void*, int )) Lookup( sslLib, "SSL_get_error", true, missing );
    SSL_set1_host = (int ( * )( void*, const char* )) Lookup( sslLib, "SSL_set1_host", true, missing );
    SSL_set_hostflags = (void ( * )( void*, unsigned int )) Lookup( sslLib, "SSL_set_hostflags", true, missing );
    SSL_get0_param = (void* ( * )( void* )) Lookup( sslLib, "SSL_get0_param", true, missing );
    SSL_get_verify_result = (long ( * )( const void* )) Lookup( sslLib, "SSL_get_verify_result", true, missing );
    SSL_get_version = (const char* ( * )( const void* )) Lookup( sslLib, "SSL_get_version", true, missing );
    SSL_get_current_cipher = (const void* ( * )( const void* )) Lookup( sslLib, "SSL_get_current_cipher", true, missing );
    SSL_CIPHER_get_name = (const char* ( * )( const void* )) Lookup( sslLib, "SSL_CIPHER_get_name", true, missing );
    SSL_get0_alpn_selected = (void ( * )( const void*, const unsigned char**, unsigned int* )) Lookup( sslLib, "SSL_get0_alpn_selected", true, missing );
    SSL_get_peer_cert_chain = (void* ( * )( const void* )) Lookup( sslLib, "SSL_get_peer_cert_chain", true, missing );
    SSL_get1_session = (void* ( * )( void* )) Lookup( sslLib, "SSL_get1_session", true, missing );
    SSL_set_session = (int ( * )( void*, void* )) Lookup( sslLib, "SSL_set_session", true, missing );
    SSL_session_reused = (int ( * )( const void* )) Lookup( sslLib, "SSL_session_reused", true, missing );
    SSL_SESSION_free = (void ( * )( void* )) Lookup( sslLib, "SSL_SESSION_free", true, missing );

    // libssl, optional
    SSL_SESSION_is_resumable = (int ( * )( const void* )) Lookup( sslLib, "SSL_SESSION_is_resumable", false, missing );

    // The one that was renamed: OpenSSL 3.x exports SSL_get1_peer_certificate (SSL_get_peer_certificate is only a macro
    // there), OpenSSL 1.1.1 exports SSL_get_peer_certificate. Both give a reference the caller frees with X509_free
    void* peerCertificate = Lookup( sslLib, "SSL_get1_peer_certificate", false, missing );
    if ( GUCEF_NULL == peerCertificate )
        peerCertificate = Lookup( sslLib, "SSL_get_peer_certificate", false, missing );
    if ( GUCEF_NULL == peerCertificate )
    {
        if ( !missing.IsNULLOrEmpty() )
            missing += ", ";
        missing += "SSL_get1_peer_certificate or SSL_get_peer_certificate";
    }
    PeerCertificate = (void* ( * )( const void* )) peerCertificate;

    if ( !missing.IsNULLOrEmpty() )
        return false;

    versionNumber = OpenSSL_version_num();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
