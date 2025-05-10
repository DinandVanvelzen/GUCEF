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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefKAITAI_CKaitaiSchemaRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CKaitaiSchemaRegistry::CKaitaiSchemaRegistry( void )
    : CORE::CTONRegistry< CORE::CTONRegistry< CKaitaiSchema, MT::CMutex >, MT::CMutex >()
    , m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaRegistry::~CKaitaiSchemaRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const MT::CILockable* 
CKaitaiSchemaRegistry::AsLockable( void ) const
{GUCEF_TRACE;

    return CORE::CTONRegistry< CORE::CTONRegistry< CKaitaiSchema, MT::CMutex >, MT::CMutex >::AsLockable();
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CKaitaiSchemaRegistry::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CKaitaiSchemaRegistry::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------*/

CKaitaiSchemaPtr
CKaitaiSchemaRegistry::TryGetSchema( const CORE::CString& schemaFamily , 
                                     const CORE::CString& schemaName   ) const
{GUCEF_TRACE;

    // First obtain the registery specific to the requested family
    TSchemaFamilyRegistryPtr schemaFamilyRegistry;
    if ( TryLookup( schemaFamily, schemaFamilyRegistry, false ) && schemaFamilyRegistry )
    {
        // Locate the specific schema by name, if available
        TSchemaPtr schema;
        if ( schemaFamilyRegistry->TryLookup( schemaName, schema, false ) && schema )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:TryGetCodec: Obtained schema \"" + schemaName + "\" from schema family " + schemaFamily );
            return schema;
        }
    }

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:TryGetCodec: Did not find schema \"" + schemaName + "\" in schema family " + schemaFamily );
    return TSchemaPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::RegisterSchema( TSchemaPtr schema                 ,
                                       const CORE::CString& schemaFamily )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( schema.IsNULL() )
        return false;
    
    MT::CObjectScopeLock lock( AsLockable() );
    
    TRegisteredObjPtr registry;
    if ( !TryLookup( schemaFamily, registry, true ) || registry.IsNULL() )
    {
        // Lazy init, adding the family as a registry
        registry = TSchemaFamilyRegistryPtr( GUCEF_NEW TSchemaFamilyRegistry() );
        Register( schemaFamily, registry );
    }

    if ( !registry.IsNULL() )
    {
        if ( registry->TryRegister( schema->GetSchemaId(), schema ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Successfully registered new schema with id \"" + schema->GetSchemaId() + 
                "\" for family " + schemaFamily );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Failed to register new schema with id \"" + schema->GetSchemaId() + 
                "\" for family " + schemaFamily );
            return false;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Failed to obtain registery for schema family " + schemaFamily );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CKaitaiSchemaRegistry::ResolveSchemaImportDependencies( CKaitaiSchemaPtr schema                   ,
                                                        const CORE::CStringSet& unresolvedImports )
{GUCEF_TRACE;

    if GUCEF_PREDICT_FALSE( schema.IsNULL() )
        return;
    
    const CORE::CString& schemaFamily = schema->GetSchemaFamily();
    
    // If this schema has unresolved imports regisster them for deferred resolution
    if ( !unresolvedImports.empty() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Noted " + CORE::ToString( unresolvedImports.size() ) + " missing import dependencies" );

        CORE::CStringSet::const_iterator i = unresolvedImports.begin();
        while ( i != unresolvedImports.end() )
        {
            const CORE::CString& importId = (*i);
            m_missingImportsSchemaFamilyMap[ schemaFamily ][ importId ].insert( schema );

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Noted missing import dependency '" + importId + 
                "' for schema '" + schema->id + "' in family " + schemaFamily );
            ++i;
        }
    }

    // Check to see if we can resolve the imports of other schemas that depend on this one
    TSchemaFamilyMap::iterator i = m_missingImportsSchemaFamilyMap.find( schemaFamily );
    if ( i != m_missingImportsSchemaFamilyMap.end() )
    {
        TSchemaSetMap::iterator j = (*i).second.find( schema->id );
        if ( j != i->second.end() )
        {
            // There are schemas that depend on this one
            TSchemaSet::iterator k = j->second.begin();
            while ( k != j->second.end() )
            {
                CKaitaiSchemaPtr dependentSchema = (*k);
                if ( !dependentSchema.IsNULL() )
                {
                    // Try to resolve the imports of the dependent schema
                    CORE::CStringSet stillUnresolvedImports;
                    dependentSchema->ResolveImports( stillUnresolvedImports );

                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Resolved import dependency '" + schema->id + 
                        "' for schema '" + dependentSchema->id + "' in family " + schemaFamily + " with " + CORE::ToString( stillUnresolvedImports.size() ) + " unresolved imports remaining" );
                }
                ++k;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( AsLockable() );
    
    m_missingImportsSchemaFamilyMap.erase( name );
    TSchemaRegistryBase::Unregister( name );
}

/*-------------------------------------------------------------------------*/

void 
CKaitaiSchemaRegistry::UnregisterAll( void )
{GUCEF_TRACE;
 
    MT::CObjectScopeLock lock( AsLockable() );
    
    m_missingImportsSchemaFamilyMap.clear();
    TSchemaRegistryBase::UnregisterAll();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchemaFromString( const CORE::CString& schemaContent ,
                                             const CORE::CString& schemaFamily  )
{GUCEF_TRACE;

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObjWithParam( schemaFamily );
    if ( !schema.IsNULL() )
    {
        CORE::CStringSet unresolvedImports;
        if ( schema->LoadSchemaFromString( schemaContent, unresolvedImports ) )
        {
            ResolveSchemaImportDependencies( schema, unresolvedImports );
            
            if ( RegisterSchema( schema, schemaFamily ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchemaFromString: Successfully registered schema into family " + 
                    schemaFamily + " using string: " + CORE::ToString( schemaContent ) );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchemaFromString: Failed to register schema into family " + 
                    schemaFamily + " using string: " + CORE::ToString( schemaContent ) );
                return false;
            }
        }
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchemaFromString: Failed to create schema object" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchema( const CORE::CUri& schemaResource  ,
                                   const CORE::CString& schemaFamily )
{GUCEF_TRACE;

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObjWithParam( schemaFamily );
    if ( !schema.IsNULL() )
    {
        CORE::CStringSet unresolvedImports;
        if ( schema->LoadSchema( schemaResource, unresolvedImports ) )
        {
            ResolveSchemaImportDependencies( schema, unresolvedImports );
                        
            if ( RegisterSchema( schema, schemaFamily ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchema: Successfully registered schema \"" + schema->GetSchemaId() + "\" into family \"" + 
                    schemaFamily + "\" It has " + CORE::ToString( unresolvedImports.size() ) + " unresolved imports" );

                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchema: Failed to register schema \"" + schema->GetSchemaId() + "\" into family \"" + 
                    schemaFamily + "\" using resource " + CORE::ToString( schemaResource ) );
                return false;
            }
        }
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchema: Failed to obtain access to resource " + CORE::ToString( schemaResource ) + " for schema family " + schemaFamily );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchemas( const CORE::CUri& schemaResourceRoot ,
                                    const CORE::CString& schemaFamily    ,
                                    bool recursive                       )
{GUCEF_TRACE;

    CORE::CCoreGlobal* coreGlobal = CORE::CCoreGlobal::Instance();
    CORE::CUriResourceAccessorFactory& raFactory = coreGlobal->GetUriResourceAccessorFactory();

    CORE::CUriResourceAccessorPtr accessor = raFactory.CreateAccessor( schemaResourceRoot );
    if ( !accessor.IsNULL() )
    {
        CORE::CUriResourceAccessor::UriVector uris;
        if ( accessor->ResolveResourcesInCollection( schemaResourceRoot, uris, recursive, false ) )
        {
            bool totalSuccess = true;
            CORE::CUriResourceAccessor::UriVector::iterator i = uris.begin();
            while ( i != uris.end() )
            {
                const CORE::CUri& uri = (*i);
                totalSuccess = LoadSchema( uri, schemaFamily ) && totalSuccess;
            }
            return totalSuccess;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchemaUsingVfs( const CORE::CString& schemaResourcePath  ,
                                           const CORE::CString& schemaFamily        )
{GUCEF_TRACE;

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObjWithParam( schemaFamily );
    if ( !schema.IsNULL() )
    {
        CORE::CStringSet unresolvedImports;
        if ( schema->LoadSchemaUsingVfs( schemaResourcePath, unresolvedImports ) )
        {
            ResolveSchemaImportDependencies( schema, unresolvedImports );
            
            if ( RegisterSchema( schema, schemaFamily ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Successfully registered schema into family " + 
                    schemaFamily + " using VFS resource " + CORE::ToString( schemaResourcePath ) );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Failed to register schema into family " + 
                    schemaFamily + " using VFS resource " + CORE::ToString( schemaResourcePath ) );
                return false;
            }
        }
        return false;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchema: Failed to obtain access to VFS resource " + 
        CORE::ToString( schemaResourcePath ) + " for schema family " + schemaFamily );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchemasUsingVfs( const CORE::CString& schemaResourcesRoot ,
                                            const CORE::CString& schemaFamily        ,
                                            bool recursive                           )
{GUCEF_TRACE;

    VFS::CVfsGlobal* vfsGlobal = VFS::CVfsGlobal::Instance();
    VFS::CVFS& vfs = vfsGlobal->GetVfs();

    VFS::CVFS::TStringVector files;
    if ( vfs.GetFileList( files, schemaResourcesRoot, recursive, true, CORE::CString::EmptyStringSet ) )
    {
        bool totalSuccess = true;
        VFS::CVFS::TStringVector::iterator i = files.begin();
        while ( i != files.end() )
        {
            const CORE::CString& schemaResourcePath = (*i);
            totalSuccess = LoadSchemaUsingVfs( schemaResourcePath, schemaFamily ) && totalSuccess;
            ++i;
        }

        return totalSuccess;
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:LoadSchema: Failed to obtain list of VFS resources at root " + schemaResourcesRoot );
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