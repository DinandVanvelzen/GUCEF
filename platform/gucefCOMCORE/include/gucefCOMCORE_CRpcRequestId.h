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

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#define GUCEF_COMCORE_CRPCREQUESTID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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
 *  The optional id of accepted work (gucefCOMCORE RPC design 6.5.3): none, a UInt64 or a string of at most 256
 *  bytes of UTF-8. A CVariant restricted to those kinds.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcRequestId : public CORE::CICloneable
{
    public:

    enum EKind
    {
        KIND_NONE   = 0 ,
        KIND_UINT64 = 1 ,
        KIND_STRING = 2
    };

    static const UInt32 MaxStringBytes = 256;

    CRpcRequestId( void );
    explicit CRpcRequestId( UInt64 id );
    explicit CRpcRequestId( const CORE::CString& id );
    CRpcRequestId( const CRpcRequestId& src );
    virtual ~CRpcRequestId() GUCEF_VIRTUAL_OVERRIDE;
    CRpcRequestId& operator=( const CRpcRequestId& src );

    /** From a variant: an unsigned or non-negative integer, or a string; false for anything else */
    bool FromVariant( const CORE::CVariant& id );

    EKind GetKind( void ) const;
    bool IsSet( void ) const;
    UInt64 GetUInt64( void ) const;
    CORE::CString GetString( void ) const;
    const CORE::CVariant& GetVariant( void ) const;
    void Clear( void );

    /** "#123" for an integer id, the text for a string id, empty for none */
    CORE::CString ToString( void ) const;

    bool operator==( const CRpcRequestId& other ) const;
    bool operator!=( const CRpcRequestId& other ) const;
    bool operator<( const CRpcRequestId& other ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    EKind m_kind;
    UInt64 m_number;
    CORE::CString m_text;
    CORE::CVariant m_variant;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */
