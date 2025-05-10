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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefKAITAI_CKaitaiSchemaEnumDefinition.h"

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

const CORE::CString CKaitaiSchemaEnumDefinition::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaEnumDefinition";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaEnumDefinition::CKaitaiSchemaEnumDefinition( void )
    : CKaitaiSchemaBaseField( EnumDefinition, CKaitaiSchemaBaseFieldPtr() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >( this )
    , m_enumValues()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumDefinition::CKaitaiSchemaEnumDefinition( CKaitaiSchemaBaseFieldPtr parent )
    : CKaitaiSchemaBaseField( EnumDefinition, parent )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >( this )
    , m_enumValues()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumDefinition::CKaitaiSchemaEnumDefinition( const CKaitaiSchemaEnumDefinition& src )
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumDefinition, MT::CMutex >( this )
    , m_enumValues( src.m_enumValues )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumDefinition& 
CKaitaiSchemaEnumDefinition::operator=( const CKaitaiSchemaEnumDefinition& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        m_enumValues = src.m_enumValues;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaEnumDefinition::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaEnumDefinition( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaEnumDefinition::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

const CKaitaiSchemaEnumDefinition::TInt32ToStringMap& 
CKaitaiSchemaEnumDefinition::GetEnumValues( void ) const
{GUCEF_TRACE;

    return m_enumValues;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaEnumDefinition::Serialize( CORE::CDataNode& domRootNode                        , 
                                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaEnumDefinition::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
        
    const CORE::CString& enumName = domRootNode.GetName();
    if GUCEF_PREDICT_FALSE( enumName.IsNULLOrEmpty() )
        return false;

    UInt32 nrOfAttributes = domRootNode.GetAttCount();
    if GUCEF_PREDICT_FALSE( 0 == nrOfAttributes )
        return false;

    id = enumName;
    gucefDataType = GUCEF_DATATYPE_ENUM;
    
    for ( UInt32 a=0; a<nrOfAttributes; ++a )
    {
        const CORE::CDataNode::TKeyValuePair* mapEntry = domRootNode.GetAttribute( a );
        if ( GUCEF_NULL != mapEntry )
        {
            Int32 enumIndex = CORE::StringToInt32( mapEntry->first, 0 );
            const CORE::CString& enumLabel = mapEntry->second;
                    
            m_enumValues[ enumIndex ] = enumLabel;
        }
    }

    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


