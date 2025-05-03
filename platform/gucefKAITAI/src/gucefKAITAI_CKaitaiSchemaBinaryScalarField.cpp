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

#include "gucefKAITAI_CKaitaiSchemaBinaryScalarField.h"

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

const CORE::CString CKaitaiSchemaBinaryScalarField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaBinaryScalarField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaBinaryScalarField::CKaitaiSchemaBinaryScalarField( void )
    : CKaitaiSchemaBaseField( BinaryScalarField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >( this )
    , m_fixedSize( -1 )
    , m_referencedScalarFieldForSize()
    , m_sizeAsExpression()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBinaryScalarField::CKaitaiSchemaBinaryScalarField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( BinaryScalarField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >( this )
    , m_fixedSize( -1 )
    , m_referencedScalarFieldForSize()
    , m_sizeAsExpression()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBinaryScalarField::CKaitaiSchemaBinaryScalarField( const CKaitaiSchemaBinaryScalarField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaBinaryScalarField, MT::CMutex >( this )
    , m_fixedSize( src.m_fixedSize )
    , m_referencedScalarFieldForSize( src.m_referencedScalarFieldForSize )
    , m_sizeAsExpression( src.m_sizeAsExpression )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBinaryScalarField::~CKaitaiSchemaBinaryScalarField()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBinaryScalarField& 
CKaitaiSchemaBinaryScalarField::operator=( const CKaitaiSchemaBinaryScalarField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );
        m_fixedSize = src.m_fixedSize;
        m_referencedScalarFieldForSize = src.m_referencedScalarFieldForSize;
        m_sizeAsExpression = src.m_sizeAsExpression;

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaBinaryScalarField::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();
    m_fixedSize = -1;
    m_referencedScalarFieldForSize.Clear();
    m_sizeAsExpression.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaBinaryScalarField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaBinaryScalarField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBinaryScalarField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaBinaryScalarField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    // the binary payload may or may not have a fixed size
    // this class supports both kinds of binary payloads
    return m_fixedSize;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBinaryScalarField::Serialize( CORE::CDataNode& domRootNode                        , 
                                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBinaryScalarField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();

    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();

    // We do not have a type
    // the type is implicit via 'size' which means its a binary payload
    CORE::CString sizeValue = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsString();
    if ( !sizeValue.IsNULLOrEmpty() )
    {
        // size fields don't just have to represent a fixed size.
        // alternatively it can be a variable size field of type:
        //   - Dynamic expressions –> The size can be determined by another field or an expression
        //   - End-of-stream (eos) –> This makes the field consume all remaining bytes
        //   - Computed values using sizeof() –> This allows referencing known fixed sizes dynamically
        // We need to figure out which one this is
        m_fixedSize = KaitaiFixedSizeValueStringToFixedSizeIfAny( sizeValue );
        if ( m_fixedSize < 0 )
        {
            // We have a variable size field
            // We need to check if it is a dynamic expression or a reference to a scalar/instance field
            sizeValue = sizeValue.Trim( true ).Trim( false );
            if ( IsValidPossibleFieldName( sizeValue ) )
            {
                // We have a reference to a scalar field
                m_referencedScalarFieldForSize = sizeValue;
            }
            else
            {
                // We have a dynamic expression
                m_sizeAsExpression = sizeValue;
            }
        }
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


