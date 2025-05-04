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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H
#define GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H

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

class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaEnumScalarField : public CKaitaiSchemaBaseField ,
                                                             public CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaEnumScalarFieldPtr;  
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaEnumScalarFieldTypedPtr;  

    virtual Int32 GetFixedSizeIfAny( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaEnumScalarField( void );                               /**< dont use this, use the other constructor */
    CKaitaiSchemaEnumScalarField( CKaitaiSchemaMetaPtr schemaMeta );
    CKaitaiSchemaEnumScalarField( const CKaitaiSchemaEnumScalarField& src );
    virtual ~CKaitaiSchemaEnumScalarField();
    CKaitaiSchemaEnumScalarField& operator=( const CKaitaiSchemaEnumScalarField& src );

    const CORE::CString& GetReferencedEnum( void ) const;

    private:

    CORE::CString m_referencedEnum;
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaEnumScalarField::CKaitaiSchemaEnumScalarFieldPtr         CKaitaiSchemaEnumScalarFieldPtr;
typedef CKaitaiSchemaEnumScalarField::CKaitaiSchemaEnumScalarFieldTypedPtr    CKaitaiSchemaEnumScalarFieldTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMAENUMSCALARFIELD_H ? */
