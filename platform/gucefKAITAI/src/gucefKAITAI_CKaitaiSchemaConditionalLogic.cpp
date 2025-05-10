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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefKAITAI_CKaitaiSchemaConditionalLogic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKaitaiSchemaConditionalLogic::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaConditionalLogic";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaConditionalLogic::CKaitaiSchemaConditionalLogic( void )
    : CKaitaiSchemaBaseField( ConditionalLogic, CKaitaiSchemaBaseFieldPtr() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >( this )
    , m_ifExpression()
   // , m_switchType( UnknownSwitch )
    , m_cases()
    , m_defaultCase()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConditionalLogic::CKaitaiSchemaConditionalLogic( CKaitaiSchemaBaseFieldPtr parent )
    : CKaitaiSchemaBaseField( ConditionalLogic, parent )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >( this )
    , m_ifExpression()
  //  , m_switchType( UnknownSwitch )
    , m_cases()
    , m_defaultCase()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConditionalLogic::CKaitaiSchemaConditionalLogic( const CKaitaiSchemaConditionalLogic& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConditionalLogic, MT::CMutex >( this )
    , m_ifExpression( src.m_ifExpression )
   // , m_switchType( src.m_switchType )
    , m_cases( src.m_cases )
    , m_defaultCase( src.m_defaultCase )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConditionalLogic::~CKaitaiSchemaConditionalLogic()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CKaitaiSchemaConditionalLogic::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();
    m_ifExpression.Clear();
  // m_switchType = UnknownSwitch;
    m_cases.clear();
    m_defaultCase.Clear();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConditionalLogic& 
CKaitaiSchemaConditionalLogic::operator=( const CKaitaiSchemaConditionalLogic& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );
        m_ifExpression = src.m_ifExpression;
      //  m_switchType = src.m_switchType;
        m_cases = src.m_cases;
        m_defaultCase = src.m_defaultCase;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaConditionalLogic::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaConditionalLogic( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaConditionalLogic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/
//
//SwitchType 
//CKaitaiSchemaConditionalLogic::GetSwitchType( void ) const
//{GUCEF_TRACE;
//
//    return m_switchType;
//}

/*-------------------------------------------------------------------------*/

const CORE::CVariantMap& 
CKaitaiSchemaConditionalLogic::GetCases( void ) const
{GUCEF_TRACE;

    return m_cases;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchemaConditionalLogic::HasDefaultCase( void ) const
{GUCEF_TRACE;

    return m_defaultCase.IsInitialized();
}

/*-------------------------------------------------------------------------*/
    
const CORE::CVariant& 
CKaitaiSchemaConditionalLogic::GetDefaultCase( void ) const
{GUCEF_TRACE;

    return m_defaultCase;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaConditionalLogic::Serialize( CORE::CDataNode& domRootNode                        , 
                                          const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaConditionalLogic::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    if GUCEF_PREDICT_FALSE( id.IsNULLOrEmpty() )
        return false; 

    // This type of field should always have an 'if' attribute which enabled or disables the field
    m_ifExpression = domRootNode.GetAttributeValueOrChildValueByName( "if" ).AsString();
    if GUCEF_PREDICT_FALSE( id.IsNULLOrEmpty() )
        return false; 

    // Note that you are not allowed to have both 'type' and 'size'
    // one or the other
    const CORE::CDataNode* typeNode = domRootNode.FindChild( "type" );
    const CORE::CDataNode* sizeNode = domRootNode.FindChild( "size" );
    const CORE::CDataNode* parentNode = GUCEF_NULL != typeNode ? typeNode : sizeNode;

    bool isTypeSwitch = GUCEF_NULL != typeNode;
    bool isSizeSwitch = GUCEF_NULL != sizeNode;

    if GUCEF_PREDICT_FALSE( !isTypeSwitch && !isSizeSwitch )
        return false;
    if GUCEF_PREDICT_FALSE( isTypeSwitch && isSizeSwitch )
        return false;
    //m_switchType = isTypeSwitch ? TypeViaSwitch : SizeViaSwitch;
    
    CORE::CString switchOnValue = parentNode->GetAttributeValueOrChildValueByName( "switch-on" ).AsString();
    const CORE::CDataNode* switchCasesNode = domRootNode.Find( "cases" );
    if GUCEF_PREDICT_FALSE( switchOnValue.IsNULLOrEmpty() ||  GUCEF_NULL == switchCasesNode )
        return false;

    UInt32 nrOfAttributes = switchCasesNode->GetAttCount();
    if GUCEF_PREDICT_FALSE( 0 == nrOfAttributes )
        return false;

    m_cases.clear();

    gucefDataType = GUCEF_DATATYPE_MAP; // @TODO: ? not a good fit
    
    for ( UInt32 a=0; a<nrOfAttributes; ++a )
    {
        const CORE::CDataNode::TKeyValuePair* caseEntry = switchCasesNode->GetAttribute( a );
        if ( GUCEF_NULL != caseEntry )
        {
            CORE::CVariant caseVar( caseEntry->first );
            const CORE::CVariant& caseTarget = caseEntry->second;
                    
            if ( caseVar != "_" ) // default case is denoted via an underscore as the case label
            {
                m_cases[ caseVar ] = caseTarget;
            }
            else
            {
                m_defaultCase = caseTarget;
            }
        }
    }

    if ( !m_cases.empty() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaSwitchField:Deserialize: Successfully deserialized switch field with id \"" + id + 
            "\" and switch-on value \"" + switchOnValue + " which has " + CORE::ToString( m_cases.size() ) + " cases" );
    
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaSwitchField:Deserialize: Failed to deserialize switch field with id \"" + id + 
            "\" and switch-on value \"" + switchOnValue + " which has no cases" );
        return false;
    }    
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


