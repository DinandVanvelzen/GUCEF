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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

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

#ifndef GUCEF_VFS_CVFSURLHANDLER_H
#include "gucefVFS_CVFSURLHandler.h"     /* URL handler for URL's with protocol "vfs" */
#define GUCEF_VFS_CVFSURLHANDLER_H
#endif /* GUCEF_VFS_CVFSURLHANDLER_H ? */

#ifndef GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#include "gucefVFS_CVfsUriResourceAccessor.h"
#define GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H
#endif /* GUCEF_VFS_CVFSURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_VFS_CASYNCVFSOPERATION_H
#include "gucefVFS_CAsyncVfsOperation.h"
#define GUCEF_VFS_CASYNCVFSOPERATION_H
#endif /* GUCEF_VFS_CASYNCVFSOPERATION_H ? */

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

void
CVfsGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex scopeLock( g_datalock );
    GUCEF_DELETE g_instance;
    g_instance = NULL;
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
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().RegisterConcreteFactory( CVfsUriResourceAccessor::SchemeName, &g_vfsUriResourceAccessorFactory );
    
    /*
     *      register all codecs/handlers/notifiers
     */
    CVFSURLHandler::Register();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefVFS Global systems initialized" );
}

/*-------------------------------------------------------------------------*/

CVfsGlobal::CVfsGlobal( void )
    : m_vfs( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsGlobal::~CVfsGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefVFS global systems" );

    /* 
     *  Unregister handlers
     */
    CVFSURLHandler::Unregister();
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAsyncVfsOperation::TaskType ); 
    CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory().UnregisterConcreteFactory( CVfsUriResourceAccessor::SchemeName );

    /*
     *      cleanup all singletons
     *      Take care to deinstance them in the correct order !!!
     */

    GUCEF_DELETE m_vfs;
    m_vfs = NULL;
}

/*-------------------------------------------------------------------------*/

CVFS&
CVfsGlobal::GetVfs( void )
{GUCEF_TRACE;

    return *m_vfs;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
