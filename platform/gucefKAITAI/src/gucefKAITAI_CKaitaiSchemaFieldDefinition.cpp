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

#include "gucefKAITAI_CKaitaiSchemaFieldDefinition.h"

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

const CORE::CString CKaitaiSchemaFieldDefinition::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaFieldDefinition";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/
CKaitaiSchemaFieldDefinition::CKaitaiSchemaFieldDefinition( void )
    : CORE::CIDataNodeSerializable()
    , id()
    , type()
    , endianess( GUCEF_BYTEORDER_UNKNOWN_ENDIAN )
    , fieldTypeId( 0 )
    , hasSize( false )
    , size( 0 )
    , isEnum( false )
    , enumId()
    , documentation()
    , hasContents( false )
    , contents()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaFieldDefinition::CKaitaiSchemaFieldDefinition( const CKaitaiSchemaFieldDefinition& src )    
    : CORE::CIDataNodeSerializable( src )
    , id( src.id )
    , type( src.type )
    , endianess( src.endianess )
    , fieldTypeId( src.fieldTypeId )
    , hasSize( src.hasSize )
    , size( src.size )
    , isEnum( src.isEnum )
    , enumId( src.enumId )
    , documentation( src.documentation )
    , hasContents( src.hasContents )
    , contents( src.contents )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaFieldDefinition& 
CKaitaiSchemaFieldDefinition::operator=( const CKaitaiSchemaFieldDefinition& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        id = src.id;
        type = src.type;
        endianess = src.endianess;
        fieldTypeId = src.fieldTypeId;
        hasSize = src.hasSize;
        size = src.size;
        isEnum = src.isEnum;
        enumId = src.enumId;
        documentation = src.documentation;
        hasContents = src.hasContents;
        contents = src.contents;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaFieldDefinition::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaFieldDefinition( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaFieldDefinition::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaFieldDefinition::Serialize( CORE::CDataNode& domRootNode                        , 
                                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::UInt8 
CKaitaiSchemaFieldDefinition::StringToEndianValue( const CORE::CString& str )
{GUCEF_TRACE;

    bool isLittleEndian = str.HasSubstr( "le" ) > -1;
    bool isBigEndian = str.HasSubstr( "be" ) > -1;

    if ( isLittleEndian )
        return GUCEF_BYTEORDER_LITTLE_ENDIAN;
    if ( isBigEndian )
        return GUCEF_BYTEORDER_BIG_ENDIAN;

    return GUCEF_BYTEORDER_UNKNOWN_ENDIAN;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaFieldDefinition::EndianValueToString( CORE::UInt8 value )
{GUCEF_TRACE;

    static const CORE::CString leStr = "le";
    static const CORE::CString beStr = "be";
    
    switch ( value )
    {
        case GUCEF_BYTEORDER_LITTLE_ENDIAN: 
            return leStr;
        case GUCEF_BYTEORDER_BIG_ENDIAN: 
            return beStr;
        default: 
            return CORE::CString::Empty; 
            
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt8 
CKaitaiSchemaFieldDefinition::KaitaiTypeStringToGucefType( const CORE::CString& str )
{GUCEF_TRACE;

    bool isLittleEndian = str.HasSubstr( "le" ) > -1;
    bool isBigEndian = str.HasSubstr( "be" ) > -1;

    if ( isLittleEndian )
        return GUCEF_BYTEORDER_LITTLE_ENDIAN;
    if ( isBigEndian )
        return GUCEF_BYTEORDER_BIG_ENDIAN;

    return GUCEF_BYTEORDER_UNKNOWN_ENDIAN;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaFieldDefinition::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, true );

    type = domRootNode.GetAttributeValueOrChildValueByName( "type", type, true ).AsString( type, true );
    if ( !type.IsNULLOrEmpty() )
        fieldTypeId = KaitaiTypeStringToGucefType(type);
    else
        fieldTypeId = GUCEF_DATATYPE_UNKNOWN;

    const CORE::CDataNode* contentsNode = domRootNode.FindChild( "contents" );
    if ( GUCEF_NULL != contentsNode )
    {
        // In Kaitai the contents field can be different types
        if ( GUCEF_DATATYPE_ARRAY == contentsNode->GetNodeType() )
        {
            //todo
        }
        else
        if ( GUCEF_DATATYPE_STRING == contentsNode->GetNodeType() )
        {
            contents = contentsNode->GetValue();
            hasContents = true;
        }

       
    }

    documentation = domRootNode.GetAttributeValueOrChildValueByName( "doc" ).AsString( CORE::CString::Empty, true);


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


