/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_COsCryptoCodecs.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_alg.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CINSTANCEBASEDCODEC_H
#include "gucefCORE_CInstanceBasedCodec.h"
#define GUCEF_CORE_CINSTANCEBASEDCODEC_H
#endif /* GUCEF_CORE_CINSTANCEBASEDCODEC_H ? */

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#include "gucefCORE_CCodecInstanceBase.h"
#define GUCEF_CORE_CCODECINSTANCEBASE_H
#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */

#ifndef AF_ALG
  #define AF_ALG 38
#endif
#ifndef SOL_ALG
  #define SOL_ALG 279
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 AeadKeySize = 32;
static const UInt32 AeadNonceSize = 12;
static const UInt32 AeadTagSize = 16;
static const UInt32 MaxDigestSize = 64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Writes "<what>: <errno text>" into 'text' */
void
DescribeErrno( const char* what, int errorCode, char* text, size_t textSize )
{GUCEF_TRACE;

    char errorText[ 128 ];
    errorText[ 0 ] = '\0';
    #if defined( __GLIBC__ ) && defined( _GNU_SOURCE )
    const char* description = strerror_r( errorCode, errorText, sizeof( errorText ) );
    #else
    const char* description = errorText;
    if ( 0 != strerror_r( errorCode, errorText, sizeof( errorText ) ) )
        errorText[ 0 ] = '\0';
    #endif
    snprintf( text, textSize, "%s: %s (errno %d)", what, GUCEF_NULL != description ? description : "", errorCode );
}

/*-------------------------------------------------------------------------*/

/** A transformation socket of the kernel crypto API bound to an algorithm, -1 when the kernel does not have it */
int
OpenTransformSocket( const char* algType, const char* algName )
{GUCEF_TRACE;

    int fd = socket( AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0 );
    if ( fd < 0 )
        return -1;

    struct sockaddr_alg address;
    memset( &address, 0, sizeof( address ) );
    address.salg_family = AF_ALG;
    strncpy( (char*) address.salg_type, algType, sizeof( address.salg_type ) - 1 );
    strncpy( (char*) address.salg_name, algName, sizeof( address.salg_name ) - 1 );
    if ( 0 != bind( fd, (struct sockaddr*) &address, sizeof( address ) ) )
    {
        close( fd );
        return -1;
    }
    return fd;
}

/*-------------------------------------------------------------------------*/

/** Closes the descriptor when it is open */
void
CloseSocket( int& fd )
{GUCEF_TRACE;

    if ( fd >= 0 )
    {
        close( fd );
        fd = -1;
    }
}

/*-------------------------------------------------------------------------*/

