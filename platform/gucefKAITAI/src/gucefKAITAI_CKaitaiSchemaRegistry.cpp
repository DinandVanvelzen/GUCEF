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

CKaitaiSchemaRegistry::TSchemaPtr 
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
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:TryGetCodec: Obtained schema " + schemaName + " from schema family " + schemaFamily );
            return schema;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:TryGetCodec: Failed to obtain schema " + schemaName + " from schema family " + schemaFamily );
    return TSchemaPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::RegisterSchema( TSchemaPtr schema                 ,
                                       const CORE::CString& schemaFamily )
{GUCEF_TRACE;

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
        if ( registry->TryRegister( schema->id, schema ) )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Successfully registered new schema with id \"" + schema->id + 
                "\" for family " + schemaFamily );
            return true;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Failed to register new schema with id \"" + schema->id + 
                "\" for family " + schemaFamily );
            return false;
        }
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchemaRegistry:RegisterSchema: Failed to obtain registery for schema family " + schemaFamily );
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CKaitaiSchemaRegistry::LoadSchemaFromString( const CORE::CString& schemaContent ,
                                             const CORE::CString& schemaFamily  )
{GUCEF_TRACE;

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObj();
    if ( !schema.IsNULL() )
    {
        if ( schema->LoadSchemaFromString( schemaContent ) )
        {
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

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObj();
    if ( !schema.IsNULL() )
    {
        if ( schema->LoadSchema( schemaResource ) )
        {
            if ( RegisterSchema( schema, schemaFamily ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: Successfully registered schema into family " + 
                    schemaFamily + " using resource " + CORE::ToString( schemaResource ) );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KaitaiSchema:LoadSchema: SFailed to register schema into family " + 
                    schemaFamily + " using resource " + CORE::ToString( schemaResource ) );
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

    TSchemaPtr schema = CKaitaiSchema::CreateSharedObj();
    if ( !schema.IsNULL() )
    {
        if ( schema->LoadSchemaUsingVfs( schemaResourcePath ) )
        {
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