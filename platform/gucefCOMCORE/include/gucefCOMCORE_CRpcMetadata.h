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

#ifndef GUCEF_COMCORE_CRPCMETADATA_H
#define GUCEF_COMCORE_CRPCMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Key / value metadata of a call (gucefCOMCORE RPC design 6.9). Values are strings, bytes, UInt64, Int64 or bool.
 *  Keys are ASCII. On the wire well-known keys cost one varint and other keys are interned per session direction,
 *  so a key travels once.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMetadata
{
    public:

    static const UInt32 MaxEntries = 64;
    static const UInt32 MaxBytes = 8192;

    static const char* IdempotencyKey;      /**< "idempotency-key", well-known 2 */
    static const char* TraceId;             /**< "trace-id", well-known 3 */
    static const char* SpanId;              /**< "span-id", well-known 4 */
    static const char* Locale;              /**< "locale", well-known 5 */
    static const char* ClientVersion;       /**< "client-version", well-known 6 */
    static const char* ClientTimeZone;      /**< "client-time-zone", well-known 7 */

    /** The well-known key reference 2 ... 7 of a key, 0 when it is not well-known */
    static UInt32 GetWellKnownKeyRef( const CORE::CString& key );

    /** The key of a well-known reference, empty when there is none */
    static const char* GetWellKnownKey( UInt32 keyRef );

    CRpcMetadata( void );
    CRpcMetadata( const CRpcMetadata& src );
    ~CRpcMetadata();
    CRpcMetadata& operator=( const CRpcMetadata& src );

    /** Sets or replaces the value of a key. False for an unsupported value type */
    bool Set( const CORE::CString& key, const CORE::CVariant& value );
    bool Get( const CORE::CString& key, CORE::CVariant& value ) const;
    CORE::CString GetString( const CORE::CString& key ) const;
    bool Has( const CORE::CString& key ) const;
    bool Remove( const CORE::CString& key );
    void Clear( void );

    UInt32 GetCount( void ) const;
    const CORE::CString& GetKeyAt( UInt32 index ) const;
    const CORE::CVariant& GetValueAt( UInt32 index ) const;

    private:

    typedef GUCEF::vector< CORE::CString >    TStringVector;
    typedef GUCEF::vector< CORE::CVariant >   TVariantVector;

    TStringVector m_keys;
    TVariantVector m_values;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCMETADATA_H ? */
