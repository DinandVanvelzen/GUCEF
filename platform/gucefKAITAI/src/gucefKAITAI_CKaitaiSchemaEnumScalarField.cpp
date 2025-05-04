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

#include "gucefKAITAI_CKaitaiSchemaEnumScalarField.h"

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

const CORE::CString CKaitaiSchemaEnumScalarField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaEnumScalarField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaEnumScalarField::CKaitaiSchemaEnumScalarField( void )
    : CKaitaiSchemaBaseField( EnumScalarField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumScalarField::CKaitaiSchemaEnumScalarField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( EnumScalarField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}
    
/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumScalarField::CKaitaiSchemaEnumScalarField( const CKaitaiSchemaEnumScalarField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaEnumScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumScalarField::~CKaitaiSchemaEnumScalarField()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaEnumScalarField& 
CKaitaiSchemaEnumScalarField::operator=( const CKaitaiSchemaEnumScalarField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaEnumScalarField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaEnumScalarField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaEnumScalarField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaEnumScalarField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    return CORE::CVariant::ByteSizeOfFixedSizeType( gucefDataType );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaEnumScalarField::GetReferencedEnum( void ) const
{GUCEF_TRACE;

    return m_referencedEnum;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaEnumScalarField::Serialize( CORE::CDataNode& domRootNode                        , 
                                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaEnumScalarField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    type = domRootNode.GetAttributeValueOrChildValueByName( "type" ).AsString();
    m_referencedEnum = domRootNode.GetAttributeValueOrChildValueByName( "enum" ).AsString();

    if GUCEF_PREDICT_FALSE( m_referencedEnum.IsNULLOrEmpty() )
        return false;
    
    if ( !type.IsNULLOrEmpty() )
    {
        gucefDataType = KaitaiBuildInTypeStringToGucefType( type );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/


