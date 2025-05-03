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

#ifndef GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H
#define GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H

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

class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaBinaryScalarField : public CKaitaiSchemaBaseField ,
                                                               public CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaBinaryScalarFieldPtr;  
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >::TSharedPtrType       CKaitaiSchemaBinaryScalarFieldTypedPtr;  

    virtual Int32 GetFixedSizeIfAny( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaBinaryScalarField( void );                               /**< dont use this, use the other constructor */
    CKaitaiSchemaBinaryScalarField( CKaitaiSchemaMetaPtr schemaMeta );
    CKaitaiSchemaBinaryScalarField( const CKaitaiSchemaBinaryScalarField& src );
    virtual ~CKaitaiSchemaBinaryScalarField();
    CKaitaiSchemaBinaryScalarField& operator=( const CKaitaiSchemaBinaryScalarField& src );

    private:

    Int32 m_fixedSize; /**< The fixed size of the field, if any. */
    CORE::CString m_referencedScalarFieldForSize; /**< The name of the field to reference, if any. */
    CORE::CString m_sizeAsExpression; /**< The expression to evaluate for the size, if any. */

};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaBinaryScalarField::CKaitaiSchemaBinaryScalarFieldPtr         CKaitaiSchemaBinaryScalarFieldPtr;
typedef CKaitaiSchemaBinaryScalarField::CKaitaiSchemaBinaryScalarFieldTypedPtr    CKaitaiSchemaBinaryScalarFieldTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMABINARYSCALARFIELD_H ? */
