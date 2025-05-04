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

const CKaitaiSchemaStructureField::CKaitaiSchemaBaseFieldPtrVector& 
CKaitaiSchemaStructureField::GetFields( void ) const
{GUCEF_TRACE; 

   return fields;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::CreateSchemaObjectForFieldDataNode( const CORE::CDataNode& fieldNode , 
                                                                 bool& totalSuccess               ) const
{GUCEF_TRACE; 
                
    CORE::CDataNodeSerializableSettings defaultSerializerSettings;
    
    // contents tag may be used with or without specifying type
    // regardless we will use the CKaitaiSchemaConstValidationScalarField
    const CORE::CVariant& contents = fieldNode.GetAttributeValueOrChildValueByName( "contents" );
    if ( contents.IsInitialized() )
    {
        CKaitaiSchemaBaseFieldPtr field = CKaitaiSchemaConstValidationScalarField::CreateSharedObjWithParam( GetSchemaMeta() ); 
        if GUCEF_PREDICT_FALSE( field.IsNULL() )        
        {
            // dont assume we can recover from allocation failures
            return CKaitaiSchemaBaseFieldPtr();
        } 
                    
        if GUCEF_PREDICT_FALSE( !field->Deserialize( fieldNode, defaultSerializerSettings ) )
        {
            totalSuccess = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize const validation field" );
        }
        // else: @TODO: is using an UnknownField placeholder the better choice on failure ?  

        return field;  
    }

    // type or size sections may actually be subject to a repeat loop
    // in such a case the thing to create is the repeat element not the thing to be repeated, since it needs to be wrapped in a loop
    const CORE::CVariant& repeat = fieldNode.GetAttributeValueOrChildValueByName( "repeat" );
    if ( repeat.IsInitialized() )
    {
        CKaitaiSchemaBaseFieldPtr field = CKaitaiSchemaRepeatLogic::CreateSharedObjWithParam( GetSchemaMeta() ); 
        if GUCEF_PREDICT_FALSE( field.IsNULL() )        
        {
            // dont assume we can recover from allocation failures
            return CKaitaiSchemaBaseFieldPtr();
        } 
                    
        if GUCEF_PREDICT_FALSE( !field->Deserialize( fieldNode, defaultSerializerSettings ) )
        {
            totalSuccess = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize repeat logic" );
        }
        // else: @TODO: is using an UnknownField placeholder the better choice on failure ?  

        return field;
    }

    CORE::CString fieldTypeStr = fieldNode.GetAttributeValueOrChildValueByName( "type" ).AsString();
    if ( !fieldTypeStr.IsNULLOrEmpty() )
    {
        // toggle between a regular numeric scalar and an enum scalar based on the presence of the enum attribute
        // the enum scalar is also a regular numeric scalar but it has a reference to the enum definition
        const CORE::CVariant& enumReference = fieldNode.GetAttributeValueOrChildValueByName( "enum" );

        CKaitaiSchemaBaseFieldPtr field;
        if ( !enumReference.IsInitialized() )
            field = CreateFieldObjectForFieldTypeStr( fieldTypeStr, GetSchemaMeta() );
        else
            field = CKaitaiSchemaEnumScalarField::CreateSharedObjWithParam( GetSchemaMeta() );                    
                    
        if GUCEF_PREDICT_FALSE( field.IsNULL() )
        {                                                
            if ( GetSchemaMeta()->AreOpaqueTypesEnabled() )
            {
                // When opaque types are enabled we can try to create a generic opaque field
                // This will runtime reference some external processing capability for this type
                field = CKaitaiSchemaOpaqueField::CreateSharedObjWithParam( GetSchemaMeta() );
            }
            else
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
                    return field;
                }
                else
                {
                    // dont assume we can recover from allocation failures
                    return CKaitaiSchemaBaseFieldPtr();
                } 
            }
        }

        if GUCEF_PREDICT_FALSE( !field->Deserialize( fieldNode, defaultSerializerSettings ) )
        {
            totalSuccess = false;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize field spec of type " + fieldTypeStr );
        }
        // else: @TODO: is using an UnknownField placeholder the better choice on deserialization failure ? 

        return field;
    }
    else
    {                    
        // We do not have a type, the type can be implicit via 'size' which means its a binary payload
        CORE::CString sizeValue = fieldNode.GetAttributeValueOrChildValueByName( "size" ).AsString();
        if ( !sizeValue.IsNULLOrEmpty() )
        {
            // we have a binary field
            CKaitaiSchemaBinaryScalarFieldPtr field = CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( m_schemaMeta );
            if GUCEF_PREDICT_TRUE( !field.IsNULL() )
            {
                if GUCEF_PREDICT_FALSE( !field->Deserialize( fieldNode, defaultSerializerSettings ) )
                {
                    totalSuccess = false;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize binary field spec with size " + sizeValue );
                }
                // else: @TODO: is using an UnknownField placeholder the better choice on deserialization failure ?  

                return field; 
            }
            else
            {
                // dont assume we can recover from allocation failures
                return CKaitaiSchemaBaseFieldPtr();
            }
        }
        else
        {
            const CORE::CDataNode* typeNode = fieldNode.FindChild( "type" );
            const CORE::CDataNode* sizeNode = fieldNode.FindChild( "size" );
            const CORE::CDataNode* parentNode = GUCEF_NULL != typeNode ? typeNode : sizeNode;
                        
            if ( GUCEF_NULL != parentNode )
            {
                // Check if this is a switch section
                CORE::CString switchOnValue = parentNode->GetAttributeValueOrChildValueByName( "switch-on" ).AsString(); 
                if ( !switchOnValue.IsNULLOrEmpty() )
                {
                    CKaitaiSchemaSwitchLogicPtr switchElement = CKaitaiSchemaSwitchLogic::CreateSharedObjWithParam( m_schemaMeta );
                    if GUCEF_PREDICT_FALSE( switchElement.IsNULL() )
                    {
                        // dont assume we can recover from allocation failures
                        return CKaitaiSchemaBaseFieldPtr();
                    }

                    if GUCEF_PREDICT_FALSE( !switchElement->Deserialize( fieldNode, defaultSerializerSettings ) )
                    {
                        totalSuccess = false;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaStructureField:Deserialize: Failed to deserialize switch statement" );
                    }

                    return switchElement;
                }
            }
        }
    }

    // Unable to determine the schema element type
    return CKaitaiSchemaBaseFieldPtr();
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
                CKaitaiSchemaBaseFieldPtr fieldObj = CreateSchemaObjectForFieldDataNode( *fieldNode, totalSuccess );
                if ( !fieldObj.IsNULL() )
                {
                    fields.push_back( fieldObj );
                }
                else
                {
                    // this should not happen
                    return false;
                }
            }
            ++i;
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
