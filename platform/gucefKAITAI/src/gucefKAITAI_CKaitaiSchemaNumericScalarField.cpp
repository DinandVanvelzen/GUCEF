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

#include "gucefKAITAI_CKaitaiSchemaNumericScalarField.h"

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

const CORE::CString CKaitaiSchemaNumericScalarField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaNumericScalarField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaNumericScalarField::CKaitaiSchemaNumericScalarField( void )
    : CKaitaiSchemaBaseField( NumericScalarField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaNumericScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaNumericScalarField::CKaitaiSchemaNumericScalarField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( NumericScalarField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaNumericScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}
    
/*-------------------------------------------------------------------------*/

CKaitaiSchemaNumericScalarField::CKaitaiSchemaNumericScalarField( const CKaitaiSchemaNumericScalarField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaNumericScalarField, MT::CMutex >( this )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaNumericScalarField::~CKaitaiSchemaNumericScalarField()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaNumericScalarField& 
CKaitaiSchemaNumericScalarField::operator=( const CKaitaiSchemaNumericScalarField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaNumericScalarField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaNumericScalarField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaNumericScalarField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaNumericScalarField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    return CORE::CVariant::ByteSizeOfFixedSizeType( gucefDataType );
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaNumericScalarField::Serialize( CORE::CDataNode& domRootNode                        , 
                                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaNumericScalarField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    type = domRootNode.GetAttributeValueOrChildValueByName( "type" ).AsString();
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


