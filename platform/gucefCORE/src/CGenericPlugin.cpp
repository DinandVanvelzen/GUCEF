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

#ifndef GUCEF_CORE_GENERICPLUGINAPI_H
#include "gucefCORE_GenericPluginAPI.h"
#define GUCEF_CORE_GENERICPLUGINAPI_H
#endif /* GUCEF_CORE_GENERICPLUGINAPI_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "CGenericPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

enum TGenericPluginFuncPtrType
{
    GPLUGINFUNCPTR_LOAD                   ,
    GPLUGINFUNCPTR_UNLOAD                 ,
    GPLUGINFUNCPTR_VERSION                ,
    GPLUGINFUNCPTR_COPYRIGHT              ,
    GPLUGINFUNCPTR_DESCRIPTION            ,
    GPLUGINFUNCPTR_LINKBACKMODULEDEPS     ,   /* optional */

    GPLUGINFUNCPTR_COUNT
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGenericPlugin::CGenericPlugin( void )
    : CIPlugin()             ,
      m_moduleHandle( NULL ) ,
      m_modulePath()         ,
      m_params()             ,
      m_metaData()
{GUCEF_TRACE;

    memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
}

/*-------------------------------------------------------------------------*/

CGenericPlugin::~CGenericPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

void
CGenericPlugin::SetPluginParams( const CValueList& pluginParams )
{GUCEF_TRACE;

    m_params = pluginParams; 
}

/*-------------------------------------------------------------------------*/

const CValueList&
CGenericPlugin::GetPluginParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

TPluginMetaDataPtr
CGenericPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CGenericPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

bool
CGenericPlugin::Link( void* modulePtr                   ,
                      TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( IsLoaded() ) 
        return false;
    
    m_moduleHandle = modulePtr;
    if ( NULL != m_moduleHandle )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "GenericPlugin: Linking API using module pointer: " + PointerToString( modulePtr ) );
            
        // We have now successfully loaded the module itself
        // We will now try to lookup the function pointers
        m_funcPointers[ GPLUGINFUNCPTR_LOAD ] = GetFunctionAddress( m_moduleHandle                ,
                                                                    "GUCEFPlugin_Load"            ,
                                                                    sizeof(UInt32)+sizeof(char**) ).funcPtr;
        m_funcPointers[ GPLUGINFUNCPTR_UNLOAD ] = GetFunctionAddress( m_moduleHandle       ,
                                                                      "GUCEFPlugin_Unload" ,
                                                                      0                    ).funcPtr;
        m_funcPointers[ GPLUGINFUNCPTR_VERSION ] = GetFunctionAddress( m_moduleHandle           ,
                                                                       "GUCEFPlugin_GetVersion" ,
                                                                       sizeof( TVersion* )      ).funcPtr;
        m_funcPointers[ GPLUGINFUNCPTR_DESCRIPTION ] = GetFunctionAddress( m_moduleHandle               ,
                                                                           "GUCEFPlugin_GetDescription" ,
                                                                           0                            ).funcPtr;

        m_funcPointers[ GPLUGINFUNCPTR_COPYRIGHT ] = GetFunctionAddress( m_moduleHandle             ,
                                                                         "GUCEFPlugin_GetCopyright" ,
                                                                         0                          ).funcPtr;

        // Optional export — may be absent in older plugins
        m_funcPointers[ GPLUGINFUNCPTR_LINKBACKMODULEDEPS ] = GetFunctionAddress( m_moduleHandle                           ,
                                                                                  "GUCEFPlugin_GetLinkBackModuleDependencies" ,
                                                                                  0                                          ).funcPtr;

        // Verify that all mandatory function pointers are loaded correctly
        if ( ( NULL != m_funcPointers[ GPLUGINFUNCPTR_LOAD ] )        &&
             ( NULL != m_funcPointers[ GPLUGINFUNCPTR_UNLOAD ] )      &&
             ( NULL != m_funcPointers[ GPLUGINFUNCPTR_VERSION ] )     &&
             ( NULL != m_funcPointers[ GPLUGINFUNCPTR_DESCRIPTION ] ) &&
             ( NULL != m_funcPointers[ GPLUGINFUNCPTR_COPYRIGHT ] )    )
        {
            // Make the module parameter list
            const char** argv = NULL;
            UInt32 argc = m_params.GetKeyCount();
            if ( argc > 0 )
            {
                argv = GUCEF_NEW const char*[ argc ];
                for ( UInt32 i=0; i<argc; ++i )
                {
                    CString keyValuePair = m_params.GetPair( i );
                    argv[ i ] = GUCEF_NEW char[ keyValuePair.Length()+1 ];
                    memcpy( (void*)argv[ i ], keyValuePair.C_String(), keyValuePair.Length()+1 );
                } 
            }
            
            bool loadInvokeSucces = true;
            Int32 loadStatus = 0;
            try
            {
                // Call the module's Load()
                loadStatus = reinterpret_cast< TGUCEFGENERICPLUGFPTR_Load >( m_funcPointers[ GPLUGINFUNCPTR_LOAD ] )( argc, argv );
            }
            catch ( const timeout_exception& )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "GenericPlugin:Link: Timeout Exception while invoking Load on plugin " + 
                    m_metaData->GetModuleFilename() + " with module address " + PointerToString( m_moduleHandle ) );
                loadInvokeSucces = false;
            }
            catch ( const std::exception& e )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "GenericPlugin:Link: Std Exception while invoking Load on plugin " + 
                    m_metaData->GetModuleFilename() + " with module address " + PointerToString( m_moduleHandle ) + ". what: " + CString( e.what() ) );
                loadInvokeSucces = false;
            }                                                                           
            
            // Cleanup the module parameter list
            if ( argc > 0 )
            {
                for ( UInt32 i=0; i<argc; ++i )
                {
                    delete[] argv[ i ];
                }
                delete[] argv;
                argv = NULL;
            }
                
            // Check whether the load was successfull
            if ( loadInvokeSucces && loadStatus > 0 )
            {
                // We have loaded & linked our plugin module
                GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "GenericPlugin: Successfully loaded module and invoked Load() which returned status " + 
                        Int32ToString( loadStatus  ) + " using module: " + PointerToString( modulePtr ) );

                // Copy the given metadata and update it with info from the actual module
                m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );
                m_metaData->SetDescription( GetDescription() );
                m_metaData->SetCopyright( GetCopyright() );
                m_metaData->SetVersion( GetVersion() );
                // Generic plugins always have this type; ensure it is set in the stored metadata
                // even when loaded via trial-and-error (where the incoming metadata has no type).
                m_metaData->SetPluginType( "GucefGenericPlugin" );

                // If the plugin exports the optional LinkBack dependencies function, use its
                // return value to supplement or override whatever was set in the config metadata.
                // The hardcoded list from the binary is authoritative over the config.
                if ( NULL != m_funcPointers[ GPLUGINFUNCPTR_LINKBACKMODULEDEPS ] )
                {
                    const char* linkBackDeps = reinterpret_cast< TGUCEFGENERICPLUGFPTR_GetLinkBackModuleDependencies >(
                            m_funcPointers[ GPLUGINFUNCPTR_LINKBACKMODULEDEPS ] )();
                    if ( NULL != linkBackDeps && '\0' != linkBackDeps[ 0 ] )
                    {
                        m_metaData->SetAllLinkBackModules( CString( linkBackDeps ) );
                        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "GenericPlugin: Plugin declared LinkBack module dependencies: " + CString( linkBackDeps ) );
                    }
                }

                return true;
            }
            else
            if ( loadInvokeSucces )
            {
                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "GenericPlugin: Failed load module because Load returned an error state (" + 
                                                        Int32ToString( loadStatus ) + ") with module: " + PointerToString( modulePtr ) );

                // Call the module's Unload()
                reinterpret_cast< TGUCEFGENERICPLUGFPTR_Unload >( m_funcPointers[ GPLUGINFUNCPTR_UNLOAD ] )();
            }
                
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CGenericPlugin: Failed to locate the required exports in module: " + PointerToString( modulePtr ) );
        }

        // We failed to link the functions :(
        memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
        m_moduleHandle = NULL;
        return false;
    }

    // Unloading->Loading must be done explicitly
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGenericPlugin::Unlink( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        try
        {
            // Call the module's Unload()
            reinterpret_cast< TGUCEFGENERICPLUGFPTR_Unload >( m_funcPointers[ GPLUGINFUNCPTR_UNLOAD ] )();
        }
        catch ( const timeout_exception& )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CGenericPlugin:Unlink: Timeout Exception while invoking Unload on plugin " + 
                m_metaData->GetModuleFilename() + " with module address " + PointerToString( m_moduleHandle ) );
            return false;
        }
        catch ( const std::exception& e )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "CGenericPlugin:Unlink: Std Exception while invoking Unload on plugin " + 
                m_metaData->GetModuleFilename() + " with module address " + PointerToString( m_moduleHandle ) + ". what: " + CString( e.what() ) );
            return false;
        }

        // Cleanup recources
        memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
        m_moduleHandle = NULL;
        m_metaData.Unlink();
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGenericPlugin::IsLoaded( void ) const
{GUCEF_TRACE;

    return NULL != m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

CString
CGenericPlugin::GetDescription( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        return reinterpret_cast< TGUCEFGENERICPLUGFPTR_GetDescription >( m_funcPointers[ GPLUGINFUNCPTR_DESCRIPTION ] )();
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

CString
CGenericPlugin::GetCopyright( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        return reinterpret_cast< TGUCEFGENERICPLUGFPTR_GetCopyright >( m_funcPointers[ GPLUGINFUNCPTR_COPYRIGHT ] )();
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

TVersion
CGenericPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        TVersion versionInfo;
        reinterpret_cast< TGUCEFGENERICPLUGFPTR_GetVersion >( m_funcPointers[ GPLUGINFUNCPTR_VERSION ] )( &versionInfo );

        return versionInfo;
    }

    TVersion unknownVersion = { -1, -1, -1, -1 };
    return unknownVersion;
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
