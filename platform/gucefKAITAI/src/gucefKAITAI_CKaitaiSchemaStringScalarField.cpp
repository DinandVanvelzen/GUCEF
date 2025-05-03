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

#include "gucefKAITAI_CKaitaiSchemaStringScalarField.h"

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

const CORE::CString CKaitaiSchemaStringScalarField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaStringScalarField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaStringScalarField::CKaitaiSchemaStringScalarField( void )
    : CKaitaiSchemaBaseField( StringScalarField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStringScalarField, MT::CMutex >( this )
    , m_encoding( "utf-8" )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStringScalarField::CKaitaiSchemaStringScalarField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( StringScalarField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStringScalarField, MT::CMutex >( this )
    , m_encoding( "utf-8" )
{GUCEF_TRACE;

}
    
/*-------------------------------------------------------------------------*/

CKaitaiSchemaStringScalarField::CKaitaiSchemaStringScalarField( const CKaitaiSchemaStringScalarField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStringScalarField, MT::CMutex >( this )
    , m_encoding( src.m_encoding )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStringScalarField::~CKaitaiSchemaStringScalarField()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStringScalarField& 
CKaitaiSchemaStringScalarField::operator=( const CKaitaiSchemaStringScalarField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaStringScalarField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaStringScalarField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaStringScalarField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaStringScalarField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    // Strings are variable size
    return -1;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStringScalarField::Serialize( CORE::CDataNode& domRootNode                        , 
                                           const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStringScalarField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                             const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    id = domRootNode.GetAttributeValueOrChildValueByName( "id" ).AsString();
    type = domRootNode.GetAttributeValueOrChildValueByName( "type" ).AsString();
    m_encoding = domRootNode.GetAttributeValueOrChildValueByName( "encoding", m_encoding ).AsString( m_encoding );
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


