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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

#include "gucefKAITAI_CKaitaiSchema.h"

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

const CORE::CString CKaitaiSchema::ClassTypeName = "GUCEF::KAITAI::CKaitaiSchema";
const CORE::CString CKaitaiSchema::SchemaFileExtension = "ksy";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchema::CKaitaiSchema( void )
    : CKaitaiSchemaBaseField( Schema )
    , CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >( this )
    , id()
    , isLittleEndian( true )
    , imports()
    , structure()
    , m_schemaFamily()
    , m_schemaDocument()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchema::CKaitaiSchema( const CORE::CString& schemaFamily )
    : CKaitaiSchemaBaseField( Schema )
    , CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >( this )
    , id()
    , isLittleEndian( true )
    , imports()
    , structure()
    , m_schemaFamily( schemaFamily )
    , m_schemaDocument()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchema::CKaitaiSchema( const CKaitaiSchema& src )
    : CKaitaiSchemaBaseField( src )
    , CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >( this )
    , id( src.id )
    , isLittleEndian( src.isLittleEndian )
    , imports( src.imports )
    , structure( src.structure )
    , m_schemaFamily( src.m_schemaFamily )
    , m_schemaDocument( src.m_schemaDocument )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchema::~CKaitaiSchema()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchema& 
CKaitaiSchema::operator=( const CKaitaiSchema& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CKaitaiSchemaBaseField::operator=( src );
        id = src.id;
        isLittleEndian = src.isLittleEndian;
        imports = src.imports;
        m_schemaFamily = src.m_schemaFamily;
        m_schemaDocument = src.m_schemaDocument;
        
        if ( !src.structure.IsNULL() )
            structure = CKaitaiSchemaStructureField::CreateSharedObjWithParam( *src.structure );
        else
            structure.Unlink();
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CKaitaiSchema::Clear( void )
{GUCEF_TRACE;

    id.Clear();
    isLittleEndian = true;
    imports.clear();
    types.clear();
        
    if ( !structure.IsNULL() )
        structure->Clear();
    structure.Unlink();
    
    //m_schemaFamily.Clear();
    m_schemaDocument.Clear();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable* 
CKaitaiSchema::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CKaitaiSchema( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchema::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

KaitaiSchemaFieldType 
CKaitaiSchema::GetFieldTypeForTypeFieldString( const CORE::CString& typeFieldStr ) const
{GUCEF_TRACE;

    UInt8 gucefTypeId = CKaitaiSchemaBaseField::KaitaiBuildInTypeStringToGucefType( typeFieldStr );
    if ( GUCEF_DATATYPE_UNKNOWN == gucefTypeId )
    {
        // no luck with the build-in types
        TFieldTypeMap::const_iterator i = types.find( typeFieldStr );
        if ( i != types.end() )
        {
            // the type is essentially a typedef
            const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;
            if ( !fieldObj.IsNULL() )
                return fieldObj->GetFieldType();
        }
    }
    else
    {
        // the type is scalar and we can return the corresponding KaitaiSchemaFieldType
        return BasicField;
    }
    
    // the type is not known. cannot resolve it
    return UnknownField;
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchema::CreateFieldObjectForFieldTypeStr( const CORE::CString& typeName ) const
{GUCEF_TRACE;

    // First check for build-in types
    // You should not be able to override build-in types
    CKaitaiSchemaBaseFieldPtr fieldObj = CreateDefaultFieldObjectForBuildInFieldTypeName( typeName );
    if ( fieldObj.IsNULL() )
    {
        // no luck with the build-in types
        // check our schema owned types map
        TFieldTypeMap::const_iterator i = types.find( typeName );
        if ( i != types.end() )
        {
            // the type is essentially a typedef defined in our types map
            const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;
            if ( !fieldObj.IsNULL() )
                return fieldObj->CloneAsFieldObject();
        }

        // As a last ditch effort check beyond this schema
        // We will need to check the registry for the type as it could be defined in the parent schema or any of the imports
        CKaitaiGlobal* kaitaiGlobal = CKaitaiGlobal::Instance();
        fieldObj = kaitaiGlobal->GetKaitaiSchemaRegistry().TryGetSchemaOrSubType( m_schemaFamily, typeName );
    }
    return fieldObj;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::IsValid( void ) const
{GUCEF_TRACE;
    
    // we are valid if we have a schema family and a schema id and imports are resolved which implied all deserialization is done
    return !m_schemaFamily.IsNULLOrEmpty() && !id.IsNULLOrEmpty() && !HasUnresolvedImports();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaBaseFieldPtr 
CKaitaiSchema::TryGetDefinedType( const CORE::CString& typeName ) const
{GUCEF_TRACE;

    // try to get the type from our types map
    TFieldTypeMap::const_iterator i = types.find( typeName );
    if ( i != types.end() )
    {
        // we have a valid type object
        const CKaitaiSchemaBaseFieldPtr& fieldObj = (*i).second;
        if ( !fieldObj.IsNULL() )
            return fieldObj;
    }
    return CKaitaiSchemaBaseFieldPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::HasUnresolvedImports( void ) const
{GUCEF_TRACE;

    // if we have unresolved imports we will have stored the document description
    // as a copy of the data node DOM. Hence we can check if the document is empty to see if we have unresolved imports
    // aks a deferment of the import resolution plus subsequent deserialization
    return !m_schemaDocument.IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::ResolveImports( CORE::CStringSet& unresolvedImports )
{GUCEF_TRACE;

    // Try to resolve the imports and add them to our 'types' lookup map
    // this depends on order of operations across schemas and as such may need to be deferred
    // Allowsing for deferred import resolution provides a more flexible implementation where we can
    // load schemas in any order and still resolve them over time

    unresolvedImports.clear();
    bool alreadyHadUnresolvedImports = HasUnresolvedImports();
    CKaitaiGlobal* kaitaiGlobal = CKaitaiGlobal::Instance();

    CORE::CStringVector::const_iterator i = imports.begin();
    while ( i != imports.end() )
    {
        const CORE::CString& importId = (*i);
        if ( !importId.IsNULLOrEmpty() )
        {
            // try to (re)resolve the import
            CKaitaiSchemaPtr importedSchema = kaitaiGlobal->GetKaitaiSchemaRegistry().TryGetSchema( m_schemaFamily, importId );
            if ( !importedSchema.IsNULL() )
            {
                // we have a valid schema object, add it to our types map
                types[ importId ] = importedSchema;
            }
            else
            {
                if ( types.find( importId ) == types.end() )
                {
                    // we could not resolve the import
                    unresolvedImports.insert( importId );
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:ResolveImports: Unable to resolve import " + importId + " at this time" );
                }
                else
                {
                    // it was already resolved by some other means
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:ResolveImports: import not resolved via registry however the type is resolved in the schema by other means: " + importId );
                }                
            }
        }
        ++i;
    }

    if ( alreadyHadUnresolvedImports && unresolvedImports.empty() )
    {
        // the last of the unresolved imports has been resolved
        // we can now deserialize the schema document
        if ( !m_schemaDocument.IsEmpty() )
        {
            // we have a schema document to deserialize
            CORE::CDataNodeSerializableSettings defaultSettings;
            if ( Deserialize( m_schemaDocument, defaultSettings, unresolvedImports ) )
            {
                // we can clear the document now that we are done with it
                m_schemaDocument.Clear();
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:ResolveImports: Successfully deserialized schema document after resolving all imports. schema id=" + id );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:ResolveImports: Failed to deserialize the schema document after resolving all imports. schema id=" + id );
                return false;
            }
        }
        else
        {
            // this should not happen, we should have a schema document to deserialize
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:ResolveImports: Missing schema document after resolving all imports. schema id=" + id );
            return false;
        }
    }
    else
    {
        return unresolvedImports.empty();
    }
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchema::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::DeserializeTypesData( const CORE::CDataNode& domRootNode                  , 
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    const CORE::CDataNode* typesNode = domRootNode.FindChild( "types" );
    if ( GUCEF_NULL != typesNode )
    {
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
                    CKaitaiSchemaStructureFieldPtr typeStruct = CKaitaiSchemaStructureField::CreateSharedObj();
                    if GUCEF_PREDICT_FALSE( typeStruct.IsNULL() )
                    {
                        totalSuccess = false;
                        ++i;
                        continue;
                    }

                    if ( typeStruct->Deserialize( *typeEntryNode, settings ) )
                    {
                        types[ typeEntryNode->GetName() ] = typeStruct;
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
                        CKaitaiSchemaBaseFieldPtr field = CreateFieldObjectForFieldTypeStr( fieldTypeStr );
                        if GUCEF_PREDICT_FALSE( field.IsNULL() )
                        {
                            totalSuccess = false;
                            ++i;
                            continue;
                        }

                        if ( field->Deserialize( *fieldNode, settings ) )
                        {
                            types[ typeEntryNode->GetName() ] = field;
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
CKaitaiSchema::DeserializeMetaData( const CORE::CDataNode& domRootNode                  , 
                                    const CORE::CDataNodeSerializableSettings& settings ,
                                    CORE::CStringSet& unresolvedImports                 )
{GUCEF_TRACE;
    
    const CORE::CDataNode* metaNode = domRootNode.FindChild( "meta" );
    if ( GUCEF_NULL != metaNode )
    {
        id = metaNode->GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, true );

        const CORE::CString& endianCurrentStr = CKaitaiSchemaBaseField::EndianValueToString( isLittleEndian ? GUCEF_BYTEORDER_LITTLE_ENDIAN : GUCEF_BYTEORDER_BIG_ENDIAN );
        CORE::CString endianStr = metaNode->GetAttributeValueOrChildValueByName( "endian", endianCurrentStr, true ).AsString( endianCurrentStr, true );
        isLittleEndian = CKaitaiSchemaBaseField::StringToEndianValue( endianStr ) == GUCEF_BYTEORDER_LITTLE_ENDIAN;

        CORE::CDataNode::TVariantVector importsList;
        if ( domRootNode.GetValuesOfChildByName( "imports", importsList ) )
        {
            imports = CORE::ToStringVector( importsList );
        }

        // Now we will try to resolve the imports and add them to our 'types' lookup map
        // this depends on order of operations across schemas
        if ( !ResolveImports( unresolvedImports ) )
        {
            // retain the document for later use, deferred deserialization
            m_schemaDocument = domRootNode;
        }
        else
        {
            // we have resolved all imports, we can clear the document
            m_schemaDocument.Clear();
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchema::Deserialize( const CORE::CDataNode& domRootNode                  , 
                            const CORE::CDataNodeSerializableSettings& settings ,
                            CORE::CStringSet& unresolvedImports                 )
{GUCEF_TRACE;

    Clear();
    
    if ( DeserializeMetaData( domRootNode, settings, unresolvedImports ) )
    {
        if ( !unresolvedImports.empty() )
        {
            return true; // we cannot proceed without resolving imports
        }

        if ( DeserializeTypesData( domRootNode, settings ) )
        {
            structure = CKaitaiSchemaStructureField::CreateSharedObj();
            if ( structure.IsNULL() )
                return false;

            // the root structure is the schema itself, inherit its properties
            structure->id = id;
            structure->isLittleEndian = isLittleEndian;

            return structure->Deserialize( domRootNode, settings );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchema::Deserialize( const CORE::CDataNode& domRootNode                  , 
                            const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    // When using this member function, which can be called from a generic interface,
    // we cannot defer to resolve imports. As such not having all imports already loaded 
    // but referenced will count as a failure since we cannot proceed with deserialization
    CORE::CStringSet unresolvedImports;
    return Deserialize( domRootNode, settings, unresolvedImports ) && unresolvedImports.empty();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchemaFromString( const CORE::CString& schemaContent  ,
                                     CORE::CStringSet& unresolvedImports )
{GUCEF_TRACE;
    
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( schemaContent, false );
    CORE::DynamicBufferAccessPtr bufferAccess = CORE::CDynamicBufferAccess::CreateSharedObjWithParam( buffer );
    return LoadSchema( bufferAccess, unresolvedImports );
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchema( CORE::IOAccessPtr schemaResource    ,
                           CORE::CStringSet& unresolvedImports )
{GUCEF_TRACE;

    // Grab a YAML codec since Kaitai uses the yaml format
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& dstoreCodecRegistry = coreGlobal->GetDStoreCodecRegistry();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec;
    if ( dstoreCodecRegistry.TryGetYamlCodec( yamlCodec ) )
    {
        // using the codec parse the resource into a usable document
        m_schemaDocument.Clear();
        if ( yamlCodec->BuildDataTree( &m_schemaDocument, schemaResource.GetPointerAlways() ) )
        {
            // now deserialize the document into the strongly typed class
            CORE::CDataNodeSerializableSettings deserializerSettings;
            return Deserialize( m_schemaDocument, deserializerSettings, unresolvedImports );
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to build document using YAML codec" );
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to locate a YAML codec" );
    return false;

}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchema( const CORE::CUri& schemaResource    ,
                           CORE::CStringSet& unresolvedImports )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CUriResourceAccessorFactory& raFactory = coreGlobal->GetUriResourceAccessorFactory();
    
    // first see if we can get access to the resource identified by the uri
    CORE::IOAccessPtr ioAccess;
    if ( raFactory.GetResourceAccess( schemaResource                                           , 
                                      ioAccess                                                 , 
                                      CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) && ioAccess )
    {
        if ( LoadSchema( ioAccess, unresolvedImports ) )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Successfully loaded schema using resource " + CORE::ToString( schemaResource ) );
            return true;
        }
        
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to load schema from resource " + CORE::ToString( schemaResource ) );
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to obtain access to resource " + CORE::ToString( schemaResource ) );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath ,
                                   CORE::CStringSet& unresolvedImports     )
{GUCEF_TRACE;

    VFS::CVfsGlobal* vfsGlobal = VFS::CVfsGlobal::Instance();
    VFS::CVFS& vfs = vfsGlobal->GetVfs();

    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( schemaResourcePath );
    if ( !resource.IsNULL() )
    {
        CORE::IOAccessPtr ioAccess = resource->GetAccess();
        if ( !ioAccess.IsNULL() )
        {
            if ( LoadSchema( ioAccess, unresolvedImports ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Successfully loaded schema using VFS resource " + CORE::ToString( schemaResourcePath ) );
                return true;
            }
        
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to load schema from VFS resource " + CORE::ToString( schemaResourcePath ) );
            return false;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to obtain access to VFS resource " + CORE::ToString( schemaResourcePath ) );
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKaitaiSchema::GetSchemaFamily( void ) const
{GUCEF_TRACE;

    return m_schemaFamily;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
