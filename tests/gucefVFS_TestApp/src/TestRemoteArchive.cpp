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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#include "TestRemoteArchive.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::map< CORE::CString, CTestRemoteArchive::SPlace > TPlaceMap;

MT::CMutex g_placeLock;

TPlaceMap&
Places( void )
{
    static TPlaceMap places;
    return places;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------*/

const CORE::CString CTestRemoteArchive::TypeName = "TestRemote";

/*-------------------------------------------------------------------------*/

void
CTestRemoteArchive::RegisterPlace( const CORE::CString& name, const SPlace& place )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_placeLock );
    Places()[ name ] = place;
}

/*-------------------------------------------------------------------------*/

void
CTestRemoteArchive::UnregisterPlace( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_placeLock );
    Places().erase( name );
}

/*-------------------------------------------------------------------------*/

CTestRemoteArchive::CTestRemoteArchive( void )
    : VFS::CVfsRemoteArchive()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTestRemoteArchive::~CTestRemoteArchive()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteArchive::LoadArchive( const VFS::CArchiveSettings& settings )
{GUCEF_TRACE;

    SPlace place;
    {
        MT::CScopeMutex lock( g_placeLock );
        TPlaceMap::const_iterator i = Places().find( settings.GetActualArchivePath().IsNULLOrEmpty() ? settings.GetArchivePath() : settings.GetActualArchivePath() );
        if ( i == Places().end() )
            return false;
        place = i->second;
    }
    if ( place.backend.IsNULL() )
        return false;

    VFS::CVfsRemoteSettings coreSettings( place.settings );
    if ( coreSettings.cacheIdentity.IsNULLOrEmpty() )
        coreSettings.cacheIdentity = settings.GetArchiveName();

    VFS::CVfsRemoteArchiveCorePtr core = VFS::CVfsRemoteArchiveCore::Create( place.backend, place.cache, coreSettings, place.scheduler, place.policies );
    place.backend->SetLinkListener( CORE::CTWeakPtr< VFS::CVfsRemoteArchiveCore, MT::CMutex >( core ) );
    AttachCore( core, settings.GetArchiveName() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteArchive::LoadArchive( const CORE::CString& /* archiveName */       ,
                                 VFS::TBasicVfsResourcePtr /* vfsResource */  ,
                                 const bool /* writeableRequest */            )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestRemoteArchive::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

static CTestRemoteArchive::TUnloadHook g_unloadHook = GUCEF_NULL;

void
CTestRemoteArchive::SetUnloadHook( TUnloadHook hook )
{GUCEF_TRACE;

    g_unloadHook = hook;
}

/*-------------------------------------------------------------------------*/

bool
CTestRemoteArchive::UnloadArchive( void )
{GUCEF_TRACE;

    TUnloadHook hook = g_unloadHook;
    if ( GUCEF_NULL != hook )
        hook();
    return VFS::CVfsRemoteArchive::UnloadArchive();
}

/*-------------------------------------------------------------------------*/
