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

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

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

KaitaiSchemaElementType 
CKaitaiSchemaBaseField::GetFieldType( void ) const
{GUCEF_TRACE;

    return BaseField;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( KaitaiSchemaElementType fieldType ,
                                                CKaitaiSchemaMetaPtr schemaMeta   )
    : CORE::CIDataNodeSerializable()
    , gucefDataType( GUCEF_DATATYPE_UNKNOWN )
    , m_fieldType( fieldType )
    , m_schemaMeta( schemaMeta )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src )    
    : CORE::CIDataNodeSerializable( src )
    , id( src.id )
    , type( src.type )
    , gucefDataType( src.gucefDataType )
    , m_fieldType( src.m_fieldType )
    , m_schemaMeta( src.m_schemaMeta )
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
        gucefDataType = src.gucefDataType;
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
    gucefDataType = GUCEF_DATATYPE_UNKNOWN;
    m_fieldType = BaseField;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBaseField::GetSchemaFamily( void ) const
{GUCEF_TRACE;

   if ( !m_schemaMeta.IsNULL() )
       return m_schemaMeta->GetSchemaFamily();
   return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBaseField::GetSchemaId( void ) const
{GUCEF_TRACE;

   if ( !m_schemaMeta.IsNULL() )
       return m_schemaMeta->GetSchemaId();
   return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::IsLittleEndian( void ) const
{GUCEF_TRACE;

   if ( !m_schemaMeta.IsNULL() )
       return m_schemaMeta->IsLittleEndian();
   return true;
}

/*-------------------------------------------------------------------------*/
bool 
CKaitaiSchemaBaseField::IsBigEndian( void ) const
{GUCEF_TRACE;

   if ( !m_schemaMeta.IsNULL() )
       return m_schemaMeta->IsBigEndian();
   return true;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMetaPtr 
CKaitaiSchemaBaseField::GetSchemaMeta( void ) const
{GUCEF_TRACE;

    return m_schemaMeta;
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

        case NumericScalarField: return CKaitaiSchemaNumericScalarField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaNumericScalarField* >( this ) );
        case StringScalarField: return CKaitaiSchemaStringScalarField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaStringScalarField* >( this ) );
        case BinaryScalarField: return CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaBinaryScalarField* >( this ) );

        case EnumDefinition: return CKaitaiSchemaEnumDefinition::CreateSharedObjWithParam(*static_cast<const CKaitaiSchemaEnumDefinition*>(this));
        
        case SwitchLogic: return CKaitaiSchemaSwitchLogic::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaSwitchLogic* >( this ) );
        case RepeatLogic: return CKaitaiSchemaRepeatLogic::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaRepeatLogic* >( this ) );
        case ConditionalLogic: return CKaitaiSchemaConditionalLogic::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaConditionalLogic* >( this ) );
        case LogicInstance: return CKaitaiSchemaLogicInstance::CreateSharedObjWithParam( *static_cast< const CKaitaiSchemaLogicInstance* >( this ) );

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
        switch ( firstCodePoint )
        {
            case 'u':
            {
                switch ( typeName[ 1 ] )
                {
                    case '1': return GUCEF_DATATYPE_UINT8;
                    case '2': return GUCEF_DATATYPE_UINT16;
                    case '4': return GUCEF_DATATYPE_UINT32;
                    case '8': return GUCEF_DATATYPE_UINT64;
                }
                break;
            }
            case 's':
            {
                switch ( typeName[ 1 ] )
                {
                    case '1': return GUCEF_DATATYPE_INT8;
                    case '2': return GUCEF_DATATYPE_INT16;
                    case '4': return GUCEF_DATATYPE_INT32;
                    case '8': return GUCEF_DATATYPE_INT64;
                }
                break;
            }
            case 'f':
            {
                switch ( typeName[ 1 ] )
                {
                    case '4': return GUCEF_DATATYPE_FLOAT32;
                    case '8': return GUCEF_DATATYPE_FLOAT64;
                }
                break;
            }
            case 'b':
            {
                // @todo: add support for Kaitai 'bool' type
                // 'b1' is single bit boolean
                return GUCEF_DATATYPE_BOOLEAN;
            }
        }
    }
    else
    {
        if ( "str" == typeName )
            return GUCEF_DATATYPE_STRING;
        if ( "bytes" == typeName )
            return GUCEF_DATATYPE_BINARY_BLOB;
        if ( "enum" == typeName )
            return GUCEF_DATATYPE_ENUM;
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

Int32 
CKaitaiSchemaBaseField::KaitaiFixedSizeValueStringToFixedSizeIfAny( const CORE::CString& sizeValue ) const
{GUCEF_TRACE;

    // in Kaitai Struct, the type and size properties of a field definition are mutually exclusive. 
    // If you specify a type, Kaitai assumes the field's size is determined by that type. 
    // Conversely, if you specify a size, Kaitai treats the field as a raw byte array of that length, meaning you cannot also define a type.
    Int32 fixedSize = CORE::StringToInt32( sizeValue, -1 );
    if ( fixedSize >= 0 )
    {
        return fixedSize;
    }
    else
    {
        Int32 sizeofKeywordOffset = sizeValue.Lowercase().HasSubstr( "sizeof(" );
        if ( sizeofKeywordOffset >= 0 )
        {
            sizeofKeywordOffset += 7; // The sizeof( keyword is 7 characters long

            // The size is defined by the sizeof() keyword
            // The usage of sizeof() mandates that the field that is referenced is a fixed size field
            // We will need to check the registry for the type as it could be defined in the parent schema
            Int32 sizeofKeywordEndOffset = sizeValue.HasChar( ')', static_cast< UInt32 >( sizeofKeywordOffset ), true );
            if ( sizeofKeywordEndOffset > 0 )
            {
                // Extract the type name from the sizeof() expression
                CORE::CString typeName = sizeValue.SubstrFromRange( sizeofKeywordOffset, sizeofKeywordEndOffset );
                typeName = typeName.Trim( true ).Trim( false );

                CKaitaiGlobal* kaitaiGlobal = CKaitaiGlobal::Instance();
                CKaitaiSchemaRegistry& schemaRegistry = kaitaiGlobal->GetKaitaiSchemaRegistry();
                CKaitaiSchemaBaseFieldPtr fieldObj = schemaRegistry.TryGetSchemaRootOrSubType( GetSchemaFamily(), GetSchemaId(), typeName );
                if ( !fieldObj.IsNULL() )
                {
                    // We have a valid field object
                    // Check if it is fixed size
                    Int32 fieldSize = fieldObj->GetFixedSizeIfAny();
                    if ( fieldSize >= 0 )
                    {
                        return fieldSize;
                    }
                }
            }
        }
    }

    // the size string could be a direct reference to a scalar field
    // in such a case its not fixed size because the scalar field itself is the variable we rely upon
    return -1;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::IsValidPossibleFieldName( const CORE::CString& testStr )
{GUCEF_TRACE;

    if ( testStr.IsNULLOrEmpty() )
        return false;

    // Must start with a letter or underscore
    if ( !std::isalpha( testStr[ 0 ] ) && testStr[ 0 ] != '_' )
        return false;

    // Must contain only letters, digits, underscores, or hyphens
    static const CORE::CString validFieldNameChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
    UInt32 nrOfViolations = testStr.GetNonMatchCharactersCount( validFieldNameChars );
    
    return nrOfViolations == 0;
}

/*-------------------------------------------------------------------------*/

CORE::UInt8 
CKaitaiSchemaBaseField::KaitaiFixedSizeValueStringToGucefType( Int32 fixedSize )
{GUCEF_TRACE;

    if ( fixedSize > 0 )
    {
        if ( fixedSize <= GUCEF_VARIANT_BSOB_SIZE )
            return GUCEF_DATATYPE_BINARY_BSOB;
        else
            return GUCEF_DATATYPE_BINARY_BLOB;
    }
    return GUCEF_DATATYPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

CORE::UInt8 
CKaitaiSchemaBaseField::KaitaiFixedSizeValueStringToGucefType( const CORE::CString& sizeValue ) const
{GUCEF_TRACE;

    Int32 fixedSize = KaitaiFixedSizeValueStringToFixedSizeIfAny( sizeValue );
    return KaitaiFixedSizeValueStringToGucefType( fixedSize );
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr
CKaitaiSchemaBaseField::CreateDefaultFieldObjectForFieldType( KaitaiSchemaElementType fieldType , 
                                                              CKaitaiSchemaMetaPtr schemaMeta   )
{GUCEF_TRACE;

    switch ( fieldType )
    {
        case BaseField:
        {
            CKaitaiSchemaBaseFieldTypedPtr baseField( GUCEF_NEW CKaitaiSchemaBaseField( BaseField, schemaMeta ) );
            return baseField;
        }

        case NumericScalarField: return CKaitaiSchemaNumericScalarField::CreateSharedObjWithParam( schemaMeta );
        case EnumScalarField: return CKaitaiSchemaEnumScalarField::CreateSharedObjWithParam( schemaMeta );
        case StringScalarField: return CKaitaiSchemaStringScalarField::CreateSharedObjWithParam( schemaMeta );
        case BinaryScalarField: return CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( schemaMeta );
        case ConstValidationField: return CKaitaiSchemaConstValidationScalarField::CreateSharedObjWithParam( schemaMeta );

        case EnumDefinition: return CKaitaiSchemaEnumDefinition::CreateSharedObjWithParam( schemaMeta );

        case SwitchLogic: return CKaitaiSchemaSwitchLogic::CreateSharedObjWithParam( schemaMeta );
        case RepeatLogic: return CKaitaiSchemaRepeatLogic::CreateSharedObjWithParam( schemaMeta );
        case ConditionalLogic: return CKaitaiSchemaConditionalLogic::CreateSharedObjWithParam( schemaMeta );
        case LogicInstance: return CKaitaiSchemaLogicInstance::CreateSharedObjWithParam( schemaMeta );

        case SubstreamField: return CKaitaiSchemaSubstreamField::CreateSharedObjWithParam( schemaMeta );
        case OpaqueField: return CKaitaiSchemaOpaqueField::CreateSharedObjWithParam( schemaMeta );
        case DelimitedField: return CKaitaiSchemaDelimitedField::CreateSharedObjWithParam( schemaMeta );
        case StructureField: return CKaitaiSchemaStructureField::CreateSharedObjWithParam( schemaMeta );

        case UnknownField:
        {
            // This type is a placeholder for a type that is not known
            // it is not intended to help data deserialization but rather allow for a schema to be
            // interpreted 'best effort' and also be displayed as such
            CKaitaiSchemaBaseFieldTypedPtr dummyField( GUCEF_NEW CKaitaiSchemaBaseField( UnknownField, schemaMeta ) );
            if ( !dummyField.IsNULL() )
            {
                dummyField->id = "UnknownField";
                dummyField->type = "UnknownField";
                dummyField->gucefDataType = GUCEF_DATATYPE_UNKNOWN;
            }
            return dummyField;
        }
        
        default:
            return CKaitaiSchemaBaseFieldPtr();
    }
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::CreateDefaultFieldObjectForBuildInFieldTypeName( const CORE::CString& typeName   ,
                                                                         CKaitaiSchemaMetaPtr schemaMeta )
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
            return CreateDefaultFieldObjectForFieldType( NumericScalarField, schemaMeta );
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        {
            return CreateDefaultFieldObjectForFieldType( StringScalarField, schemaMeta );
        }
        case GUCEF_DATATYPE_BINARY_BSOB:
        case GUCEF_DATATYPE_BINARY_BLOB:
        {
            return CreateDefaultFieldObjectForFieldType( BinaryScalarField, schemaMeta );
        }
        case GUCEF_DATATYPE_ENUM:
        {
            return CreateDefaultFieldObjectForFieldType( EnumScalarField, schemaMeta );
        }

        default:
        {
            return CKaitaiSchemaBaseFieldPtr();
        }        
    }
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::CreateFieldObjectForFieldTypeStr( const CORE::CString& typeName   ,
                                                          CKaitaiSchemaMetaPtr schemaMeta ) const
{GUCEF_TRACE;

    // First check for build-in types
    // You should not be able to override build-in types
    CKaitaiSchemaBaseFieldPtr fieldObj = CreateDefaultFieldObjectForBuildInFieldTypeName( typeName, schemaMeta );
    if ( fieldObj.IsNULL() && !schemaMeta.IsNULL() )
    {
        // This might be a typedef
        // We will need to check the registry for the type as it could be defined in the parent schema or any of the imports
        CKaitaiGlobal* kaitaiGlobal = CKaitaiGlobal::Instance();
        fieldObj = kaitaiGlobal->GetKaitaiSchemaRegistry().TryGetSchemaRootOrSubType( schemaMeta->GetSchemaFamily() , 
                                                                                      schemaMeta->GetSchemaId()     , 
                                                                                      typeName                      );
    }
    return fieldObj;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchemaBaseField::IsFixedSize( void ) const
{GUCEF_TRACE;

    return GetFixedSizeIfAny() >= 0;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaBaseField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    // The base class is not fixed size because its not a valid type in of itself
    // The derived classes will override this
    return -1;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, false );

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


