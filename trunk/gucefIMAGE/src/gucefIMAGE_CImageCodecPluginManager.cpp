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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#include "gucefIMAGE_CImageCodecPlugin.h"
#define GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#endif /* GUCEF_IMAGE_CIMAGECODECPLUGIN_H ? */

#include "gucefIMAGE_CImageCodecPluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageCodecPluginManager::CImageCodecPluginManager( void )
    : CPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageCodecPluginManager::~CImageCodecPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::TPluginPtr
CImageCodecPluginManager::RegisterPlugin( void* modulePtr                         ,
                                          CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    CImageCodecPlugin* plugin = new CImageCodecPlugin();
    if ( plugin->Link( modulePtr      ,
                       pluginMetaData ) )
    {
        return plugin;
    }

    delete plugin;
    return NULL;
}

/*-------------------------------------------------------------------------*/

void
CImageCodecPluginManager::UnregisterPlugin( CORE::TPluginPtr plugin )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CImageCodecPluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return "GucefImageCodecPlugin";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
