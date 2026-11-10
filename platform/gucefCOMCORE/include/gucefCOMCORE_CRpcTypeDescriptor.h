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

#ifndef GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H
#define GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H
#include "gucefCOMCORE_CRpcFieldDescriptor.h"
#define GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCFIELDDESCRIPTOR_H ? */

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
 *  A message type of a contract (gucefCOMCORE RPC design 3.4): fields sorted by number, a number to slot table
 *  (dense when the numbers allow it, so a lookup by number is O(1)), reserved numbers and names.
 *  Immutable once its contract is built.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcTypeDescriptor
{
    public:

    typedef GUCEF::vector< CRpcFieldDescriptor* >   TFieldVector;
    typedef GUCEF::set< UInt32 >                    TUInt32Set;

    CRpcTypeDescriptor( void );
    ~CRpcTypeDescriptor();

    const CORE::CString& GetName( void ) const;
    const CORE::CString& GetFullName( void ) const;
    const CORE::CString& GetDoc( void ) const;

    UInt32 GetFieldCount( void ) const;

    /** By slot index, which is the position by ascending field number */
    const CRpcFieldDescriptor& GetFieldAt( UInt32 slotIndex ) const;

    const CRpcFieldDescriptor* FindFieldByNumber( UInt32 number ) const;
    const CRpcFieldDescriptor* FindFieldByName( const CORE::CString& name ) const;

    const TUInt32Set& GetReservedNumbers( void ) const;
    const CORE::CString::StringSet& GetReservedNames( void ) const;

    private:
    friend class CRpcContract;

    typedef GUCEF::map< UInt32, UInt32 >            TNumberSlotMap;
    typedef GUCEF::map< CORE::CString, UInt32 >     TNameSlotMap;
    typedef GUCEF::vector< Int32 >                  TDenseSlotTable;

    void BuildLookups( void );

    CRpcTypeDescriptor( const CRpcTypeDescriptor& src );                /**< not implemented */
    CRpcTypeDescriptor& operator=( const CRpcTypeDescriptor& src );     /**< not implemented */

    CORE::CString m_name;
    CORE::CString m_fullName;
    CORE::CString m_doc;
    TFieldVector m_fields;
    TDenseSlotTable m_denseSlots;
    TNumberSlotMap m_sparseSlots;
    TNameSlotMap m_nameSlots;
    TUInt32Set m_reservedNumbers;
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

#endif /* GUCEF_COMCORE_CRPCTYPEDESCRIPTOR_H ? */
