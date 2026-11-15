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

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "guidriverOgre_COgrePluginAdapter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

enum TOgrePluginFuncPtrType
{
    OGREPLUGINFUNCPTR_STARTPLUGIN ,
    OGREPLUGINFUNCPTR_STOPPLUGIN  ,

    OGREPLUGINFUNCPTR_COUNT
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TOGREPLUGFPTR_StartPlugin ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TOGREPLUGFPTR_StopPlugin ) ( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

COgrePluginAdapter::COgrePluginAdapter( void )
    : CIPlugin()             ,
      m_moduleHandle( GUCEF_NULL ) ,
      m_modulePath()         ,
      m_params()             ,
      m_metaData()
{GUCEF_TRACE;

    memset( m_funcPointers, 0, OGREPLUGINFUNCPTR_COUNT );
}

/*-------------------------------------------------------------------------*/

COgrePluginAdapter::~COgrePluginAdapter()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

void
COgrePluginAdapter::SetPluginParams( const CORE::CValueList& pluginParams )
{GUCEF_TRACE;

    m_params = pluginParams; 
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
COgrePluginAdapter::GetPluginParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginMetaDataPtr
COgrePluginAdapter::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
COgrePluginAdapter::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::Link( void* modulePtr                         ,
                          CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( IsLoaded() ) return false;
    
    m_moduleHandle = modulePtr;
    if ( GUCEF_NULL != m_moduleHandle )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Linking API using module pointer: " + CORE::PointerToString( modulePtr ) );
            
        // We have now successfully loaded the module itself
        // We will now try to lookup the function pointers
        m_funcPointers[ OGREPLUGINFUNCPTR_STARTPLUGIN ] = CORE::GetFunctionAddress( m_moduleHandle   ,
                                                                                    "dllStartPlugin" ,
                                                                                    0                ).funcPtr;
        m_funcPointers[ OGREPLUGINFUNCPTR_STOPPLUGIN ] = CORE::GetFunctionAddress( m_moduleHandle  ,
                                                                                   "dllStopPlugin" ,
                                                                                   0               ).funcPtr;

        // Verify that all function pointers are loaded correctly
        if ( ( GUCEF_NULL != m_funcPointers[ OGREPLUGINFUNCPTR_STARTPLUGIN ] ) &&
             ( GUCEF_NULL != m_funcPointers[ OGREPLUGINFUNCPTR_STOPPLUGIN ] )  )
        {
            // Call the module's dllStartPlugin()
            reinterpret_cast< TOGREPLUGFPTR_StartPlugin >( m_funcPointers[ OGREPLUGINFUNCPTR_STARTPLUGIN ] )();
                
            // We have loaded & linked our plugin module
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapterCORE::: Successfully loaded module and invoked dllStartPlugin() using module: " + CORE::PointerToString( modulePtr ) );

            // Copy the given metadata 
            m_metaData = CORE::TPluginMetaDataPtr( new CORE::CPluginMetaData( *pluginMetaData ) );                

            return true;                
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "COgrePluginAdapter: Failed to locate the required exports in module: " + CORE::PointerToString( modulePtr ) );
        }

        // We failed to link the functions :(
        memset( m_funcPointers, 0, OGREPLUGINFUNCPTR_COUNT );
        m_moduleHandle = GUCEF_NULL;
        return false;
    }

    // Unloading->Loading must be done explicitly
    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::Unlink( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        // Call the module's Unload()
        reinterpret_cast< TOGREPLUGFPTR_StopPlugin >( m_funcPointers[ OGREPLUGINFUNCPTR_STOPPLUGIN ] )();

        // Cleanup recources
        memset( m_funcPointers, 0, OGREPLUGINFUNCPTR_COUNT );
        m_moduleHandle = GUCEF_NULL;
        m_metaData.Unlink();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
COgrePluginAdapter::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgrePluginAdapter::GetDescription( void ) const
{GUCEF_TRACE;

    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

CORE::CString
COgrePluginAdapter::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Please refer to the Ogre plugin module, most are MIT License";
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
COgrePluginAdapter::GetVersion( void ) const
{GUCEF_TRACE;

    static const CORE::TVersion unknownVersion = { -1, -1, -1, -1 };
    return unknownVersion;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEROGRE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