/** Sends all of the data, more of the same message may follow when 'more' is set */
bool
SendAll( int fd, const void* data, UInt32 dataSize, bool more )
{GUCEF_TRACE;

    const UInt8* p = static_cast< const UInt8* >( data );
    do
    {
        ssize_t sent = send( fd, p, dataSize, more ? MSG_MORE : 0 );
        if ( sent < 0 )
        {
            if ( EINTR == errno )
                continue;
            return false;
        }
        if ( (size_t) sent > dataSize )
            return false;
        p += sent;
        dataSize -= (UInt32) sent;
    }
    while ( dataSize > 0 );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANCES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A hash or HMAC in progress in the kernel: the data goes to the operation socket with MSG_MORE,
 *  FINAL sends the end of the message and reads the digest
 */
class CAfAlgHashInstance : public CCodecInstanceBase
{
    public:

    CAfAlgHashInstance( int transformFd, int operationFd, UInt32 digestSize, bool hexOutput )
        : CCodecInstanceBase()
        , m_transformFd( transformFd )
        , m_operationFd( operationFd )
        , m_digestSize( digestSize )
        , m_hexOutput( hexOutput )
        , m_inMessage( false )
    {GUCEF_TRACE;
    }

    virtual ~CAfAlgHashInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CloseSocket( m_operationFd );
        CloseSocket( m_transformFd );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, false, encode ) )
            return false;

        UInt8 digest[ MaxDigestSize ];
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) && m_inMessage )
        {
            // an unfinished message is dropped: finish it and throw the digest away
            m_inMessage = false;
            if ( !SendAll( m_operationFd, GUCEF_NULL, 0, false ) || (ssize_t) m_digestSize != recv( m_operationFd, digest, m_digestSize, 0 ) )
                return SetErrno( "Unable to drop the unfinished message" );
        }

        if ( GUCEF_NULL != input && inputSize > 0 )
        {
            if ( !SendAll( m_operationFd, input, inputSize, true ) )
            {
                m_inMessage = false;
                return SetErrno( "Unable to hash the input" );
            }
            m_inMessage = true;
        }

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, m_hexOutput ? 2 * m_digestSize : m_digestSize, out ) )
                return false;

            m_inMessage = false;
            if ( !SendAll( m_operationFd, GUCEF_NULL, 0, false ) )
                return SetErrno( "Unable to finish the hash" );
            ssize_t received = recv( m_operationFd, digest, m_digestSize, 0 );
            if ( (ssize_t) m_digestSize != received )
                return SetErrno( "Unable to read the digest" );

            if ( m_hexOutput )
                CInstanceBasedCodec::AppendHex( digest, m_digestSize, output );
            else
                output.Append( digest, m_digestSize, true );
        }
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return 0;
    }

    private:

    bool SetErrno( const char* what )
    {GUCEF_TRACE;

        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        DescribeErrno( what, errno, text, sizeof( text ) );
        return SetStatus( GUCEF_CODEC_STATUS_FAILED, text );
    }

    int m_transformFd;
    int m_operationFd;
    UInt32 m_digestSize;
    bool m_hexOutput;
    bool m_inMessage;
};

/*-------------------------------------------------------------------------*/

/**
 *  An AEAD cipher with its key in the kernel. A message is one sendmsg() of the associated data and the input with
 *  the operation, nonce and associated data length as control messages, and one recvmsg() of the result, which the
 *  kernel gives with the associated data in front.
 */
class CAfAlgAeadInstance : public CCodecInstanceBase
{
    public:

    CAfAlgAeadInstance( int transformFd, int operationFd )
        : CCodecInstanceBase()
        , m_transformFd( transformFd )
        , m_operationFd( operationFd )
        , m_aadScratch()
    {GUCEF_TRACE;

        // room for the associated data the kernel copies back, so packet headers need no allocation later
        m_aadScratch.SetBufferSize( 256, false );
    }

