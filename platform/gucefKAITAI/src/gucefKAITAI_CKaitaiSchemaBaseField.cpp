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
                                                CKaitaiSchemaBaseFieldPtr parent  )
    : CORE::CIDataNodeSerializable()
    , gucefDataType( GUCEF_DATATYPE_UNKNOWN )
    , m_fieldType( fieldType )
    , m_parent( parent )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src )    
    : CORE::CIDataNodeSerializable( src )
    , id( src.id )
    , type( src.type )
    , gucefDataType( src.gucefDataType )
    , m_fieldType( src.m_fieldType )
    , m_parent( src.m_parent )
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
        m_parent = src.m_parent;
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
    m_parent.Unlink();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBaseField::GetSchemaFamily( void ) const
{GUCEF_TRACE;

   CKaitaiSchemaMetaPtr schemaMeta = GetSchemaMeta();
   if ( !schemaMeta.IsNULL() )
       return schemaMeta->GetSchemaFamily();
   return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaBaseField::GetSchemaId( void ) const
{GUCEF_TRACE;

   CKaitaiSchemaMetaPtr schemaMeta = GetSchemaMeta();
   if ( !schemaMeta.IsNULL() )
       return schemaMeta->GetSchemaId();
   return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaBaseField::IsLittleEndian( void ) const
{GUCEF_TRACE;

   CKaitaiSchemaMetaPtr schemaMeta = GetSchemaMeta();
   if ( !schemaMeta.IsNULL() )
       return schemaMeta->IsLittleEndian();
   return true;
}

/*-------------------------------------------------------------------------*/
bool 
CKaitaiSchemaBaseField::IsBigEndian( void ) const
{GUCEF_TRACE;

   CKaitaiSchemaMetaPtr schemaMeta = GetSchemaMeta();
   if ( !schemaMeta.IsNULL() )
       return schemaMeta->IsBigEndian();
   return true;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaMetaPtr 
CKaitaiSchemaBaseField::GetSchemaMeta( void ) const
{GUCEF_TRACE;

    CKaitaiSchemaBaseFieldPtr root = GetRootParent();
    if ( !root.IsNULL() )
    {
        // The root should be the schema object itself
        return root->GetSchemaMeta();
    }
    return CKaitaiSchemaMetaPtr();
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

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::AsSharedPtr( void ) const
{GUCEF_TRACE;

    switch ( m_fieldType )
    {
        case NumericScalarField: return static_cast< const CKaitaiSchemaNumericScalarField* >( this )->CreateSharedPtr();
        case StringScalarField: return static_cast< const CKaitaiSchemaStringScalarField* >( this )->CreateSharedPtr();
        case BinaryScalarField: return static_cast< const CKaitaiSchemaBinaryScalarField* >( this )->CreateSharedPtr();

        case EnumDefinition: return static_cast< const CKaitaiSchemaEnumDefinition* >( this )->CreateSharedPtr();
        
        case SwitchLogic: return static_cast< const CKaitaiSchemaSwitchLogic* >( this )->CreateSharedPtr();
        case RepeatLogic: return static_cast< const CKaitaiSchemaRepeatLogic* >( this )->CreateSharedPtr();
        case ConditionalLogic: return static_cast< const CKaitaiSchemaConditionalLogic* >( this )->CreateSharedPtr();
        case LogicInstance: return static_cast< const CKaitaiSchemaLogicInstance* >( this )->CreateSharedPtr();

        case SubstreamField: return static_cast< const CKaitaiSchemaSubstreamField* >( this )->CreateSharedPtr();
        case OpaqueField: return static_cast< const CKaitaiSchemaOpaqueField* >( this )->CreateSharedPtr();
        case DelimitedField: return static_cast< const CKaitaiSchemaDelimitedField* >( this )->CreateSharedPtr();
        case StructureField: return static_cast< const CKaitaiSchemaStructureField* >( this )->CreateSharedPtr();

        case BaseField:
        case UnknownField:
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

                CKaitaiSchemaBaseFieldPtr fieldObj = TryGetReferencedElement( typeName, AsSharedPtr(), false );
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
                                                              CKaitaiSchemaBaseFieldPtr parent  )
{GUCEF_TRACE;

    switch ( fieldType )
    {
        case BaseField:
        {
            CKaitaiSchemaBaseFieldTypedPtr baseField( GUCEF_NEW CKaitaiSchemaBaseField( BaseField, parent ) );
            return baseField;
        }

        case NumericScalarField: return CKaitaiSchemaNumericScalarField::CreateSharedObjWithParam( parent );
        case EnumScalarField: return CKaitaiSchemaEnumScalarField::CreateSharedObjWithParam( parent );
        case StringScalarField: return CKaitaiSchemaStringScalarField::CreateSharedObjWithParam( parent );
        case BinaryScalarField: return CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( parent );
        case ConstValidationField: return CKaitaiSchemaConstValidationScalarField::CreateSharedObjWithParam( parent );

        case EnumDefinition: return CKaitaiSchemaEnumDefinition::CreateSharedObjWithParam( parent );

        case SwitchLogic: return CKaitaiSchemaSwitchLogic::CreateSharedObjWithParam( parent );
        case RepeatLogic: return CKaitaiSchemaRepeatLogic::CreateSharedObjWithParam( parent );
        case ConditionalLogic: return CKaitaiSchemaConditionalLogic::CreateSharedObjWithParam( parent );
        case LogicInstance: return CKaitaiSchemaLogicInstance::CreateSharedObjWithParam( parent );

        case SubstreamField: return CKaitaiSchemaSubstreamField::CreateSharedObjWithParam( parent );
        case OpaqueField: return CKaitaiSchemaOpaqueField::CreateSharedObjWithParam( parent );
        case DelimitedField: return CKaitaiSchemaDelimitedField::CreateSharedObjWithParam( parent );
        case StructureField: return CKaitaiSchemaStructureField::CreateSharedObjWithParam( parent );

        case UnknownField:
        {
            // This type is a placeholder for a type that is not known
            // it is not intended to help data deserialization but rather allow for a schema to be
            // interpreted 'best effort' and also be displayed as such
            CKaitaiSchemaBaseFieldTypedPtr dummyField( GUCEF_NEW CKaitaiSchemaBaseField( UnknownField, parent ) );
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
CKaitaiSchemaBaseField::CreateDefaultFieldObjectForBuildInFieldTypeName( const CORE::CString& typeName    ,
                                                                         CKaitaiSchemaBaseFieldPtr parent )
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
            return CreateDefaultFieldObjectForFieldType( NumericScalarField, parent );
        }
        case GUCEF_DATATYPE_ASCII_STRING:
        case GUCEF_DATATYPE_UTF8_STRING:
        case GUCEF_DATATYPE_UTF16_LE_STRING:
        case GUCEF_DATATYPE_UTF16_BE_STRING:
        case GUCEF_DATATYPE_UTF32_LE_STRING:
        case GUCEF_DATATYPE_UTF32_BE_STRING:
        {
            return CreateDefaultFieldObjectForFieldType( StringScalarField, parent );
        }
        case GUCEF_DATATYPE_BINARY_BSOB:
        case GUCEF_DATATYPE_BINARY_BLOB:
        {
            return CreateDefaultFieldObjectForFieldType( BinaryScalarField, parent );
        }
        case GUCEF_DATATYPE_ENUM:
        {
            return CreateDefaultFieldObjectForFieldType( EnumScalarField, parent );
        }

        default:
        {
            return CKaitaiSchemaBaseFieldPtr();
        }        
    }
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchemaBaseField::IsSchemaElementALogicalOne( KaitaiSchemaElementType elementType )
{GUCEF_TRACE;

    switch ( elementType )
    {
        case SwitchLogic:
        case RepeatLogic:
        case ConditionalLogic:
        case EnumDefinition:
            return true;
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::TryGetNonLogicalParent( CKaitaiSchemaBaseFieldPtr thisElement )
{GUCEF_TRACE;

    if ( !thisElement.IsNULL() )
    {
        CKaitaiSchemaBaseFieldPtr parent = thisElement->GetParent();
        if ( !parent.IsNULL() )
        {
            KaitaiSchemaElementType parentType = parent->GetFieldType();
            if ( IsSchemaElementALogicalOne( parentType ) )
            {
                // logical elements simply wrap a field in actionable logic 
                // per the purpose of this function we will ignore them
                parent = parent->GetParent();
                if ( parent.IsNULL() )
                    return CKaitaiSchemaBaseFieldPtr();
                parentType = parent->GetFieldType();
            }
        }
        return parent;
    }

    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::TryGetRootSchemaObj( CKaitaiSchemaBaseFieldPtr fromElement )
{GUCEF_TRACE;

    if ( !fromElement.IsNULL() )
    {
        CKaitaiSchemaBaseFieldPtr rootObj = fromElement->GetRootParent();
        if ( !rootObj.IsNULL() )
        {
            if ( rootObj->GetFieldType() == Schema ) // better safe than sorry
            {
                // We cannot cast here since the type is not know technically at this base class
                // but anyone using this function can now safely cast it to a schema object
                return rootObj;
            }
        }
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::TryGetReferencedElement( const CORE::CString& typeName         ,
                                                 CKaitaiSchemaBaseFieldPtr thisElement ,
                                                 bool currentScopeOnly                 )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( thisElement.IsNULL() || typeName.IsNULLOrEmpty() ) // input sanity check
        return CKaitaiSchemaBaseFieldPtr();
    
    // Order of precedence for non-fully-qualified references in KAITAI:
    //  1 - Instance variables
    //  2 - Fields in the same 'seq' section
    //  3 - Enums
    //  4 - Types
    //  5 - root level enums (if not already at root)
    //  6 - root level types (if not already at root)
    // 
    //  Fully qualified names do not follow this order of precedence in the local scope
    //  rather they obey the scope as specified per their namespace qualification
    //  Note that root level 'instances' require a fully qualified name to be used from a nested scope
    // 
    //  When using fully qualified names you also have to consider imports
    //  this yields the following order of precedence:
    //    1 - Instance variables as per the full qualification, the leaf node of the namespace 
    //    2 - Fields names as per the full qualification, the leaf node of the namespace
    //    3 - Enums as per the full qualification, the leaf node of the namespace
    //    4 - Types as per the full qualification, the leaf node of the namespace
    //    5 - Imported Types as per the full qualification, the leaf node of the namespace
    //
    // Hence:
    // - If :: is used, we should not search for the type in the default resolution order 
    //      (local instances → fields → enums → types → imports)
    //   but rather obey the explicit type namespacing from the root as directed by the double colon notation 
    //   :: separated names always start at the schema root, explicitly following a root-based namespacing approach in Kaitai Struct. This ensures that:
    //      - Every :: separated name is resolved from the schema root.
    //      - Nested types and imported types are fully qualified, preventing ambiguity.
    //      - There is no implicit hierarchical traversal—the name explicitly states its full path.
    //  Same thing for _root and _parent prefixed names, they are always fully qualified and start as per the namespace qualification
    //

    if ( !currentScopeOnly )
    {
        if ( typeName.HasSubstr( "::" ) > -1 )
        {
            // The type name is a fully qualified name
            // We need to split it up and check if the first part is a valid import
            // If so we can use the import to resolve the rest of the name
            CKaitaiSchemaPtr schema = TryGetRootSchemaObj( thisElement ).StaticCast< CKaitaiSchema >();
            if ( !schema.IsNULL() )
            {
                return schema->TryGetReferencedFullyQualifiedElement( typeName );
            }

            // We cannot resolve the type name
            // fully qualified names are not allowed to be used in the default resolution order
            return CKaitaiSchemaBaseFieldPtr();
        }
        else
        if ( typeName.StartsWith( "_root." ) )
        {
            // The type name is a fully qualified reference to a root type
            CKaitaiSchemaBaseFieldPtr schema = TryGetRootSchemaObj( thisElement );
            if ( !schema.IsNULL() )
            {
                CORE::CString rootTypeName = typeName.CutChars( 6 ); // remove the _root. prefix
                return schema->TryGetReferencedElement( rootTypeName, schema, true );
            }

            // We cannot resolve the type name
            // fully qualified names are not allowed to be used in the default resolution order
            return CKaitaiSchemaBaseFieldPtr();        
        }
        else
        if ( typeName.StartsWith( "_parent." ) )
        {
            // The type name is a fully qualified reference to a type namespaced to the parent
            CKaitaiSchemaBaseFieldPtr parent = TryGetNonLogicalParent( thisElement );
            if ( !parent.IsNULL() )
            {
                CORE::CString parentScopeTypeName = typeName.CutChars( 8 ); // remove the _parent. prefix
                return parent->TryGetReferencedElement( parentScopeTypeName, thisElement, true );
            }

            // We cannot resolve the type name
            // fully qualified names are not allowed to be used in the default resolution order
            return CKaitaiSchemaBaseFieldPtr();        
        }
    }
    
    // Try to resolve the type name in the default resolution order
    // we first check the current scope
    CKaitaiSchemaBaseFieldPtr parent = TryGetNonLogicalParent( thisElement );
    if ( !parent.IsNULL() )
    {
        KaitaiSchemaElementType parentType = parent->GetFieldType();
        switch ( parentType )
        {
            case StructureField:
            {
                CKaitaiSchemaStructureFieldPtr parentStructure = parent.StaticCast< CKaitaiSchemaStructureField >();
                CKaitaiSchemaBaseFieldPtr scopeInstance = parentStructure->TryGetLocalScopeElement( typeName );
                if ( !scopeInstance.IsNULL() )
                    return scopeInstance;
                break;
            }
            case Schema:
            {
                CKaitaiSchemaPtr parentSchema = parent.StaticCast< CKaitaiSchema >();
                CKaitaiSchemaBaseFieldPtr scopeInstance = parentSchema->TryGetLocalScopeElement( typeName );
                if ( !scopeInstance.IsNULL() )
                    return scopeInstance;
                
                // Since the parent is already the schema, no need for further checks
                return CKaitaiSchemaStructureFieldPtr();
            }
            default:
                break; // no action needed. 'same scope' precedence choices are not applicable
        }
    }

    if ( !currentScopeOnly )
    {
        // We were unable to resolve the type name in the current scope
        // We will now check the root scope since enums and types defined there are also valid 
        // as a lower precedence choice
        CKaitaiSchemaPtr parentSchema = TryGetRootSchemaObj( thisElement ).StaticCast< CKaitaiSchema >();
        if ( !parentSchema.IsNULL() )
        {
            CKaitaiSchemaBaseFieldPtr scopeInstance = parentSchema->TryGetLocalScopeElement( typeName );
            if ( !scopeInstance.IsNULL() )
                return scopeInstance;
        }
    }

    // Nothing found that qualifies for the type name given
    return CKaitaiSchemaStructureFieldPtr();
}
/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::CreateFieldObjectForFieldTypeStr( const CORE::CString& typeName         ,
                                                          CKaitaiSchemaBaseFieldPtr thisElement )
{GUCEF_TRACE;

    CKaitaiSchemaBaseFieldPtr parent = TryGetNonLogicalParent( thisElement );

    // First check for build-in types
    // You should not be able to override build-in types
    CKaitaiSchemaBaseFieldPtr fieldObj = CreateDefaultFieldObjectForBuildInFieldTypeName( typeName, parent );
    if ( fieldObj.IsNULL() )
    {
        CKaitaiSchemaBaseFieldPtr templateObj = TryGetReferencedElement( typeName, thisElement, false );
        if ( !templateObj.IsNULL() )
            return templateObj->CloneAsFieldObject();
    }
    return fieldObj;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::GetParent( void ) const
{GUCEF_TRACE;

    return m_parent;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaBaseField::GetRootParent( void ) const
{GUCEF_TRACE;

    CKaitaiSchemaBaseFieldPtr parent = m_parent;
    while ( !parent.IsNULL() )
    {
        CKaitaiSchemaBaseFieldPtr parentParent = parent->GetParent();
        if ( parentParent.IsNULL() )
            break;
        parent = parentParent;
    }
    return parent;
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


