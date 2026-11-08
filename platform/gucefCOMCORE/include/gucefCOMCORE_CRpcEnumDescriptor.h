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

#ifndef GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H
#define GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_RPCTYPES_H
#include "gucefCOMCORE_RpcTypes.h"
#define GUCEF_COMCORE_RPCTYPES_H
#endif /* GUCEF_COMCORE_RPCTYPES_H ? */

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

class CRpcContract;

/*-------------------------------------------------------------------------*/

/**
 *  An enum of a contract (gucefCOMCORE RPC design 3.1, 3.4): values by name and number, reserved numbers and names.
 *  Enums are open: an unknown value is kept as a number. Immutable once its contract is built.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcEnumDescriptor
{
    public:

    struct GUCEF_COMCORE_EXPORT_CPP SValue
    {
        Int32 number;
        CORE::CString name;
        UInt32 since;
        CORE::CString doc;

        SValue( void );
    };
    typedef GUCEF::vector< SValue >     TValueVector;
    typedef GUCEF::set< Int32 >         TInt32Set;

    CRpcEnumDescriptor( void );
    ~CRpcEnumDescriptor();

    const CORE::CString& GetName( void ) const;         /**< "NoteState" */
    const CORE::CString& GetFullName( void ) const;     /**< "gucef.sample.notes.NoteState" */
    const CORE::CString& GetDoc( void ) const;

    UInt32 GetValueCount( void ) const;
    const SValue& GetValueAt( UInt32 index ) const;     /**< sorted by number */
    const SValue* FindValueByNumber( Int32 number ) const;
    const SValue* FindValueByName( const CORE::CString& name ) const;

    const TInt32Set& GetReservedNumbers( void ) const;
    const CORE::CString::StringSet& GetReservedNames( void ) const;

    private:
    friend class CRpcContract;

    CRpcEnumDescriptor( const CRpcEnumDescriptor& src );                /**< not implemented */
    CRpcEnumDescriptor& operator=( const CRpcEnumDescriptor& src );     /**< not implemented */

    CORE::CString m_name;
    CORE::CString m_fullName;
    CORE::CString m_doc;
    TValueVector m_values;
    TInt32Set m_reservedNumbers;
    CORE::CString::StringSet m_reservedNames;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCENUMDESCRIPTOR_H ? */
