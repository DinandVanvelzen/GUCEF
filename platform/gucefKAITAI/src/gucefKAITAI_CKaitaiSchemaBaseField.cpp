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

#ifndef GUCEF_KAITAI_CKAITAISCHEMA_H
#include "gucefKAITAI_CKaitaiSchema.h"
#define GUCEF_KAITAI_CKAITAISCHEMA_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */

#include "gucefKAITAI_CKaitaiSchemaBaseField.h"

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

const CORE::CString CKaitaiSchemaBaseField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaBaseField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

KaitaiSchemaFieldType 
CKaitaiSchemaBaseField::GetFieldType( void ) const
{GUCEF_TRACE;

    return BaseField;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( KaitaiSchemaFieldType fieldType )
    : CORE::CIDataNodeSerializable()
    , id()
    , type()
    , isLittleEndian( true )
    , gucefDataType( GUCEF_DATATYPE_UNKNOWN )
    , validExpression()
    , m_fieldType( fieldType )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src )    
    : CORE::CIDataNodeSerializable( src )
    , id( src.id )
    , type( src.type )
    , isLittleEndian( src.isLittleEndian )
    , gucefDataType( src.gucefDataType )
    , validExpression( src.validExpression )
    , m_fieldType( src.m_fieldType )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField& 
CKaitaiSchemaBaseField::operator=( const CKaitaiSchemaBaseField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        id = src.id;
        type = src.type;
        isLittleEndian = src.isLittleEndian;
        gucefDataType = src.gucefDataType;
        validExpression = src.validExpression;
        m_fieldType = src.m_fieldType;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaBaseField::Clear( void )
{GUCEF_TRACE;

    id.Clear();
    type.Clear();
    isLittleEndian = true;
    gucefDataType = GUCEF_DATATYPE_UNKNOWN;
    validExpression.Clear();
    m_fieldType = BaseField;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaBaseField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaBaseField( *this );
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::CloneAsFieldObject( void ) const
{GUCEF_TRACE;

    switch ( m_fieldType )
    {
        case BaseField: return CKaitaiSchemaBaseFieldTypedPtr( GUCEF_NEW CKaitaiSchemaBaseField( *this ) );
        case UnknownField: return CKaitaiSchemaBaseFieldTypedPtr( GUCEF_NEW CKaitaiSchemaBaseField( *this ) );
        case BasicField: return CKaitaiSchemaBasicField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaBasicField* >( this ) );
        case SwitchField: return CKaitaiSchemaSwitchField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaSwitchField* >( this ) );
        case InstanceField: return CKaitaiSchemaInstanceField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaInstanceField* >( this ) );
        case SubstreamField: return CKaitaiSchemaSubstreamField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaSubstreamField* >( this ) );
        case OpaqueField: return CKaitaiSchemaOpaqueField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaOpaqueField*>( this ) );
        case DelimitedField: return CKaitaiSchemaDelimitedField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaDelimitedField*>( this ) );
        case StructureField: return CKaitaiSchemaStructureField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaStructureField*>( this ) );

        default:
            return CKaitaiSchemaBaseFieldPtr();
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBaseField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::Serialize( CORE::CDataNode& domRootNode                        , 
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::UInt8 
CKaitaiSchemaBaseField::StringToEndianValue( const CORE::CString& str )
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
CKaitaiSchemaBaseField::EndianValueToString( CORE::UInt8 value )
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
CKaitaiSchemaBaseField::KaitaiBuildInTypeStringToGucefType( const CORE::CString& typeName )
{GUCEF_TRACE;

    if ( typeName.Length() == 2 )
    {
        UInt32 firstCodePoint = typeName[ 0 ];
        if ( firstCodePoint == 'u' )
        {
            switch ( typeName[ 1 ] )
            {
                case '1': return GUCEF_DATATYPE_UINT8;
                case '2': return GUCEF_DATATYPE_UINT16;
                case '4': return GUCEF_DATATYPE_UINT32;
                case '8': return GUCEF_DATATYPE_UINT64;
            }
        }
        else
        if ( firstCodePoint == 'i' )
        {
            switch ( typeName[ 1 ] )
            {
                case '1': return GUCEF_DATATYPE_INT8;
                case '2': return GUCEF_DATATYPE_INT16;
                case '4': return GUCEF_DATATYPE_INT32;
                case '8': return GUCEF_DATATYPE_INT64;
            }
        }
        else
        if ( firstCodePoint == 'f' )
        {
            switch ( typeName[ 1 ] )
            {
                case '4': return GUCEF_DATATYPE_FLOAT32;
                case '8': return GUCEF_DATATYPE_FLOAT64;
            }
        }
    }

    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr
CKaitaiSchemaBaseField::CreateDefaultFieldObjectForFieldType( KaitaiSchemaFieldType fieldType )
{GUCEF_TRACE;

    switch ( fieldType )
    {
        case BaseField:
        case BasicField:
        {
            // the type is scalar and we can return the corresponding KaitaiSchemaFieldType
            return CKaitaiSchemaBasicField::CreateSharedObj();
        }
        //case ConditionalField:
        //{
        //    // the type is scalar and we can return the corresponding KaitaiSchemaFieldType
        //    return CKaitaiSchemaConditionalField::CreateSharedObj();
        //}

        //case RepeatedField:
        //{
        //    // the type is scalar and we can return the corresponding KaitaiSchemaFieldType
        //    return CKaitaiSchemaRepeatedField::CreateSharedObj();
        //}
        case SwitchField:
        {
            return CKaitaiSchemaSwitchField::CreateSharedObj();
        }
        case InstanceField:
        {
            return CKaitaiSchemaSwitchField::CreateSharedObj();
        }
        case SubstreamField:
        {
            return CKaitaiSchemaSubstreamField::CreateSharedObj();
        }
        //case BitField,
        //case EnumField,
        case OpaqueField:
        {
            return CKaitaiSchemaOpaqueField::CreateSharedObj();
        }
        case DelimitedField:
        {
            return CKaitaiSchemaDelimitedField::CreateSharedObj();
        }
        case StructureField:
        {
            return CKaitaiSchemaStructureField::CreateSharedObj();
        }

        case UnknownField:
        {
            // This type is a placeholder for a type that is not known
            // it is not intended to help data deserialization but rather allow for a schema to be
            // interpreted 'best effort' and also be displayed as such
            CKaitaiSchemaBaseFieldTypedPtr dummyField( GUCEF_NEW CKaitaiSchemaBaseField( UnknownField ) );
            if ( !dummyField.IsNULL() )
            {
                dummyField->id = "UnknownField";
                dummyField->type = "UnknownField";
                dummyField->gucefDataType = GUCEF_DATATYPE_UNKNOWN;
                dummyField->isLittleEndian = true;
            }
            return dummyField;
        }
        
        default:
            return CKaitaiSchemaBaseFieldPtr();
    }
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::CreateDefaultFieldObjectForBuildInFieldTypeName( const CORE::CString& typeName )
{GUCEF_TRACE;

    CORE::UInt8 gupDataType = KaitaiBuildInTypeStringToGucefType( typeName );
    switch ( gupDataType )
    {
        case GUCEF_DATATYPE_UINT8:
        case GUCEF_DATATYPE_UINT16:
        case GUCEF_DATATYPE_UINT32:
        case GUCEF_DATATYPE_UINT64:
        case GUCEF_DATATYPE_INT8:
        case GUCEF_DATATYPE_INT16:
        case GUCEF_DATATYPE_INT32:
        case GUCEF_DATATYPE_INT64:
        case GUCEF_DATATYPE_FLOAT32:
        case GUCEF_DATATYPE_FLOAT64:
        {
            return CreateDefaultFieldObjectForFieldType( BasicField );
        }

        default:
        {
            return CKaitaiSchemaBaseFieldPtr();
        }        
    }
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, false );

    //type = domRootNode.GetAttributeValueOrChildValueByName( "type", type, true ).AsString( type, false );
    //if ( !type.IsNULLOrEmpty() )
    //    gucefDataType = KaitaiTypeStringToGucefType( type );
    //else
    //    gucefDataType = GUCEF_DATATYPE_UNKNOWN;

    //if ( GUCEF_DATATYPE_UNKNOWN == gucefDataType )
    //{
    //    CORE::CString sizeStr = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsString();
    //    if ( sizeStr.IsNULLOrEmpty() )
    //    {
    //        size.propertyType = FIELD_PROPERTY_TYPE_NOT_SPECIFIED;
    //        size.union_data.specifiedUInt32 = CORE::StringToUInt32( sizeStr );
    //    }
    //    if ( CORE::IsANumber( sizeStr ) )
    //    {
    //        size.propertyType = FIELD_PROPERTY_TYPE_SPECIFIED_UINT32;
    //        size.union_data.specifiedUInt32 = CORE::StringToUInt32( sizeStr );
    //        hasSize = true;
    //    }
    //    else
    //    {
    //        hasSize = false;
    //    }
    //}
    //else
    //{
    //    size = CORE::CVariant::ByteSizeOfFixedSizeType( gucefDataType );
    //}

    //const CORE::CDataNode* contentsNode = domRootNode.FindChild( "contents" );
    //if ( GUCEF_NULL != contentsNode )
    //{
    //    // In Kaitai the contents field can be different types
    //    if ( GUCEF_DATATYPE_ARRAY == contentsNode->GetNodeType() )
    //    {
    //        //todo
    //    }
    //    else
    //    if ( GUCEF_DATATYPE_STRING == contentsNode->GetNodeType() )
    //    {
    //        contents = contentsNode->GetValue();
    //        hasContents = true;
    //    }

    //   
    //}

    //documentation = domRootNode.GetAttributeValueOrChildValueByName( "doc" ).AsString( CORE::CString::Empty, true);


    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::IsValid( void ) const
{GUCEF_TRACE;

    // Default implementation assumes validation passes
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


