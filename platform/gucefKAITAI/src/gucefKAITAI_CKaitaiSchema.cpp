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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchema::CKaitaiSchema( void )
    : CORE::CIDataNodeSerializable()
    , CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >( this )
    , id()
    , imports()
    , endianess( GUCEF_BYTEORDER_UNKNOWN_ENDIAN )
    , fields()
    , enums()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchema::CKaitaiSchema( const CKaitaiSchema& src )
    : CORE::CIDataNodeSerializable( src )
    , CORE::CTSharedObjCreator< CKaitaiSchema, MT::CMutex >( this )
    , id( src.id )
    , imports( src.imports )
    , endianess( src.endianess )
    , fields( src.fields )
    , enums( src.enums )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchema& 
CKaitaiSchema::operator=( const CKaitaiSchema& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        id = src.id;
        imports = src.imports;
        endianess = src.endianess;
        fields = src.fields;
        enums = src.enums;
    }
    return *this;
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

bool
CKaitaiSchema::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    return false;
}


/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::DeserializeEnumDefinitions( const CORE::CDataNode& domRootNode                  , 
                                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* enumsNode = domRootNode.FindChild( "enums" );
    if ( GUCEF_NULL != enumsNode )
    {
        bool totalSuccess = true;
        CORE::CDataNode::const_iterator i = enumsNode->ConstBegin();
        while ( i != enumsNode->ConstEnd() )
        {
            const CORE::CDataNode* enumNode = (*i);
            
            CKaitaiSchemaEnumDefinition enumDef;
            if ( enumDef.Deserialize( *enumNode, settings ) && !enumDef.id.IsNULLOrEmpty() )
            {
                enums[ enumDef.id ] = enumDef;    
            }
            else
            {
                totalSuccess = false;
            }
            ++i;
        }

        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::DeserializeFieldSequence( const CORE::CDataNode& domRootNode                  , 
                                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* seqNode = domRootNode.FindChild( "seq" );
    if ( GUCEF_NULL != seqNode )
    {
        bool totalSuccess = true;
        CORE::CDataNode::const_iterator i = seqNode->ConstBegin();
        while ( i != seqNode->ConstEnd() )
        {
            const CORE::CDataNode* fieldNode = (*i);
            
            CKaitaiSchemaFieldDefinition field;
            if ( field.Deserialize( *fieldNode, settings ) && !field.id.IsNULLOrEmpty() )
            {
                fields[ field.id ] = field;    
            }
            else
            {
                totalSuccess = false;
            }
            ++i;
        }

        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::DeserializeMetaData( const CORE::CDataNode& domRootNode                  , 
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;
    
    const CORE::CDataNode* metaNode = domRootNode.FindChild( "meta" );
    if ( GUCEF_NULL != metaNode )
    {
        id = metaNode->GetAttributeValueOrChildValueByName( "id", id, true ).AsString( id, true );

        const CORE::CString& endianCurrentStr = CKaitaiSchemaFieldDefinition::EndianValueToString( endianess );
        CORE::CString endianStr = metaNode->GetAttributeValueOrChildValueByName( "endian", endianCurrentStr, true ).AsString( endianCurrentStr, true );
        endianess = CKaitaiSchemaFieldDefinition::StringToEndianValue( endianStr );

        CORE::CDataNode::TVariantVector importsList;
        if ( domRootNode.GetValuesOfChildByName( "imports", importsList ) )
            imports = CORE::ToStringSet( importsList );
        
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKaitaiSchema::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    if ( DeserializeMetaData( domRootNode, settings ) )
    {
        bool totalSuccess = true;
        totalSuccess = DeserializeFieldSequence( domRootNode, settings ) && totalSuccess;
        totalSuccess = DeserializeEnumDefinitions( domRootNode, settings ) && totalSuccess;
        return totalSuccess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchemaFromString( const CORE::CString& schemaContent )
{GUCEF_TRACE;
    
    CORE::CDynamicBuffer buffer;
    buffer.LinkTo( schemaContent, false );
    CORE::DynamicBufferAccessPtr bufferAccess = CORE::CDynamicBufferAccess::CreateSharedObjWithParam( buffer );
    return LoadSchema( bufferAccess );
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchema( CORE::IOAccessPtr schemaResource )
{GUCEF_TRACE;

    // Grab a YAML codec since Kaitai uses the yaml format
    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CDStoreCodecRegistry& dstoreCodecRegistry = coreGlobal->GetDStoreCodecRegistry();
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr yamlCodec;
    if ( dstoreCodecRegistry.TryGetYamlCodec( yamlCodec ) )
    {
        // using the codec parse the resource into a usable document
        CORE::CDataNode schemaDocument;
        if ( yamlCodec->BuildDataTree( &schemaDocument, schemaResource.GetPointerAlways() ) )
        {
            // now deserialize the document into the strongly typed class
            CORE::CDataNodeSerializableSettings deserializerSettings;
            return Deserialize( schemaDocument, deserializerSettings );
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to build document using YAML codec" );
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to locate a YAML codec" );
    return false;

}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchema::LoadSchema( const CORE::CUri& schemaResource )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CUriResourceAccessorFactory& raFactory = coreGlobal->GetUriResourceAccessorFactory();
    
    // first see if we can get access to the resource identified by the uri
    CORE::IOAccessPtr ioAccess;
    if ( raFactory.GetResourceAccess( schemaResource                                           , 
                                      ioAccess                                                 , 
                                      CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) && ioAccess )
    {
        if ( LoadSchema( ioAccess ) )
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
CKaitaiSchema::LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath )
{GUCEF_TRACE;

    VFS::CVfsGlobal* vfsGlobal = VFS::CVfsGlobal::Instance();
    VFS::CVFS& vfs = vfsGlobal->GetVfs();

    VFS::TBasicVfsResourcePtr resource = vfs.GetFile( schemaResourcePath );
    if ( !resource.IsNULL() )
    {
        CORE::IOAccessPtr ioAccess = resource->GetAccess();
        if ( !ioAccess.IsNULL() )
        {
            if ( LoadSchema( ioAccess ) )
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
