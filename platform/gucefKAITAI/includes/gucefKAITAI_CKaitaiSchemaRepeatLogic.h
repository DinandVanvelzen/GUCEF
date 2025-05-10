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

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREPEATLOGIC_H
#define GUCEF_KAITAI_CKAITAISCHEMAREPEATLOGIC_H

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

enum RepeatType
{
    UnknownRepeat = 0,
    
    RepeatUntilExpressionFalse,
    RepeatUntilEndOfStream,
    RepeatUpToGivenCount
};

/*-------------------------------------------------------------------------*/

/**
 * CKaitaiSchemaRepeatLogic supports looping constructs in the Kaitai schema.
 */
class GUCEF_KAITAI_PUBLIC_CPP CKaitaiSchemaRepeatLogic : public CKaitaiSchemaBaseField ,
                                                         public CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >
{
    public:

    static const CORE::CString ClassTypeName;

    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >::TBasicSharedPtrType  CKaitaiSchemaRepeatLogicPtr;
    typedef typename CORE::CTSharedObjCreator< CKaitaiSchemaRepeatLogic, MT::CMutex >::TSharedPtrType       CKaitaiSchemaRepeatLogicTypedPtr;
    
    CORE::CString switchOn;                              // Expression to determine type    
    CORE::CString defaultCase;                           // Default case for unmatched conditions

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Clear( void ) GUCEF_VIRTUAL_OVERRIDE;

    CKaitaiSchemaRepeatLogic( void );                               /**< dont use this, use the other constructor */
    CKaitaiSchemaRepeatLogic( CKaitaiSchemaBaseFieldPtr parent );
    CKaitaiSchemaRepeatLogic( const CKaitaiSchemaRepeatLogic& src );
    virtual ~CKaitaiSchemaRepeatLogic();
    CKaitaiSchemaRepeatLogic& operator=( const CKaitaiSchemaRepeatLogic& src );

    RepeatType GetRepeatType( void ) const;

    const CORE::CString& GetRepeatExpression( void ) const;

    CKaitaiSchemaBaseFieldPtr GetRepeatedField( void ) const;

    private:
    
    CORE::CString m_repeatExpression;          /**< the repeat expression to evaluate if applicable */
    RepeatType m_repeatType;                   /**< the manner in which the repeat logic should execute */
    CKaitaiSchemaBaseFieldPtr m_repeatedField; /**< The field to repeat */
};

/*-------------------------------------------------------------------------*/

typedef CKaitaiSchemaRepeatLogic::CKaitaiSchemaRepeatLogicPtr         CKaitaiSchemaRepeatLogicPtr;
typedef CKaitaiSchemaRepeatLogic::CKaitaiSchemaRepeatLogicTypedPtr    CKaitaiSchemaRepeatLogicTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAISCHEMAREPEATLOGIC_H ? */
