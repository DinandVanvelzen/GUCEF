/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTRSASIGNER_H
#define GUCEF_AUTH_TESTAPP_TESTRSASIGNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The private operation of RSA with a key of the tests, to make the signed tokens that the fake provider hands out.
 *  The product only verifies. Montgomery arithmetic on 32 bit limbs, the Chinese remainder theorem: a signature takes
 *  a few milliseconds. The keys are the throw away keys of TestRsaVectors.h.
 */
class CTestRsaKey
{
    public:

    CTestRsaKey( const char* nHex, const char* eHex, const char* pHex, const char* qHex, const char* dpHex, const char* dqHex, const char* qinvHex );

    /** RSASSA-PKCS1-v1_5 with SHA-256 over the message: the signature of RS256 */
    bool SignSha256( const void* message, GUCEF::UInt32 size, GUCEF::CORE::CDynamicBuffer& signature ) const;

    const GUCEF::CORE::CDynamicBuffer& GetModulus( void ) const { return m_modulus; }

    const GUCEF::CORE::CDynamicBuffer& GetExponent( void ) const { return m_exponent; }

    /** The key as an entry of a JWKS document */
    GUCEF::CORE::CString ToJwk( const GUCEF::CORE::CString& keyId, const GUCEF::CORE::CString& algorithm = "RS256" ) const;

    /** A signed token: header and payload are JSON text; the key id and algorithm go in the header by the caller */
    GUCEF::CORE::CString MakeToken( const GUCEF::CORE::CString& headerJson, const GUCEF::CORE::CString& payloadJson ) const;

    private:

    typedef GUCEF::vector< GUCEF::UInt32 > TBig;

    static TBig FromHex( const char* hex );
    static void Trim( TBig& value );
    static bool GreaterOrEqual( const TBig& a, const TBig& b );
    static void SubtractInPlace( TBig& a, const TBig& b );
    static TBig Reduce( const TBig& value, const TBig& modulus );
    static TBig Multiply( const TBig& a, const TBig& b );
    static TBig PowerMod( const TBig& base, const TBig& exponent, const TBig& modulus );
    static TBig MultiplyMod( const TBig& a, const TBig& b, const TBig& modulus );

    private:

    TBig m_n;
    TBig m_p;
    TBig m_q;
    TBig m_dp;
    TBig m_dq;
    TBig m_qinv;
    GUCEF::CORE::CDynamicBuffer m_modulus;
    GUCEF::CORE::CDynamicBuffer m_exponent;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTRSASIGNER_H ? */
