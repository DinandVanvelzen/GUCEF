/*
 *  guidriverWin32GL: module implementing GL based window management for Win32
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_GUI_CWINDOWMANAGER_H
#include "gucefGUI_CWindowManager.h"
#define GUCEF_GUI_CWINDOWMANAGER_H
#endif /* GUCEF_GUI_CWINDOWMANAGER_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H
#include "guidriverOgre_COgreWindowManagerImp.h"
#define GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H
#endif /* GUIDRIVEROGRE_COGREWINDOWMANAGERIMP_H ? */

#ifndef GUCEF_CORE_COGREPLUGINMANAGER_H
#include "guidriverOgre_COgrePluginManager.h"
#define GUCEF_CORE_COGREPLUGINMANAGER_H
#endif /* GUCEF_CORE_COGREPLUGINMANAGER_H ? */

#ifndef GUIDRIVEROGRE_COGRELOGADAPTER_H
#include "guidriverOgre_COgreLogAdapter.h"
#define GUIDRIVEROGRE_COGRELOGADAPTER_H
#endif /* GUIDRIVEROGRE_COGRELOGADAPTER_H ? */

#include "guidriverOgre_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static GUIDRIVEROGRE::COgreLogAdapter ogreLogAdapter;
static GUIDRIVEROGRE::COgrePluginManager ogrePluginManager;
static TOgreWindowManagerImpPtr ogreWindowManager = TOgreWindowManagerImpPtr( new COgreWindowManagerImp() );

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUI::CGuiGlobal::Instance()->GetWindowManager().RegisterBackend( "Ogre", ogreWindowManager );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUI::CGuiGlobal::Instance()->GetWindowManager().UnregisterBackend( "Ogre" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 0; 
    versionInfo->minor = 1;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;
    
    return "Copyright (C) Dinand Vanvelzen, released under LGPLv3";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX 
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Window management backend using Ogre";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefGUI";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/