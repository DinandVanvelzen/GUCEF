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

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

#include "gucefKAITAI_CKaitaiSchemaStructureField.h"

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

const CORE::CString CKaitaiSchemaStructureField::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchemaStructureField";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::CKaitaiSchemaStructureField( void )
    : CKaitaiSchemaBaseField( StructureField, CKaitaiSchemaMeta::CreateSharedObj() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::CKaitaiSchemaStructureField( CKaitaiSchemaMetaPtr schemaMeta )
    : CKaitaiSchemaBaseField( StructureField, schemaMeta )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::CKaitaiSchemaStructureField( const CKaitaiSchemaStructureField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::~CKaitaiSchemaStructureField()
{GUCEF_TRACE;
    // Nothing to do here
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField& 
CKaitaiSchemaStructureField::operator=( const CKaitaiSchemaStructureField& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );

    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaStructureField::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();
    fields.clear();
    params.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchemaStructureField::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchemaStructureField( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchemaStructureField::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaStructureField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    // A structure field can only be fixed size if all its fields are fixed size
    Int32 totalSize = 0;
    CKaitaiSchemaBaseFieldPtrVector::const_iterator i = fields.begin();
    while ( i != fields.end() )
    {
        const CKaitaiSchemaBaseFieldPtr& field = (*i);
        if ( !field.IsNULL() )
        {
            Int32 fieldSize = field->GetFixedSizeIfAny();
            if ( fieldSize < 0 )
                return -1; // not fixed size
            totalSize += fieldSize;
        }
        else
        {
            return -1; // not fixed size
        }
        ++i;
    }
    return totalSize;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStructureField::Serialize( CORE::CDataNode& domRootNode                        , 
                                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStructureField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;    
    
    id = domRootNode.GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, false );

    const CORE::CDataNode* seqNode = domRootNode.FindChild( "seq" );
    if ( GUCEF_NULL != seqNode )
    {        
        if GUCEF_PREDICT_FALSE( 0 == seqNode->GetNrOfDirectChildNodes() )
        {
            // sequence section with no children is malformed
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Sequence section with no children is malformed. id=" + id );
            return false;
        }
        
        bool totalSuccess = true;
        CORE::CDataNode::const_iterator i = seqNode->ConstBegin();
        while ( i != seqNode->ConstEnd() )
        {
            const CORE::CDataNode* fieldNode = (*i);
            if ( GUCEF_NULL != fieldNode )
            {
                CORE::CString fieldTypeStr = fieldNode->GetAttributeValueOrChildValueByName( "type" ).AsString();
                if ( !fieldTypeStr.IsNULLOrEmpty() )
                {
                    CKaitaiSchemaBaseFieldPtr field = CreateFieldObjectForFieldTypeStr( fieldTypeStr, GetSchemaMeta() );
                    if GUCEF_PREDICT_FALSE( field.IsNULL() )
                    {                                                
                        totalSuccess = false;

                        // In order to do 'best effort' deserialization we will skip the field
                        // However due to field ordering we cannot just leave the slot empty
                        // we have to make it expliciy there is an error location in the sequence
                        // We will create a dummy field object to fill the gap
                        field = CreateDefaultFieldObjectForFieldType( UnknownField, GetSchemaMeta() );
                        if ( !field.IsNULL() )
                        {
                            field->type = fieldTypeStr;
                            fields.push_back( field );    
                        }

                        ++i;
                        continue;
                    }

                    if GUCEF_PREDICT_FALSE( !field->Deserialize( *fieldNode, settings ) )
                    {
                        totalSuccess = false;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize field spec of type " + fieldTypeStr );
                    }
                    fields.push_back( field );  // @TODO: is using an UnknownField placeholder the better choice on failure ?
                }
                else
                {                    
                    // We do not have a type, the type can be implicit via 'size' which means its a binary payload
                    CORE::CString sizeValue = fieldNode->GetAttributeValueOrChildValueByName( "size" ).AsString();
                    if ( !sizeValue.IsNULLOrEmpty() )
                    {
                        // we have a binary field
                        CKaitaiSchemaBinaryScalarFieldPtr field = CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( m_schemaMeta );
                        if ( !field.IsNULL() )
                        {
                            if GUCEF_PREDICT_FALSE( !field->Deserialize( *fieldNode, settings ) )
                            {
                                totalSuccess = false;
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize binary field spec with size " + sizeValue );
                            }
                            fields.push_back( field );  // @TODO: is using an UnknownField placeholder the better choice on failure ?  
                        }
                        else
                        {
                            // dont assume we can recover from allocation failures
                            return false;
                        }
                    }
                    else
                    {
                        // Check if this is a switch section
                        const CORE::CDataNode* switchOnNode = fieldNode->Find( "switch-on" );
                        if ( GUCEF_NULL != switchOnNode )
                        {
                            // We have a switch section
                            CKaitaiSchemaSwitchFieldPtr switchElement = CKaitaiSchemaSwitchField::CreateSharedObjWithParam( m_schemaMeta );
                            if GUCEF_PREDICT_FALSE( switchElement.IsNULL() )
                            {
                                // dont assume we can recover from allocation failures
                                return false;
                            }

                            if GUCEF_PREDICT_FALSE( !switchElement->Deserialize( *switchOnNode, settings ) )
                            {
                                totalSuccess = false;
                                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize switch statement" );
                            }
                        }
                    }
                }
            }
            ++i;
        }
        
        return true;
    }
    return false;



    //type = domRootNode.GetAttributeValueOrChildValueByName( "type", type, true ).AsString( type, false );
    //if ( !type.IsNULLOrEmpty() )
    //    fieldTypeId = KaitaiTypeStringToGucefType(type);
    //else
    //    fieldTypeId = GUCEF_DATATYPE_UNKNOWN;

    //if ( GUCEF_DATATYPE_UNKNOWN == fieldTypeId )
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
    //    size = CORE::CVariant::ByteSizeOfFixedSizeType( fieldTypeId );
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
