/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVE_H
#define GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#include "TestRemoteBackend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A remote archive for the tests: CVfsRemoteArchive over a CTestRemoteBackend. The place it works on (the backend, the block
 *  cache, the settings and the scheduler) is registered under a name beforehand, and the archive path of the settings it is
 *  mounted with is that name, so that it can be made by the factory of a VFS like any other archive type.
 */
class CTestRemoteArchive : public GUCEF::VFS::CVfsRemoteArchive
{
    public:

    static const GUCEF::CORE::CString TypeName;

    typedef GUCEF::CORE::CTFactory< GUCEF::VFS::CArchive, CTestRemoteArchive, GUCEF::MT::CMutex > TFactory;

    struct SPlace
    {
        CTestRemoteBackendPtr backend;
        GUCEF::VFS::CVfsBlockCachePtr cache;
        GUCEF::VFS::CVfsRemoteSettings settings;
        GUCEF::VFS::CVfsFetchScheduler* scheduler;
        GUCEF::VFS::CVfsThreadPolicyRegistry* policies;
        SPlace( void ) : backend(), cache(), settings(), scheduler( GUCEF_NULL ), policies( GUCEF_NULL ) {}
    };

    static void RegisterPlace( const GUCEF::CORE::CString& name, const SPlace& place );

    static void UnregisterPlace( const GUCEF::CORE::CString& name );

    CTestRemoteArchive( void );

    virtual ~CTestRemoteArchive() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const GUCEF::VFS::CArchiveSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadArchive( const GUCEF::CORE::CString& archiveName       ,
                              GUCEF::VFS::TBasicVfsResourcePtr vfsResource  ,
                              const bool writeableRequest                   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Calls the hook (when there is one) before the archive is unloaded: for the case that looks at what is going on in the place at that moment */
    virtual bool UnloadArchive( void ) GUCEF_VIRTUAL_OVERRIDE;

    typedef void ( *TUnloadHook )( void );
    static void SetUnloadHook( TUnloadHook hook );
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTREMOTEARCHIVE_H ? */
