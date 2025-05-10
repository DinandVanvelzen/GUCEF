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

#include "gucefKAITAI_CKaitaiSchemaLogicInstance.h"

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

const CORE::CString CKaitaiSchemaLogicInstance::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaLogicInstance";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaLogicInstance::CKaitaiSchemaLogicInstance( void )
    : CKaitaiSchemaBaseField( LogicInstance, CKaitaiSchemaBaseFieldPtr() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaLogicInstance::CKaitaiSchemaLogicInstance( CKaitaiSchemaBaseFieldPtr parent )
    : CKaitaiSchemaBaseField( LogicInstance, parent )
    , CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaLogicInstance::CKaitaiSchemaLogicInstance( const CKaitaiSchemaLogicInstance& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaLogicInstance, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaLogicInstance::~CKaitaiSchemaLogicInstance()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaLogicInstance& 
CKaitaiSchemaLogicInstance::operator=( const CKaitaiSchemaLogicInstance& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaLogicInstance::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaLogicInstance( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaLogicInstance::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaLogicInstance::Serialize( CORE::CDataNode& domRootNode                        , 
                                       const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaLogicInstance::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    id = domRootNode.GetName();
    type = "instance";

    if GUCEF_PREDICT_FALSE( id.IsNULLOrEmpty() )
        return false;

    m_expression = domRootNode.GetAttributeValueOrChildValueByName( "value" ).AsString();

    // if we dont have an expression it defeats the purpose of this class
    if GUCEF_PREDICT_FALSE( m_expression.IsNULLOrEmpty() )
        return false;
    
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaLogicInstance::GetExpression( void ) const
{GUCEF_TRACE;

    return m_expression;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


