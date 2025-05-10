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

#ifndef GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H
#define GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H

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

//enum SwitchType
//{
//    UnknownSwitch = 0,
//    
//    TypeViaSwitch,
//    SizeViaSwitch,
//};

/*-------------------------------------------------------------------------*/

/**
 * CKaitaiSchemaConditionalLogic supports take one action or another based on an expression.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaConditionalLogic : public CKaitaiSchemaBaseField ,
                                                              public CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaConditionalLogicPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >::TSharedPtrType       CKaitaiSchemaConditionalLogicTypedPtr;
    
    CORE::CString switchOn;                              // Expression to determine type    
    CORE::CString defaultCase;                           // Default case for unmatched conditions

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaConditionalLogic( void );                               /**< dont use this, use the other constructor */
    CKaitaiSchemaConditionalLogic( CKaitaiSchemaBaseFieldPtr parent );
    CKaitaiSchemaConditionalLogic( const CKaitaiSchemaConditionalLogic& src );
    virtual ~CKaitaiSchemaConditionalLogic();
    CKaitaiSchemaConditionalLogic& operator=( const CKaitaiSchemaConditionalLogic& src );

    //SwitchType GetSwitchType( void ) const;

    const CORE::CVariantMap& GetCases( void ) const;
    
    bool HasDefaultCase( void ) const;

    const CORE::CVariant& GetDefaultCase( void ) const;

    private:
    
    CORE::CString m_ifExpression;   // the expression that enables or disables the associated field
    CORE::CVariantMap m_cases;       // Mapping of cases to types
    CORE::CVariant m_defaultCase;    // Default case for unmatched conditions

};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaConditionalLogic::CKaitaiSchemaConditionalLogicPtr         CKaitaiSchemaConditionalLogicPtr;
typedef CKaitaiSchemaConditionalLogic::CKaitaiSchemaConditionalLogicTypedPtr    CKaitaiSchemaConditionalLogicTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMACONDITIONALLOGIC_H ? */
