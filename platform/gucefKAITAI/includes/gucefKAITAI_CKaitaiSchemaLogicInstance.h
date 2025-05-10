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

#ifndef GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H
#define GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H

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
 * CKaitaiSchemaLogicInstance represents computed fields derived from other schema data.
 * Can support complex expressions or dependencies.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaLogicInstance : public CKaitaiSchemaBaseField ,
                                                           public CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaLogicInstancePtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >::TSharedPtrType       CKaitaiSchemaLogicInstanceTypedPtr;
    
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetExpression( void ) const;
    
    CKaitaiSchemaLogicInstance( void );                                   /**< dont use this, use the other constructor */
    CKaitaiSchemaLogicInstance( CKaitaiSchemaBaseFieldPtr parent );
    CKaitaiSchemaLogicInstance( const CKaitaiSchemaLogicInstance& src );
    virtual ~CKaitaiSchemaLogicInstance();
    CKaitaiSchemaLogicInstance& operator=( const CKaitaiSchemaLogicInstance& src );

    private:
    
    CORE::CString m_expression; // Logic for deriving the value
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaLogicInstance::CKaitaiSchemaLogicInstancePtr         CKaitaiSchemaLogicInstancePtr;
typedef CKaitaiSchemaLogicInstance::CKaitaiSchemaLogicInstanceTypedPtr    CKaitaiSchemaLogicInstanceTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMALOGICINSTANCE_H ? */