    virtual ~CAfAlgAeadInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CloseSocket( m_operationFd );
        CloseSocket( m_transformFd );
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool encode = true;
        if ( !CheckDirection( flags, true, true, encode ) )
            return false;
        if ( ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) != ( flags & ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) ) )
            return SetStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, "An AEAD cipher takes a whole message per call: BEGIN and FINAL" );
        if ( GUCEF_NULL == args || args->structSize < sizeof( SCodecCallArgs ) || GUCEF_NULL == args->nonce )
            return SetStatus( GUCEF_CODEC_STATUS_MISSING_PARAMETER, "A nonce is required" );
        if ( AeadNonceSize != args->nonceSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The nonce must be 12 bytes" );
        if ( GUCEF_NULL == input && inputSize > 0 )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No input data" );
        if ( !encode && inputSize < AeadTagSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is shorter than the tag" );

        const UInt32 aadSize = GUCEF_NULL != args->aad ? args->aadSize : 0;
        const UInt32 outputSize = encode ? inputSize + AeadTagSize : inputSize - AeadTagSize;
        if ( encode && outputSize < inputSize )
            return SetStatus( GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );

        UInt8* out = GUCEF_NULL;
        if ( !ReserveOutput( output, outputSize, out ) )
            return false;
        if ( m_aadScratch.GetBufferSize() < aadSize )
        {
            m_aadScratch.SetBufferSize( aadSize, false );
            if ( m_aadScratch.GetBufferSize() < aadSize )
                return SetStatus( GUCEF_CODEC_STATUS_FAILED, "Unable to make room for the associated data" );
        }

        // the request: operation, nonce and associated data length as control messages
        union
        {
            char buffer[ CMSG_SPACE( sizeof( UInt32 ) ) + CMSG_SPACE( sizeof( struct af_alg_iv ) + AeadNonceSize ) + CMSG_SPACE( sizeof( UInt32 ) ) ];
            struct cmsghdr align;
        } control;
        memset( &control, 0, sizeof( control ) );

        struct iovec sendParts[ 2 ];
        sendParts[ 0 ].iov_base = const_cast< void* >( args->aad );
        sendParts[ 0 ].iov_len = aadSize;
        sendParts[ 1 ].iov_base = const_cast< void* >( input );
        sendParts[ 1 ].iov_len = inputSize;

        struct msghdr request;
        memset( &request, 0, sizeof( request ) );
        request.msg_control = control.buffer;
        request.msg_controllen = sizeof( control.buffer );
        request.msg_iov = sendParts;
        request.msg_iovlen = 2;

        struct cmsghdr* message = CMSG_FIRSTHDR( &request );
        message->cmsg_level = SOL_ALG;
        message->cmsg_type = ALG_SET_OP;
        message->cmsg_len = CMSG_LEN( sizeof( UInt32 ) );
        UInt32 operation = encode ? ALG_OP_ENCRYPT : ALG_OP_DECRYPT;
        memcpy( CMSG_DATA( message ), &operation, sizeof( operation ) );

        message = CMSG_NXTHDR( &request, message );
        message->cmsg_level = SOL_ALG;
        message->cmsg_type = ALG_SET_IV;
        message->cmsg_len = CMSG_LEN( sizeof( struct af_alg_iv ) + AeadNonceSize );
        struct af_alg_iv* iv = (struct af_alg_iv*) CMSG_DATA( message );
        iv->ivlen = AeadNonceSize;
        memcpy( iv->iv, args->nonce, AeadNonceSize );

        message = CMSG_NXTHDR( &request, message );
        message->cmsg_level = SOL_ALG;
        message->cmsg_type = ALG_SET_AEAD_ASSOCLEN;
        message->cmsg_len = CMSG_LEN( sizeof( UInt32 ) );
        memcpy( CMSG_DATA( message ), &aadSize, sizeof( aadSize ) );

        // without waiting: a message larger than the socket buffer is refused instead of blocking for a reader
        ssize_t sent = sendmsg( m_operationFd, &request, MSG_DONTWAIT );
        if ( sent < 0 || (size_t) sent != (size_t) aadSize + inputSize )
        {
            if ( sent >= 0 || EAGAIN == errno || EWOULDBLOCK == errno )
            {
                ResetOperationSocket();
                return SetStatus( GUCEF_CODEC_STATUS_INVALID_INPUT, "The message is larger than the kernel takes at once" );
            }
            int errorCode = errno;
            ResetOperationSocket();
            errno = errorCode;
            return SetErrno( GUCEF_CODEC_STATUS_FAILED, "Unable to pass the message to the kernel" );
        }

        // the result: the associated data again, then the output.
        // The kernel only runs the operation for a read with room: a message without associated data and without
        // plaintext (a lone tag) still gets a byte of room, or its tag would not even be checked
        UInt8 noOutput = 0;
        struct iovec receiveParts[ 2 ];
        receiveParts[ 0 ].iov_base = m_aadScratch.GetBufferPtr();
        receiveParts[ 0 ].iov_len = aadSize;
        receiveParts[ 1 ].iov_base = out;
        receiveParts[ 1 ].iov_len = outputSize;
        if ( 0 == aadSize + outputSize )
        {
            receiveParts[ 1 ].iov_base = &noOutput;
            receiveParts[ 1 ].iov_len = 1;
        }

        struct msghdr result;
        memset( &result, 0, sizeof( result ) );
        result.msg_iov = receiveParts;
        result.msg_iovlen = 2;

        ssize_t received = recvmsg( m_operationFd, &result, 0 );
        if ( received < 0 )
        {
            if ( EBADMSG == errno )
                return SetStatus( GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The tag does not match: the message is not authentic" );
            int errorCode = errno;
            ResetOperationSocket();
            errno = errorCode;
            return SetErrno( GUCEF_CODEC_STATUS_FAILED, "Unable to read the result from the kernel" );
        }
        if ( (size_t) received != (size_t) aadSize + outputSize )
        {
            ResetOperationSocket();
            return SetStatus( GUCEF_CODEC_STATUS_FAILED, "The kernel gave a result of an unexpected size" );
        }

        output.SetDataSize( output.GetDataSize() + outputSize );
        return SetOk();
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return AeadTagSize;
    }

    private:

    bool SetErrno( TCodecStatus status, const char* what )
    {GUCEF_TRACE;

        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        DescribeErrno( what, errno, text, sizeof( text ) );
        return SetStatus( status, text );
    }

    /** After a refused message the operation socket may hold part of it: a new one starts clean */
    void ResetOperationSocket( void )
    {GUCEF_TRACE;

        CloseSocket( m_operationFd );
        m_operationFd = accept( m_transformFd, GUCEF_NULL, 0 );
    }

    int m_transformFd;
    int m_operationFd;
    CDynamicBuffer m_aadScratch;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CODECS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A hash ("sha256", "sha512") or HMAC ("hmac-sha256") of the kernel crypto API */
class CAfAlgHashCodec : public CInstanceBasedCodec
{
    public:

    CAfAlgHashCodec( const CString& familyName, const CString& typeName, const char* algName, UInt32 digestSize, bool isMac )
        : CInstanceBasedCodec( familyName, typeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
        , m_algName( algName )
        , m_digestSize( digestSize )
        , m_isMac( isMac )
    {GUCEF_TRACE;
    }

    CAfAlgHashCodec( const CAfAlgHashCodec& src )
        : CInstanceBasedCodec( src )
        , m_algName( src.m_algName )
        , m_digestSize( src.m_digestSize )
        , m_isMac( src.m_isMac )
    {GUCEF_TRACE;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_isMac )
            info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key" ) );
        else
            info.push_back( CCodecParameterInfo( "output", GUCEF_DATATYPE_STRING, false, CVariant( "hex" ), "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" ) );
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CAfAlgHashCodec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CAfAlgHashCodec";
        return classTypeName;
    }

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params, CICodecInstancePtr& instance, CString& errorMessage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        bool hexOutput = false;
        const void* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( m_isMac )
        {
            if ( !TryGetBinaryParam( params, "key", key, keySize ) )
            {
                errorMessage = "The parameter \"key\" is required";
                return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
            }
        }
        else
        {
            bool isValid = true;
            hexOutput = IsHexOutputRequested( params, true, isValid );
            if ( !isValid )
            {
                errorMessage = "The parameter \"output\" must be \"hex\" or \"raw\"";
                return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
            }
        }

        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        int transformFd = OpenTransformSocket( "hash", m_algName );
        if ( transformFd < 0 )
        {
            DescribeErrno( "AF_ALG: the algorithm is not available", errno, text, sizeof( text ) );
            errorMessage = text;
            return GUCEF_CODEC_STATUS_FAILED;
        }
        if ( m_isMac && 0 != setsockopt( transformFd, SOL_ALG, ALG_SET_KEY, key, keySize ) )
        {
            DescribeErrno( "AF_ALG: the key was refused", errno, text, sizeof( text ) );
            errorMessage = text;
            CloseSocket( transformFd );
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }
        int operationFd = accept( transformFd, GUCEF_NULL, 0 );
        if ( operationFd < 0 )
        {
            DescribeErrno( "AF_ALG: unable to start an operation", errno, text, sizeof( text ) );
            errorMessage = text;
            CloseSocket( transformFd );
            return GUCEF_CODEC_STATUS_FAILED;
        }

        instance = CICodecInstancePtr( GUCEF_NEW CAfAlgHashInstance( transformFd, operationFd, m_digestSize, hexOutput ) );
        return GUCEF_CODEC_STATUS_OK;
    }

    private:

    const char* m_algName;
    UInt32 m_digestSize;
    bool m_isMac;
};

/*-------------------------------------------------------------------------*/

/** An AEAD cipher with a 32 byte key, a 12 byte nonce and a 16 byte tag ("aes-256-gcm", "chacha20-poly1305") of the kernel crypto API */
class CAfAlgAeadCodec : public CInstanceBasedCodec
{
    public:

    CAfAlgAeadCodec( const CString& typeName, const char* algName )
        : CInstanceBasedCodec( CoreCodecTypes::EncryptionCodec, typeName, GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC | GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY )
        , m_algName( algName )
    {GUCEF_TRACE;
    }

    CAfAlgAeadCodec( const CAfAlgAeadCodec& src )
        : CInstanceBasedCodec( src )
        , m_algName( src.m_algName )
    {GUCEF_TRACE;
    }

    virtual void GetParameterInfo( CCodecParameterInfoList& info ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        info.push_back( CCodecParameterInfo( "key", GUCEF_DATATYPE_BINARY_BLOB, true, CVariant(), "The secret key, 32 bytes" ) );
        info.push_back( CCodecParameterInfo( "nonce", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "12 bytes, never twice with the same key; required for one shot use, per call for instances" ) );
        info.push_back( CCodecParameterInfo( "aad", GUCEF_DATATYPE_BINARY_BLOB, false, CVariant(), "Associated data: authenticated, not encrypted" ) );
    }

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CAfAlgAeadCodec( *this );
    }

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CString classTypeName = "GUCEF::CORE::CAfAlgAeadCodec";
        return classTypeName;
    }

    /** Opens a transformation with the key, -1 with the reason in 'status' and 'errorMessage' when that failed */
    static int OpenKeyedTransform( const char* algName, const void* key, UInt32 keySize, TCodecStatus& status, char* errorMessage, size_t errorMessageSize )
    {GUCEF_TRACE;

        int transformFd = OpenTransformSocket( "aead", algName );
        if ( transformFd < 0 )
        {
            DescribeErrno( "AF_ALG: the algorithm is not available", errno, errorMessage, errorMessageSize );
            status = GUCEF_CODEC_STATUS_FAILED;
            return -1;
        }
        if ( 0 != setsockopt( transformFd, SOL_ALG, ALG_SET_KEY, key, keySize ) )
        {
            DescribeErrno( "AF_ALG: the key was refused", errno, errorMessage, errorMessageSize );
            status = GUCEF_CODEC_STATUS_INVALID_PARAMETER;
            CloseSocket( transformFd );
            return -1;
        }
        if ( 0 != setsockopt( transformFd, SOL_ALG, ALG_SET_AEAD_AUTHSIZE, GUCEF_NULL, AeadTagSize ) )
        {
            DescribeErrno( "AF_ALG: the tag size was refused", errno, errorMessage, errorMessageSize );
            status = GUCEF_CODEC_STATUS_FAILED;
            CloseSocket( transformFd );
            return -1;
        }
        status = GUCEF_CODEC_STATUS_OK;
        return transformFd;
    }

    protected:

    virtual TCodecStatus CreateInstanceImpl( const CValueList& params, CICodecInstancePtr& instance, CString& errorMessage ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const void* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( !TryGetBinaryParam( params, "key", key, keySize ) )
        {
            errorMessage = "The parameter \"key\" is required";
            return GUCEF_CODEC_STATUS_MISSING_PARAMETER;
        }
        if ( AeadKeySize != keySize )
        {
            errorMessage = "The parameter \"key\" must be 32 bytes";
            return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
        }

        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        TCodecStatus status = GUCEF_CODEC_STATUS_OK;
        int transformFd = OpenKeyedTransform( m_algName, key, keySize, status, text, sizeof( text ) );
        if ( transformFd < 0 )
        {
            errorMessage = text;
            return status;
        }
        int operationFd = accept( transformFd, GUCEF_NULL, 0 );
        if ( operationFd < 0 )
        {
            DescribeErrno( "AF_ALG: unable to start an operation", errno, text, sizeof( text ) );
            errorMessage = text;
            CloseSocket( transformFd );
            return GUCEF_CODEC_STATUS_FAILED;
        }

        instance = CICodecInstancePtr( GUCEF_NEW CAfAlgAeadInstance( transformFd, operationFd ) );
        return GUCEF_CODEC_STATUS_OK;
    }

    private:

    const char* m_algName;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
COsCryptoCodecs::CreateCodecs( CCodecRegistry::TICodecPtrVector& codecs )
{GUCEF_TRACE;

    struct SHashAlgorithm
    {
        const CString* familyName;
        const char* typeName;
        const char* algName;
        UInt32 digestSize;
        bool isMac;
    };
    static const SHashAlgorithm hashAlgorithms[] =
    {
        { &CoreCodecTypes::CryptographicHashCodec, "sha256", "sha256", 32, false },
        { &CoreCodecTypes::CryptographicHashCodec, "sha512", "sha512", 64, false },
        { &CoreCodecTypes::MacCodec, "hmac-sha256", "hmac(sha256)", 32, true }
    };
    for ( UInt32 i=0; i<sizeof( hashAlgorithms ) / sizeof( hashAlgorithms[ 0 ] ); ++i )
    {
        const SHashAlgorithm& algorithm = hashAlgorithms[ i ];
        int fd = OpenTransformSocket( "hash", algorithm.algName );
        if ( fd < 0 )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OsCryptoCodecs: AF_ALG does not offer \"" << algorithm.algName << "\" (errno " << errno << "), not registered" );
            continue;
        }
        CloseSocket( fd );
        codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CAfAlgHashCodec( *algorithm.familyName, algorithm.typeName, algorithm.algName, algorithm.digestSize, algorithm.isMac ) ) );
    }

    struct SAeadAlgorithm
    {
        const char* typeName;
        const char* algName;
    };
    static const SAeadAlgorithm aeadAlgorithms[] =
    {
        { "aes-256-gcm", "gcm(aes)" },
        { "chacha20-poly1305", "rfc7539(chacha20,poly1305)" }
    };
    static const UInt8 probeKey[ AeadKeySize ] = { 0 };
    for ( UInt32 i=0; i<sizeof( aeadAlgorithms ) / sizeof( aeadAlgorithms[ 0 ] ); ++i )
    {
        const SAeadAlgorithm& algorithm = aeadAlgorithms[ i ];

        // the whole set up a message needs, so an algorithm that is there but cannot be used is left out too
        char text[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE ];
        TCodecStatus status = GUCEF_CODEC_STATUS_OK;
        int transformFd = CAfAlgAeadCodec::OpenKeyedTransform( algorithm.algName, probeKey, AeadKeySize, status, text, sizeof( text ) );
        int operationFd = transformFd >= 0 ? accept( transformFd, GUCEF_NULL, 0 ) : -1;
        bool available = operationFd >= 0;
        if ( transformFd >= 0 && !available )
            DescribeErrno( "AF_ALG: unable to start an operation", errno, text, sizeof( text ) );
        CloseSocket( operationFd );
        CloseSocket( transformFd );
        if ( !available )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "OsCryptoCodecs: AF_ALG does not offer \"" << algorithm.algName << "\", not registered: " << text );
            continue;
        }
        codecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CAfAlgAeadCodec( algorithm.typeName, algorithm.algName ) ) );
    }
}

/*-------------------------------------------------------------------------*/

const char*
COsCryptoCodecs::GetProviderName( void )
{GUCEF_TRACE;

    return "AF_ALG";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */
