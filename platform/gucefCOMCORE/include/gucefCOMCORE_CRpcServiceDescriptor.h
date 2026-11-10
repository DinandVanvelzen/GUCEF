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

#ifndef GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H
#define GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H
#include "gucefCOMCORE_CRpcMethodDescriptor.h"
#define GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H ? */

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
 *  A service of a contract (gucefCOMCORE RPC design 3.3, 3.4): methods sorted by number, reserved method numbers.
 *  Immutable once its contract is built.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcServiceDescriptor
{
    public:

    typedef GUCEF::vector< CRpcMethodDescriptor* >  TMethodVector;
    typedef GUCEF::set< UInt32 >                    TUInt32Set;

    CRpcServiceDescriptor( void );
    ~CRpcServiceDescriptor();

    UInt32 GetNumber( void ) const;
    const CORE::CString& GetName( void ) const;
    const CORE::CString& GetFullName( void ) const;
    UInt32 GetSince( void ) const;
    const CORE::CString& GetDoc( void ) const;

    UInt32 GetMethodCount( void ) const;
    const CRpcMethodDescriptor& GetMethodAt( UInt32 index ) const;     /**< sorted by number */
    const CRpcMethodDescriptor* FindMethodByNumber( UInt32 number ) const;
    const CRpcMethodDescriptor* FindMethodByName( const CORE::CString& name ) const;

    const TUInt32Set& GetReservedNumbers( void ) const;

    private:
    friend class CRpcContract;

    CRpcServiceDescriptor( const CRpcServiceDescriptor& src );                /**< not implemented */
    CRpcServiceDescriptor& operator=( const CRpcServiceDescriptor& src );     /**< not implemented */

    UInt32 m_number;
    CORE::CString m_name;
    CORE::CString m_fullName;
    UInt32 m_since;
    CORE::CString m_doc;
    TMethodVector m_methods;
    TUInt32Set m_reservedNumbers;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H ? */
