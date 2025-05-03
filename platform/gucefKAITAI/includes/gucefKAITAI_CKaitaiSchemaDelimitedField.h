/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H
#define GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#include "gucefKAITAI_CKaitaiSchemaBaseField.h"
#define GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMABASEFIELD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * CKaitaiSchemaDelimitedField represents fields with variable-length data that terminate upon reaching a specific delimiter.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaDelimitedField : public CKaitaiSchemaBaseField ,
                                                            public CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaDelimitedFieldPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaDelimitedField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaDelimitedFieldTypedPtr;
    
    
    CORE::CString delimiter; // Delimiter marking the end of the field

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaDelimitedField( void );                           /**< dont use this, use the other constructor */
    CKaitaiSchemaDelimitedField( CKaitaiSchemaMetaPtr schemaMeta );
    CKaitaiSchemaDelimitedField( const CKaitaiSchemaDelimitedField& src );
    virtual ~CKaitaiSchemaDelimitedField();
    CKaitaiSchemaDelimitedField& operator=( const CKaitaiSchemaDelimitedField& src );
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaDelimitedField::CKaitaiSchemaDelimitedFieldPtr         CKaitaiSchemaDelimitedFieldPtr;
typedef CKaitaiSchemaDelimitedField::CKaitaiSchemaDelimitedFieldTypedPtr    CKaitaiSchemaDelimitedFieldTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMADELIMITEDFIELD_H ? */
