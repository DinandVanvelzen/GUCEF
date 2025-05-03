///*
// *  gucefKAITAI: Platform module supporting the Kaitai binary description format
// *
// *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
// *
// *  Licensed under the Apache License, Version 2.0 (the "License");
// *  you may not use this file except in compliance with the License.
// *  You may obtain a copy of the License at
// *
// *    http://www.apache.org/licenses/LICENSE-2.0
// *
// *  Unless required by applicable law or agreed to in writing, software
// *  distributed under the License is distributed on an "AS IS" BASIS,
// *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// *  See the License for the specific language governing permissions and
// *  limitations under the License.
// */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
//#include "dvcppstringutils.h"
//#define GUCEF_CORE_DVCPPSTRINGUTILS_H
//#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
//
//#ifndef GUCEF_CORE_CDATANODE_H
//#include "CDataNode.h"
//#define GUCEF_CORE_CDATANODE_H
//#endif /* GUCEF_CORE_CDATANODE_H ? */
//
//#include "gucefKAITAI_CKaitaiSchemaFieldDefinition.h"
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCEF {
//namespace KAITAI {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      GLOBAL VARS                                                        //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//const CORE::CString CKaitaiSchemaBaseField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaBaseField";
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      IMPLEMENTATION                                                     //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( void )
//    : CORE::CIDataNodeSerializable()
//    , id()
//    , type()
//    , endianess( GUCEF_BYTEORDER_UNKNOWN_ENDIAN )
//    , fieldTypeId( 0 )
//    , hasSize( false )
//    , size( 0 )
//    , isEnum( false )
//    , enumId()
//    , documentation()
//    , hasContents( false )
//    , contents()
//{GUCEF_TRACE;
//}
//
///*-------------------------------------------------------------------------*/
//
//CKaitaiSchemaBaseField::CKaitaiSchemaBaseField( const CKaitaiSchemaBaseField& src )    
//    : CORE::CIDataNodeSerializable( src )
//    , id( src.id )
//    , type( src.type )
//    , endianess( src.endianess )
//    , fieldTypeId( src.fieldTypeId )
//    , hasSize( src.hasSize )
//    , size( src.size )
//    , isEnum( src.isEnum )
//    , enumId( src.enumId )
//    , documentation( src.documentation )
//    , hasContents( src.hasContents )
//    , contents( src.contents )
//{GUCEF_TRACE;
//}
//
///*-------------------------------------------------------------------------*/
//
//CKaitaiSchemaBaseField& 
//CKaitaiSchemaBaseField::operator=( const CKaitaiSchemaBaseField& src )
//{GUCEF_TRACE;
//
//    if ( this != &src )
//    {
//        CORE::CIDataNodeSerializable::operator=( src );
//        id = src.id;
//        type = src.type;
//        endianess = src.endianess;
//        fieldTypeId = src.fieldTypeId;
//        hasSize = src.hasSize;
//        size = src.size;
//        isEnum = src.isEnum;
//        enumId = src.enumId;
//        documentation = src.documentation;
//        hasContents = src.hasContents;
//        contents = src.contents;
//    }
//    return *this;
//}
//
///*-------------------------------------------------------------------------*/
//
//CORE::CICloneable* 
//CKaitaiSchemaBaseField::Clone( void ) const
//{GUCEF_TRACE;
//
//    return GUCEF_NEW CKaitaiSchemaBaseField( *this );
//}
//
///*-------------------------------------------------------------------------*/
//
//const CORE::CString& 
//CKaitaiSchemaBaseField::GetClassTypeName( void ) const
//{GUCEF_TRACE;
//
//    return ClassTypeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool 
//CKaitaiSchemaBaseField::Serialize( CORE::CDataNode& domRootNode                        , 
//                                   const CORE::CDataNodeSerializableSettings& settings ) const
//{GUCEF_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//CORE::UInt8 
//CKaitaiSchemaBaseField::StringToEndianValue( const CORE::CString& str )
//{GUCEF_TRACE;
//
//    bool isLittleEndian = str.HasSubstr( "le" ) > -1;
//    bool isBigEndian = str.HasSubstr( "be" ) > -1;
//
//    if ( isLittleEndian )
//        return GUCEF_BYTEORDER_LITTLE_ENDIAN;
//    if ( isBigEndian )
//        return GUCEF_BYTEORDER_BIG_ENDIAN;
//
//    return GUCEF_BYTEORDER_UNKNOWN_ENDIAN;
//}
//
///*-------------------------------------------------------------------------*/
//
//const CORE::CString& 
//CKaitaiSchemaBaseField::EndianValueToString( CORE::UInt8 value )
//{GUCEF_TRACE;
//
//    static const CORE::CString leStr = "le";
//    static const CORE::CString beStr = "be";
//    
//    switch ( value )
//    {
//        case GUCEF_BYTEORDER_LITTLE_ENDIAN: 
//            return leStr;
//        case GUCEF_BYTEORDER_BIG_ENDIAN: 
//            return beStr;
//        default: 
//            return CORE::CString::Empty; 
//            
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//CORE::UInt8 
//CKaitaiSchemaBaseField::KaitaiTypeStringToGucefType( const CORE::CString& str )
//{GUCEF_TRACE;
//
//    if ( str.Length() == 2 )
//    {
//        UInt32 firstCodePoint = str[ 0 ];
//        if ( firstCodePoint == 'u' )
//        {
//            switch ( str[ 1 ] )
//            {
//                case '1': return GUCEF_DATATYPE_UINT8;
//                case '2': return GUCEF_DATATYPE_UINT16;
//                case '4': return GUCEF_DATATYPE_UINT32;
//                case '8': return GUCEF_DATATYPE_UINT64;
//            }
//        }
//        else
//        if ( firstCodePoint == 'i' )
//        {
//            switch ( str[ 1 ] )
//            {
//                case '1': return GUCEF_DATATYPE_INT8;
//                case '2': return GUCEF_DATATYPE_INT16;
//                case '4': return GUCEF_DATATYPE_INT32;
//                case '8': return GUCEF_DATATYPE_INT64;
//            }
//        }
//        else
//        if ( firstCodePoint == 'f' )
//        {
//            switch ( str[ 1 ] )
//            {
//                case '4': return GUCEF_DATATYPE_FLOAT32;
//                case '8': return GUCEF_DATATYPE_FLOAT64;
//            }
//        }
//    }
//
//    return GUCEF_DATATYPE_UNKNOWN;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool 
//CKaitaiSchemaBaseField::Deserialize( const CORE::CDataNode& domRootNode                  , 
//                                     const CORE::CDataNodeSerializableSettings& settings )
//{GUCEF_TRACE;
//    
//    id = domRootNode.GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, false );
//
//    type = domRootNode.GetAttributeValueOrChildValueByName( "type", type, true ).AsString( type, false );
//    if ( !type.IsNULLOrEmpty() )
//        fieldTypeId = KaitaiTypeStringToGucefType(type);
//    else
//        fieldTypeId = GUCEF_DATATYPE_UNKNOWN;
//
//    if ( GUCEF_DATATYPE_UNKNOWN == fieldTypeId )
//    {
//        CORE::CString sizeStr = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsString();
//        if ( sizeStr.IsNULLOrEmpty() )
//        {
//            size.propertyType = FIELD_PROPERTY_TYPE_NOT_SPECIFIED;
//            size.union_data.specifiedUInt32 = CORE::StringToUInt32( sizeStr );
//        }
//        if ( CORE::IsANumber( sizeStr ) )
//        {
//            size.propertyType = FIELD_PROPERTY_TYPE_SPECIFIED_UINT32;
//            size.union_data.specifiedUInt32 = CORE::StringToUInt32( sizeStr );
//            hasSize = true;
//        }
//        else
//        {
//            hasSize = false;
//        }
//    }
//    else
//    {
//        size = CORE::CVariant::ByteSizeOfFixedSizeType( fieldTypeId );
//    }
//
//    const CORE::CDataNode* contentsNode = domRootNode.FindChild( "contents" );
//    if ( GUCEF_NULL != contentsNode )
//    {
//        // In Kaitai the contents field can be different types
//        if ( GUCEF_DATATYPE_ARRAY == contentsNode->GetNodeType() )
//        {
//            //todo
//        }
//        else
//        if ( GUCEF_DATATYPE_STRING == contentsNode->GetNodeType() )
//        {
//            contents = contentsNode->GetValue();
//            hasContents = true;
//        }
//
//       
//    }
//
//    documentation = domRootNode.GetAttributeValueOrChildValueByName( "doc" ).AsString( CORE::CString::Empty, true);
//
//
//    return false;
//}
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace KAITAI */
//}; /* namespace GUCEF */
//
///*-------------------------------------------------------------------------*/
//
//
