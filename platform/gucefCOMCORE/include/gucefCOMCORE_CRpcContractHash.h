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

#ifndef GUCEF_COMCORE_CRPCCONTRACTHASH_H
#define GUCEF_COMCORE_CRPCCONTRACTHASH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The identity of a contract on the wire: SHA-256 of its canonical schema encoding (gucefCOMCORE RPC design 3.8).
 *  The text form is lowercase hex. A hash of all zero bytes means "none".
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcContractHash
{
    public:

    static const UInt32 Size = 32;

    CRpcContractHash( void );
    CRpcContractHash( const UInt8* bytes );
    CRpcContractHash( const CRpcContractHash& src );
    CRpcContractHash& operator=( const CRpcContractHash& src );

    /** The SHA-256 of the given bytes */
    static CRpcContractHash Compute( const void* data, UInt32 size );

    bool IsNull( void ) const;
    const UInt8* GetBytes( void ) const;
    void SetBytes( const UInt8* bytes );

    CORE::CString ToHexString( void ) const;
    bool FromHexString( const CORE::CString& hex );

    bool operator==( const CRpcContractHash& other ) const;
    bool operator!=( const CRpcContractHash& other ) const;
    bool operator<( const CRpcContractHash& other ) const;

    private:

    UInt8 m_bytes[ Size ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCCONTRACTHASH_H ? */
