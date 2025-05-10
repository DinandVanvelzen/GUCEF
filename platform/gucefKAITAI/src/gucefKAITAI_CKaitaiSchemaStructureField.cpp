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
    : CKaitaiSchemaBaseField( StructureField, CKaitaiSchemaBaseFieldPtr() )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , m_instances()
    , m_enums()
    , m_types()
    , m_fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::CKaitaiSchemaStructureField( CKaitaiSchemaBaseFieldPtr parent )
    : CKaitaiSchemaBaseField( StructureField, parent )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , m_instances()
    , m_enums()
    , m_types()
    , m_fields()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaStructureField::CKaitaiSchemaStructureField( const CKaitaiSchemaStructureField& src )    
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >( this )
    , m_instances()
    , m_enums()
    , m_types()
    , m_fields()
{GUCEF_TRACE;

    // Use the assignment operator to do the work
    *this = src;
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
        Clear();
        
        CKaitaiSchemaBaseField::operator=( src );

        // deep copy the instances
        TFieldTypeMap::const_iterator i = src.m_instances.begin();
        while ( i != src.m_instances.end() )
        {
            const CORE::CString& typeName = (*i).first;
            const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;

            if ( !fieldObj.IsNULL() )
            {
                // we have a valid type object
                m_instances[ typeName ] = fieldObj->CloneAsFieldObject();
            }
            ++i;
        }

        // deep copy the enums
        i = src.m_enums.begin();
        while ( i != src.m_enums.end() )
        {
            const CORE::CString& typeName = (*i).first;
            const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;

            if ( !fieldObj.IsNULL() )
            {
                // we have a valid type object
                m_enums[ typeName ] = fieldObj->CloneAsFieldObject();
            }
            ++i;
        }

        // deep copy the types
        i = src.m_types.begin();
        while ( i != src.m_types.end() )
        {
            const CORE::CString& typeName = (*i).first;
            const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;

            if ( !fieldObj.IsNULL() )
            {
                // we have a valid type object
                m_types[ typeName ] = fieldObj->CloneAsFieldObject();
            }
            ++i;
        }
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaStructureField::Clear( void )
{GUCEF_TRACE;

    CKaitaiSchemaBaseField::Clear();

    TFieldTypeMap::const_iterator i = m_instances.begin();
    while ( i != m_instances.end() )
    {
        CKaitaiSchemaBaseFieldPtr obj = (*i).second;
        if ( !obj.IsNULL() )
            obj->Clear();
        ++i;
    }
    m_instances.clear();

    i = m_enums.begin();
    while ( i != m_enums.end() )
    {
        CKaitaiSchemaBaseFieldPtr obj = (*i).second;
        if ( !obj.IsNULL() )
            obj->Clear();
        ++i;
    }
    m_enums.clear();

    i = m_types.begin();
    while ( i != m_types.end() )
    {
        CKaitaiSchemaBaseFieldPtr obj = (*i).second;
        if ( !obj.IsNULL() )
            obj->Clear();
        ++i;
    }
    m_types.clear();

    CKaitaiSchemaBaseFieldPtrVector::iterator n = m_fields.begin();
    while ( n != m_fields.end() )
    {
        CKaitaiSchemaBaseFieldPtr obj = (*n);
        if ( !obj.IsNULL() )
            obj->Clear();
        ++n;
    }
    m_fields.clear();

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

const CKaitaiSchema::TFieldTypeMap& 
CKaitaiSchemaStructureField::GetDefinedEnums( void ) const
{GUCEF_TRACE;

    // return the enums map
    return m_enums;
}

/*-------------------------------------------------------------------------*/

const CKaitaiSchema::TFieldTypeMap& 
CKaitaiSchemaStructureField::GetDefinedTypes( void ) const
{GUCEF_TRACE;

    // return the types map
    return m_types;
}

/*-------------------------------------------------------------------------*/

Int32
CKaitaiSchemaStructureField::GetFixedSizeIfAny( void ) const
{GUCEF_TRACE;

    // A structure field can only be fixed size if all its fields are fixed size
    Int32 totalSize = 0;
    CKaitaiSchemaBaseFieldPtrVector::const_iterator i = m_fields.begin();
    while ( i != m_fields.end() )
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

   return m_fields;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::TryGetLocalScopeElement( const CORE::CString& elementName ) const
{GUCEF_TRACE;

    // Order of precedence for non-fully-qualified references in KAITAI:
    //  1 - Instance variables
    //  2 - Fields in the same 'seq' section
    //  3 - Enums
    //  4 - Types
    CKaitaiSchemaBaseFieldPtr element = TryGetLocalScopeInstance( elementName );
    if ( element.IsNULL() )
        element = TryGetLocalScopeField( elementName );    
        if ( element.IsNULL() )
            element = TryGetLocalScopeEnum( elementName );
            if ( element.IsNULL() )
                element = TryGetLocalScopeTypedef( elementName );

    return element;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::TryGetLocalScopeInstance( const CORE::CString& instanceName ) const
{GUCEF_TRACE;

    // try to get the type from our types map
    TFieldTypeMap::const_iterator i = m_instances.find( instanceName );
    if ( i != m_instances.end() )
    {
        // we have a valid type object
        const CKaitaiSchemaBaseFieldPtr& instanceObj = (*i).second;
        if ( !instanceObj.IsNULL() )
            return instanceObj;
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::TryGetLocalScopeEnum( const CORE::CString& enumName ) const
{GUCEF_TRACE;

    // try to get the type from our types map
    TFieldTypeMap::const_iterator i = m_enums.find( enumName );
    if ( i != m_enums.end() )
    {
        // we have a valid type object
        const CKaitaiSchemaBaseFieldPtr& enumObj = (*i).second;
        if ( !enumObj.IsNULL() )
            return enumObj;
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::TryGetLocalScopeTypedef( const CORE::CString& typeName ) const
{GUCEF_TRACE;

    // try to get the type from our types map
    TFieldTypeMap::const_iterator i = m_types.find( typeName );
    if ( i != m_types.end() )
    {
        // we have a valid type object
        const CKaitaiSchemaBaseFieldPtr& typeObj = (*i).second;
        if ( !typeObj.IsNULL() )
            return typeObj;
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::TryGetLocalScopeField( const CORE::CString& fieldName ) const
{GUCEF_TRACE;

    // try to get the type from our types map
    CKaitaiSchemaBaseFieldPtrVector::const_iterator i = m_fields.begin();
    while ( i != m_fields.end() )
    {
        // we have a valid type object
        const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i);
        if ( !fieldObj.IsNULL() && fieldObj->GetFieldType() != UnknownField )
        {
            if ( fieldObj->id == fieldName )
            {
                // we have a match
                return fieldObj;
            }
        }
        ++i;
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchemaStructureField::CreateSchemaObjectForFieldDataNode( const CORE::CDataNode& fieldNode , 
                                                                 bool& totalSuccess               ,
                                                                 CKaitaiSchemaBaseFieldPtr parent ) const
{GUCEF_TRACE; 
                
    CORE::CDataNodeSerializableSettings defaultSerializerSettings;
    
    // contents tag may be used with or without specifying type
    // regardless we will use the CKaitaiSchemaConstValidationScalarField
    const CORE::CVariant& contents = fieldNode.GetAttributeValueOrChildValueByName( "contents" );
    if ( contents.IsInitialized() )
    {
        CKaitaiSchemaBaseFieldPtr field = CKaitaiSchemaConstValidationScalarField::CreateSharedObjWithParam( parent ); 
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
        CKaitaiSchemaBaseFieldPtr field = CKaitaiSchemaRepeatLogic::CreateSharedObjWithParam( parent ); 
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
            field = CreateFieldObjectForFieldTypeStr( fieldTypeStr, AsSharedPtr() );
        else
            field = CKaitaiSchemaEnumScalarField::CreateSharedObjWithParam( parent );                    
                    
        if GUCEF_PREDICT_FALSE( field.IsNULL() )
        {                                                
            if ( GetSchemaMeta()->AreOpaqueTypesEnabled() )
            {
                // When opaque types are enabled we can try to create a generic opaque field
                // This will runtime reference some external processing capability for this type
                field = CKaitaiSchemaOpaqueField::CreateSharedObjWithParam( parent );
            }
            else
            {
                totalSuccess = false;

                // In order to do 'best effort' deserialization we will skip the field
                // However due to field ordering we cannot just leave the slot empty
                // we have to make it expliciy there is an error location in the sequence
                // We will create a dummy field object to fill the gap
                field = CreateDefaultFieldObjectForFieldType( UnknownField, parent );
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
            CKaitaiSchemaBinaryScalarFieldPtr field = CKaitaiSchemaBinaryScalarField::CreateSharedObjWithParam( parent );
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
                    CKaitaiSchemaSwitchLogicPtr switchElement = CKaitaiSchemaSwitchLogic::CreateSharedObjWithParam( parent );
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
CKaitaiSchemaStructureField::DeserializeTypesData( const CORE::CDataNode& domRootNode                  , 
                                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    const CORE::CDataNode* typesNode = domRootNode.FindChild( "types" );
    if ( GUCEF_NULL != typesNode )
    {
        m_types.clear();
        
        bool totalSuccess = true;
        CORE::CDataNode::const_iterator i = typesNode->ConstBegin();
        while ( i != typesNode->ConstEnd() )
        {
            const CORE::CDataNode* typeEntryNode = (*i);
            const CORE::CDataNode* seqNode = typeEntryNode->FindChild( "seq" );
            
            if ( GUCEF_NULL != seqNode )
            {
                UInt32 nrOfFields = seqNode->GetNrOfDirectChildNodes();
                if ( nrOfFields > 1 )
                {
                    // the type is a complex type, a structure
                    CKaitaiSchemaStructureFieldPtr typeStruct = CKaitaiSchemaStructureField::CreateSharedObjWithParam( CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::CreateSharedPtr() );
                    if GUCEF_PREDICT_FALSE( typeStruct.IsNULL() )
                    {
                        totalSuccess = false;
                        ++i;
                        continue;
                    }

                    if ( typeStruct->Deserialize( *typeEntryNode, settings ) )
                    {
                        m_types[ typeEntryNode->GetName() ] = typeStruct;
                    }
                    else
                    {
                        totalSuccess = false;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:DeserializeTypesData: Failed to deserialize structure type with name " + typeEntryNode->GetName() );
                    }
                }
                else
                if ( nrOfFields == 1 )
                {
                    // the type is a simple type, a field
                    const CORE::CDataNode* fieldNode = ( *seqNode->ConstBegin() );
                    if ( GUCEF_NULL != fieldNode )
                    {
                        CORE::CString fieldTypeStr = fieldNode->GetAttributeValueOrChildValueByName( "type" ).AsString();
                        CKaitaiSchemaBaseFieldPtr field = CreateFieldObjectForFieldTypeStr( fieldTypeStr, CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::CreateSharedPtr() );
                        if GUCEF_PREDICT_FALSE( field.IsNULL() )
                        {
                            totalSuccess = false;
                            ++i;
                            continue;
                        }

                        if ( field->Deserialize( *fieldNode, settings ) )
                        {
                            m_types[ typeEntryNode->GetName() ] = field;
                        }
                        else
                        {
                            totalSuccess = false;
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:DeserializeTypesData: Failed to deserialize field of type " + fieldTypeStr + " for typedef " + typeEntryNode->GetName() );
                        }
                    }
                }
                else
                {
                    // sequence section with no children is malformed
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:DeserializeTypesData: Sequence section with no children is malformed. type=" + typeEntryNode->GetName() );
                    totalSuccess = false;
                }
            }

            ++i;
        }
        
        return totalSuccess;
    }
    return true; // having a 'types' section is optional
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStructureField::DeserializeEnumsData( const CORE::CDataNode& domRootNode                  , 
                                                   const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    const CORE::CDataNode* enumsNode = domRootNode.FindChild( "enums" );
    if ( GUCEF_NULL != enumsNode )
    {
        m_enums.clear();
        
        bool totalSuccess = true;
        CORE::CDataNode::const_iterator i = enumsNode->ConstBegin();
        while ( i != enumsNode->ConstEnd() )
        {
            CKaitaiSchemaEnumDefinitionPtr enumDef = CKaitaiSchemaEnumDefinition::CreateSharedObjWithParam( CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::CreateSharedPtr() );
            if ( enumDef.IsNULL() )
                return false;

            const CORE::CDataNode* enumDefNode = (*i);

            if ( enumDef->Deserialize( *enumDefNode, settings ) )
            {
                m_enums[ enumDef->id ] = enumDef;
            }
            else
            {
                totalSuccess = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:DeserializeEnumsData: Failed to deserialize enum definition with name " + enumDefNode->GetName() );
            }

            ++i;
        }
        return totalSuccess;
    }
    return true; // having a 'enums' section is optional
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStructureField::DeserializeInstancesData( const CORE::CDataNode& domRootNode                  , 
                                                       const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    return true; // @TODO: implement this
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaStructureField::Deserialize( const CORE::CDataNode& domRootNode                  , 
                                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;    
    
    bool instancesParseSuccess = DeserializeInstancesData( domRootNode, settings );
    bool enumsParseSuccess = DeserializeEnumsData( domRootNode, settings );
    bool typesParseSuccess = DeserializeTypesData( domRootNode, settings );  
    
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
                CKaitaiSchemaBaseFieldPtr fieldObj = CreateSchemaObjectForFieldDataNode( *fieldNode, totalSuccess, CORE::CTSharedObjCreator< CKaitaiSchemaStructureField, MT::CMutex >::CreateSharedPtr() );
                if ( !fieldObj.IsNULL() )
                {
                    m_fields.push_back( fieldObj );
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
