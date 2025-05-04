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

#include "gucefKAITAI_CKaitaiSchemaConstValidationScalarField.h"

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

const CORE::CString CKaitaiSchemaConstValidationScalarField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaConstValidationScalarField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaConstValidationScalarField::CKaitaiSchemaConstValidationScalarField( void )
    : CKaitaiSchemaBaseField( ConstValidationField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConstValidationScalarField, MT::CMutex >( this )
    , m_referencedScalarFieldToValidate()
    , m_constantValue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConstValidationScalarField::CKaitaiSchemaConstValidationScalarField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( ConstValidationField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConstValidationScalarField, MT::CMutex >( this )
    , m_referencedScalarFieldToValidate()
    , m_constantValue()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConstValidationScalarField::CKaitaiSchemaConstValidationScalarField( const CKaitaiSchemaConstValidationScalarField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaConstValidationScalarField, MT::CMutex >( this )
    , m_referencedScalarFieldToValidate( src.m_referencedScalarFieldToValidate )
    , m_constantValue( src.m_constantValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConstValidationScalarField::~CKaitaiSchemaConstValidationScalarField()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaConstValidationScalarField& 
CKaitaiSchemaConstValidationScalarField::operator=( const CKaitaiSchemaConstValidationScalarField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );
        m_referencedScalarFieldToValidate = src.m_referencedScalarFieldToValidate;
        m_constantValue = src.m_constantValue;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaConstValidationScalarField::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();
    m_referencedScalarFieldToValidate.Clear();
    m_constantValue.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaConstValidationScalarField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaConstValidationScalarField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaConstValidationScalarField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CVariant& 
CKaitaiSchemaConstValidationScalarField::GetConstantValue( void ) const
{GUCEF_TRACE;

    return m_constantValue;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKaitaiSchemaConstValidationScalarField::GetReferencedScalarFieldToValidate( void ) const
{GUCEF_TRACE;
    
    return m_referencedScalarFieldToValidate;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaConstValidationScalarField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    return static_cast< Int32 >( m_constantValue.ByteSize( false ) );
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaConstValidationScalarField::Serialize( CORE::CDataNode& domRootNode                        , 
                                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaConstValidationScalarField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();

    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    type = "bytes";
    gucefDataType = GUCEF_DATATYPE_BINARY_BLOB;

    m_constantValue = domRootNode.GetAttributeValueOrChildValueByName( "contents" );
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


