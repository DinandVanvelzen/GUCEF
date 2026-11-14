/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#include "gucefCORE_CLoaderDelegatedPluginLoadLogic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The following are type definitions for the gucefLOADER module C interface functions we need
 */
typedef void* ( GUCEF_CALLSPEC_PREFIX *TGUCEFLOADERCINTERFACE_LoadModule ) ( const char* moduleFilename  ,
                                                                             const char* rootDir         ,
                                                                             const char* moduleGroupName ,
                                                                             long majorVersion           ,
                                                                             long minorVersion           ,
                                                                             long patchVersion           ,
                                                                             long releaseVersion         ) GUCEF_CALLSPEC_SUFFIX;
typedef void ( GUCEF_CALLSPEC_PREFIX *TGUCEFLOADERCINTERFACE_UnloadModule ) ( void* modulePtr ) GUCEF_CALLSPEC_SUFFIX;


/*-------------------------------------------------------------------------*/

struct SGucefLoaderCInterface
{
    TGUCEFLOADERCINTERFACE_LoadModule loadModule;
    TGUCEFLOADERCINTERFACE_UnloadModule unloadModule;
};
typedef struct SGucefLoaderCInterface TGucefLoaderCInterface;


/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void*
CLoaderDelegatedPluginLoadLogic::LoadPlugin( const CString& rootDir        ,
                                             const CString& moduleName     ,
                                             const CString& groupName      ,
                                             const TVersion* pluginVersion )
{GUCEF_TRACE;

    Initialize();

    TGucefLoaderCInterface* cInterface = (TGucefLoaderCInterface*) m_cInterface;
    if ( GUCEF_NULL == cInterface ) return GUCEF_NULL;

    TVersion moduleVersion;
    memset( &moduleVersion, -1, sizeof(moduleVersion) );
    if ( 0 != pluginVersion )
    {
        moduleVersion = *pluginVersion;
    }

    return cInterface->loadModule( moduleName.C_String() ,
                                   rootDir.C_String()    ,
                                   groupName.C_String()  ,
                                   moduleVersion.major   ,
                                   moduleVersion.minor   ,
                                   moduleVersion.patch   ,
                                   moduleVersion.release );
}

/*-------------------------------------------------------------------------*/

void
CLoaderDelegatedPluginLoadLogic::UnloadPlugin( void* modulePtr )
{GUCEF_TRACE;

    Initialize();

    TGucefLoaderCInterface* cInterface = (TGucefLoaderCInterface*) m_cInterface;
    if ( GUCEF_NULL == cInterface ) return;

    cInterface->unloadModule( modulePtr );
}

/*-------------------------------------------------------------------------*/

void
CLoaderDelegatedPluginLoadLogic::Initialize( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_cInterface )
    {
        // Link to the already loaded gucefLOADER module
        void* modulePtr = GetModulePointer( "gucefLOADER" );
        if ( GUCEF_NULL == modulePtr )
        {
            modulePtr = GetModulePointer( "gucefLOADER_d" );
        }
        if ( GUCEF_NULL != modulePtr )
        {
            TGucefLoaderCInterface* cInterface = GUCEF_NEW TGucefLoaderCInterface();
            memset( cInterface, 0, sizeof(TGucefLoaderCInterface) );

            cInterface->loadModule = (TGUCEFLOADERCINTERFACE_LoadModule) GetFunctionAddress( modulePtr, "LoadSpecificModule", 3*sizeof(const char*) + 4*sizeof(long) ).funcPtr;
            cInterface->unloadModule = (TGUCEFLOADERCINTERFACE_UnloadModule) GetFunctionAddress( modulePtr, "UnloadSpecificModule", sizeof(void*) ).funcPtr;

            if ( ( GUCEF_NULL != cInterface->loadModule )   &&
                 ( GUCEF_NULL != cInterface->unloadModule )  )
            {
                m_cInterface = cInterface;
            }
            else
            {
                GUCEF_DELETE cInterface;
                m_cInterface = GUCEF_NULL;
            }
        }
    }

}

/*-------------------------------------------------------------------------*/

CLoaderDelegatedPluginLoadLogic::CLoaderDelegatedPluginLoadLogic( void )
    : m_cInterface( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLoaderDelegatedPluginLoadLogic::~CLoaderDelegatedPluginLoadLogic()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_cInterface )
    {
        GUCEF_DELETE (TGucefLoaderCInterface*) m_cInterface;
        m_cInterface = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CString
CLoaderDelegatedPluginLoadLogic::GetLoaderLogicTypeName( void ) const
{GUCEF_TRACE;

    return "gucefLOADER";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
