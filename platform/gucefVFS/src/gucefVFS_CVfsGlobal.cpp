/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */


#ifndef GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#include "gucefVFS_CVfsUriResourceAccessor.h"
#define GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#endif /* GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_VFS_CASYNCVFSOPERATION_H
#include "gucefVFS_CAsyncVfsOperation.h"
#define GUCEF_VFS_CASYNCVFSOPERATION_H
#endif /* GUCEF_VFS_CASYNCVFSOPERATION_H ? */

#ifndef GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#include "gucefVFS_CRemoteArchiveEvents.h"
#define GUCEF_VFS_CREMOTEARCHIVEEVENTS_H
#endif /* GUCEF_VFS_CREMOTEARCHIVEEVENTS_H ? */

#ifndef GUCEF_VFS_CVFSFETCHTASKCONSUMER_H
#include "gucefVFS_CVfsFetchTaskConsumer.h"
#define GUCEF_VFS_CVFSFETCHTASKCONSUMER_H
#endif /* GUCEF_VFS_CVFSFETCHTASKCONSUMER_H ? */

#ifndef GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H
#include "gucefVFS_CVfsAsyncCallTaskConsumer.h"
#define GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H
#endif /* GUCEF_VFS_CVFSASYNCCALLTASKCONSUMER_H ? */

#include "gucefVFS_CVfsGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CORE::CUriResourceAccessor, CVfsUriResourceAccessor, MT::CMutex >     TVfsUriResourceAccessorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CVfsGlobal::g_datalock;
CVfsGlobal* CVfsGlobal::g_instance = GUCEF_NULL;

TAsyncVfsOperationTaskFactory g_asyncVfsOperationTaskFactory;
TVfsUriResourceAccessorFactory g_vfsUriResourceAccessorFactory;
CORE::CTFactory< CORE::CTaskConsumer, CVfsFetchTaskConsumer, MT::CMutex > g_vfsFetchTaskConsumerFactory;
CORE::CTFactory< CORE::CTaskConsumer, CVfsAsyncCallTaskConsumer, MT::CMutex > g_vfsAsyncCallTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsGlobal*
CVfsGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex scopeLock( g_datalock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CVfsGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CVfsGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeBlockCacheFolderKey( const CORE::CString& folder )
{GUCEF_TRACE;

    CORE::CString key = CORE::RelativePath( folder ).ReplaceChar( '\\', '/' );
    while ( key.Length() > 1 && '/' == key[ key.Length() - 1 ] )
        key = key.SubstrFromRange( 0, key.Length() - 1 );
    return key;
}

/*-------------------------------------------------------------------------*/

CVfsBlockCachePtr
CVfsGlobal::AcquireBlockCache( const CORE::CString& folder ,
                               UInt64 maxBytes             ,
                               UInt32 blockBytes           ,
                               UInt32 maxOpenFiles         ,
                               bool breakStaleLock         )
{GUCEF_TRACE;

    if ( folder.IsNULLOrEmpty() )
        return CVfsBlockCachePtr();

    const CORE::CString key = MakeBlockCacheFolderKey( folder );
    MT::CScopeMutex lock( m_blockCacheLock );

    bool expiredEntry = false;
    TBlockCacheMap::iterator i = m_blockCaches.find( key );
    if ( i != m_blockCaches.end() )
    {
        CVfsBlockCachePtr existing = i->second.GetSharedPtr();
        if ( !existing.IsNULL() )
        {
            if ( existing->GetMaxBytes() != maxBytes || existing->GetBlockBytes() != blockBytes )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CVfsGlobal: The block cache of " << key << " is open already with other settings than asked for, the open one is used" );
            return existing;
        }
        // the last user let go: its destructor may still be closing the folder
        expiredEntry = true;
        m_blockCaches.erase( i );
    }

    // a cache that is still closing its folder holds cache.lock for a moment: a short wait for it, only when there was one
    const UInt32 attempts = expiredEntry ? 40 : 1;
    for ( UInt32 attempt = 0; attempt < attempts; ++attempt )
    {
        CVfsBlockCachePtr cache( GUCEF_NEW CVfsBlockCache() );
        if ( cache->Open( folder, maxBytes, blockBytes, maxOpenFiles, breakStaleLock ) )
        {
            m_blockCaches[ key ] = TBlockCacheWeakPtr( cache );
            return cache;
        }
        if ( attempt + 1 < attempts )
            MT::PrecisionDelay( 25 );
    }
    return CVfsBlockCachePtr();
}

/*-------------------------------------------------------------------------*/

CVfsBlockCachePtr
CVfsGlobal::GetDefaultBlockCache( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_blockCacheLock );
    return m_defaultBlockCache;
}

/*-------------------------------------------------------------------------*/

void
CVfsGlobal::SetDefaultBlockCache( const CVfsBlockCachePtr& cache )
{GUCEF_TRACE;

    CVfsBlockCachePtr previous;
    {
        MT::CScopeMutex lock( m_blockCacheLock );
        previous = m_defaultBlockCache;
        m_defaultBlockCache = cache;
    }
    // the previous one goes (and closes its folder) outside the lock
}

/*-------------------------------------------------------------------------*/

void
CVfsGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_datalock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CVfsGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the Core systems are initialzed first since this module depends on Core
    CORE::CCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefVFS Global systems" );

    // Init events
    CVFS::RegisterEvents();
    CAsyncVfsOperation::RegisterEvents();
    CRemoteArchiveEvents::RegisterEvents();
    
    try
    {
        CORE::CCoreGlobal::Instance()->GetCodecRegistry().Register( "VFSPackCodec", CORE::CCodecRegistry::TCodecFamilyRegistryTypedPtr( GUCEF_NEW CORE::CCodecRegistry::TCodecFamilyRegistry() ) );
    }
    catch ( CORE::CCodecRegistry::EAlreadyRegistered& )
    {
    } 

    /*
     *      Instantiate all singletons
     */
    m_vfs = GUCEF_NEW CVFS();

    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAsyncVfsOperation::TaskType, &g_asyncVfsOperationTaskFactory );
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CVfsFetchTaskConsumer::TaskType, &g_vfsFetchTaskConsumerFactory );
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CVfsAsyncCallTaskConsumer::TaskType, &g_vfsAsyncCallTaskConsumerFactory );
    m_fetchScheduler = GUCEF_NEW CVfsFetchScheduler();
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().RegisterConcreteFactory( CVfsUriResourceAccessor::SchemeName, &g_vfsUriResourceAccessorFactory );
    
    /*
     *      register all codecs/handlers/notifiers
     */

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefVFS Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefVFS" );
}

/*-------------------------------------------------------------------------*/

CVfsGlobal::CVfsGlobal( void )
    : m_vfs( GUCEF_NULL )
    , m_threadPolicyRegistry()
    , m_fetchScheduler( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsGlobal::~CVfsGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefVFS global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefVFS" );

    /*
     *  Unregister handlers
     */
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncVfsOperation::TaskType ); 

    // the consumer of the asynchronous calls stays registered, as that of the background fetches does: calls that are still queued
    // when the VFS goes are run by it and find the VFS shut, which a pool that has to empty its queue before it stops depends on
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().UnregisterConcreteFactory( CVfsUriResourceAccessor::SchemeName );

    // the default block cache goes first: its folder is closed here, before the systems it may be used by
    SetDefaultBlockCache( CVfsBlockCachePtr() );

    /*
     *      cleanup all singletons
     *      Take care to deinstance them in the correct order !!!
     */

    GUCEF_DELETE m_vfs;
    m_vfs = GUCEF_NULL;

    // after the VFS: its archives drop their background jobs when they unload
    GUCEF_DELETE m_fetchScheduler;
    m_fetchScheduler = GUCEF_NULL;
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CVfsFetchTaskConsumer::TaskType );
}

/*-------------------------------------------------------------------------*/

CVFS&
CVfsGlobal::GetVfs( void )
{GUCEF_TRACE;

    return *m_vfs;
}

/*-------------------------------------------------------------------------*/

CVfsThreadPolicyRegistry&
CVfsGlobal::GetThreadPolicyRegistry( void )
{GUCEF_TRACE;

    return m_threadPolicyRegistry;
}

/*-------------------------------------------------------------------------*/

CVfsFetchScheduler&
CVfsGlobal::GetFetchScheduler( void )
{GUCEF_TRACE;

    return *m_fetchScheduler;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
