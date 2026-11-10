/*
 *  gucefINPUT: GUCEF module providing input device interaction
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

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef INPUTDRIVERPLUGINSTRUCTS_H
#include "inputdriverpluginstructs.h"   /* plugin API structs */
#define INPUTDRIVERPLUGINSTRUCTS_H
#endif /* INPUTDRIVERPLUGINSTRUCTS_H ? */

#include "CInputDriverPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum
{
        INPUTDRIVERPLUG_INIT            = 0        ,
        INPUTDRIVERPLUG_SHUTDOWN                   ,
        INPUTDRIVERPLUG_NAME                       ,
        INPUTDRIVERPLUG_COPYRIGHT                  ,
        INPUTDRIVERPLUG_VERSION                    ,

        INPUTDRIVERPLUG_UPDATE                     ,

        INPUTDRIVERPLUG_CREATECONTEXT              ,
        INPUTDRIVERPLUG_DESTROYCONTEXT             ,

        INPUTDRIVERPLUG_LASTPTR
};

/*-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Init )                   ( void** plugdata, const int argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Shutdown )                 ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Name )              ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Copyright )         ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const CORE::TVersion* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Version ) ( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_Update )      ( void* plugdata, void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_CreateContext )  ( void* plugdata, void** contextdata, int argc, const char** argv, const TInputCallbacks* callbacks ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TINPUTDRIVERPLUGFPTR_DestroyContext ) ( void* plugdata, void* contextdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class for internal use only.
 *  Used to store our per-context data
 */
class CPluginInputContext : public CInputContext
{
    private:

    void* m_contextData;

    public:

    CPluginInputContext( CInputDriverPlugin* pluginAPI  ,
                         const CORE::CValueList& params ,
                         void* data = NULL              )
     : CInputContext( *pluginAPI, params )  ,
       m_contextData( data )
    {GUCEF_TRACE;
    }

    virtual ~CPluginInputContext()
    {GUCEF_TRACE;

    }

    inline CInputDriverPlugin* GetPluginAPI( void )
    {GUCEF_TRACE;

        return static_cast< CInputDriverPlugin* >( GetDriver() );
    }

    inline void* GetContextData( void )
    {GUCEF_TRACE;

        return m_contextData;
    }

    inline void** GetContextDataPtr( void )
    {
        return &m_contextData;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseAttached( void* userData       ,
                                     const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->AddMouse( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseDetached( void* userData       ,
                                     const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->RemoveMouse( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnKeyboardAttached( void* userData       ,
                                        const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->AddKeyboard( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnKeyboardDetached( void* userData       ,
                                        const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->RemoveKeyboard( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnDeviceAttached( void* userData       ,
                                      const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->AddDevice( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnDeviceDetached( void* userData       ,
                                      const Int32 deviceID ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    static_cast<CPluginInputContext*>( userData )->GetPluginAPI()->RemoveDevice( deviceID );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseButtonDown( void* userData           ,
                                       const Int32 deviceID     ,
                                       const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectMouseButtonChange( context->GetID() ,
                                                      deviceID         ,
                                                      buttonindex      ,
                                                      true             );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseButtonUp( void* userData           ,
                                     const Int32 deviceID     ,
                                     const UInt32 buttonindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectMouseButtonChange( context->GetID() ,
                                                      deviceID         ,
                                                      buttonindex      ,
                                                      false            );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseMove( void* userData        ,
                                 const Int32 deviceID  ,
                                 const Int32 xPos      ,
                                 const Int32 yPos      ,
                                 const Int32 xDelta    ,
                                 const Int32 yDelta    ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectMouseMove( context->GetID() ,
                                              deviceID         ,
                                              xPos             ,
                                              yPos             ,
                                              xDelta           ,
                                              yDelta           );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnMouseVarChanged( void* userData         ,
                                       const Int32 deviceID   ,
                                       const UInt32 varIndex  ,
                                       const Int32 value      ,
                                       const Int32 valueDelta ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectMouseVariableChange( context->GetID() ,
                                                        deviceID         ,
                                                        varIndex         ,
                                                        value            ,
                                                        valueDelta       );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnKeyboardText( void* userData       ,
                                    const Int32 deviceID ,
                                    const char* utf8Text ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL == utf8Text || '\0' == *utf8Text )
        return;
    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectKeyboardText( context->GetID(), deviceID, utf8Text );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnKeyboardKeyDown( void* userData            ,
                                       const Int32 deviceID      ,
                                       const KeyCode keyCode     ,
                                       const UInt32 keyModStates ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectKeyboardKeyChange( context->GetID() ,
                                                      deviceID         ,
                                                      keyCode          ,
                                                      true             );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnKeyboardKeyUp( void* userData            ,
                                     const Int32 deviceID      ,
                                     const KeyCode keyCode     ,
                                     const UInt32 keyModStates ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    CPluginInputContext* context = static_cast<CPluginInputContext*>( userData );
    context->GetPluginAPI()->InjectKeyboardKeyChange( context->GetID() ,
                                                      deviceID         ,
                                                      keyCode          ,
                                                      false            );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnDeviceBooleanOff( void* userData          ,
                                        const Int32 deviceid    ,
                                        const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    //@TODO
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnDeviceBooleanOn( void* userData          ,
                                       const Int32 deviceid    ,
                                       const UInt32 stateindex ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    //@TODO
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
CInputDriverPlugin::OnDeviceVarChanged( void* userData          ,
                                        const Int32 deviceid    ,
                                        const UInt32 stateindex ,
                                        const Float32 value     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    //@TODO
}

/*-------------------------------------------------------------------------*/

CInputDriverPlugin::CInputDriverPlugin( void )
        : CInputDriver()     ,
          m_sohandle( NULL ) ,
          m_plugdata( NULL ) ,
          m_metaData()
{GUCEF_TRACE;

    memset( m_fptable, NULL, sizeof( m_fptable ) );
}

/*-------------------------------------------------------------------------*/

CInputDriverPlugin::~CInputDriverPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

const CORE::TVersion*
CInputDriverPlugin::GetVersion( void )
{GUCEF_TRACE;

    return ( (TINPUTDRIVERPLUGFPTR_Version) m_fptable[ INPUTDRIVERPLUG_VERSION ] )( &m_plugdata );
}

/*-------------------------------------------------------------------------*/

bool
CInputDriverPlugin::Link( void* modulePtr                         ,
                          CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( NULL == modulePtr ) return false;

    Unlink();
    m_sohandle = modulePtr;

    /*
     *      Link up generic module utilities
     */
    m_fptable[ INPUTDRIVERPLUG_INIT ] = CORE::GetFunctionAddress( m_sohandle                                      ,
                                                                  "INPUTDRIVERPLUG_Init"                          ,
                                                                  sizeof(void**)+sizeof(int)+sizeof(const char**) ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_SHUTDOWN ] = CORE::GetFunctionAddress( m_sohandle                 ,
                                                                      "INPUTDRIVERPLUG_Shutdown" ,
                                                                      1*sizeof(void*)            ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_NAME ] = CORE::GetFunctionAddress( m_sohandle             ,
                                                                  "INPUTDRIVERPLUG_Name" ,
                                                                  1*sizeof(void*)        ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_COPYRIGHT ] = CORE::GetFunctionAddress( m_sohandle                  ,
                                                                       "INPUTDRIVERPLUG_Copyright" ,
                                                                       1*sizeof(void*)             ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_VERSION ] = CORE::GetFunctionAddress( m_sohandle                ,
                                                                     "INPUTDRIVERPLUG_Version" ,
                                                                     1*sizeof(void*)           ).funcPtr;


    /*
     *      Link up driver utilities
     */


    m_fptable[ INPUTDRIVERPLUG_UPDATE ] = CORE::GetFunctionAddress( m_sohandle               ,
                                                                    "INPUTDRIVERPLUG_Update" ,
                                                                    2*sizeof(void*)          ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_CREATECONTEXT ] = CORE::GetFunctionAddress( m_sohandle                      ,
                                                                           "INPUTDRIVERPLUG_CreateContext" ,
                                                                           4*sizeof(void*)                 ).funcPtr;
    m_fptable[ INPUTDRIVERPLUG_DESTROYCONTEXT ] = CORE::GetFunctionAddress( m_sohandle                       ,
                                                                            "INPUTDRIVERPLUG_DestroyContext" ,
                                                                            2*sizeof(void*)                  ).funcPtr;


    if ( ( !m_fptable[ INPUTDRIVERPLUG_INIT ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_SHUTDOWN ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_NAME ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_COPYRIGHT ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_VERSION ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_UPDATE ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_CREATECONTEXT ] ) ||
         ( !m_fptable[ INPUTDRIVERPLUG_DESTROYCONTEXT ] ) )
    {
            memset( m_fptable, NULL, sizeof(void*) * INPUTDRIVERPLUG_LASTPTR );
            m_sohandle = NULL;

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Invalid input driver module: " + CORE::PointerToString( modulePtr ) );
            return false;
    }

    /*
     *      Intialize the plugin module
     */
    CORE::CValueList params;
    pluginMetaData->GetParams( params );
    char** argv = 0; int argc = 0;
    CreateArgMatrix( params, argv, argc );
    ( (TINPUTDRIVERPLUGFPTR_Init) m_fptable[ INPUTDRIVERPLUG_INIT ] )( &m_plugdata, argc, const_cast<const char**>( argv ) );
    DestroyArgMatrix( argv, argc );

    // Fill in the plugin metadata based on what the actual module can tell us
    CORE::TPluginMetaDataStoragePtr metaData = CORE::TPluginMetaDataStoragePtr( new CORE::CPluginMetaData( *pluginMetaData ) );
    metaData->SetDescription( ( (TINPUTDRIVERPLUGFPTR_Name) m_fptable[ INPUTDRIVERPLUG_NAME ] )( m_plugdata ) );
    metaData->SetCopyright( ( (TINPUTDRIVERPLUGFPTR_Copyright) m_fptable[ INPUTDRIVERPLUG_COPYRIGHT ] )( m_plugdata ) );
    metaData->SetVersion( *( ( (TINPUTDRIVERPLUGFPTR_Version) m_fptable[ INPUTDRIVERPLUG_VERSION ] )( m_plugdata ) ) );
    m_metaData = metaData;

    return true;

}

/*-------------------------------------------------------------------------*/

void
CInputDriverPlugin::Unlink( void )
{GUCEF_TRACE;

    if ( NULL != m_sohandle )
    {
        ( (TINPUTDRIVERPLUGFPTR_Shutdown) m_fptable[ INPUTDRIVERPLUG_SHUTDOWN ] )( m_plugdata );
    }
    m_metaData.Unlink();
    memset( m_fptable, NULL, sizeof(void*) * INPUTDRIVERPLUG_LASTPTR );
    m_sohandle = NULL;
}

/*-------------------------------------------------------------------------*/

bool
CInputDriverPlugin::OnUpdate( const UInt64 tickcount               ,
                              const Float64 updateDeltaInMilliSecs ,
                              CInputContext* context               )
{GUCEF_TRACE;

    if ( NULL != m_sohandle )
    {
        return ( (TINPUTDRIVERPLUGFPTR_Update) m_fptable[ INPUTDRIVERPLUG_UPDATE ] )( m_plugdata, static_cast<CPluginInputContext*>( context )->GetContextData() ) > 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CInputDriverPlugin::CreateArgMatrix( const CORE::CValueList& params, char**& argv, int& argc )
{GUCEF_TRACE;

    if ( 0 == params.GetKeyCount() )
    {
        argc = 0;
        argv = 0;
        return;
    }

    argc = params.GetKeyCount()*2;
    argv = new char*[ argc ];

    UInt32 n=0;
    CORE::CString value, key;
    for ( UInt32 i=0; i<params.GetKeyCount(); ++i )
    {
        value = params.GetValue( i );
        key = params.GetKey( i );

        argv[ n ] = new char[ key.Length()+2 ];
        argv[ n ][ 0 ] = '-';
        memcpy( ((argv[ n ])+1), key.C_String(), key.Length()+1 );
        ++n;

        argv[ n ] = new char[ value.Length()+1 ];
        memcpy( argv[ n ], value.C_String(), value.Length()+1 );
        ++n;
    }
}

/*-------------------------------------------------------------------------*/

void
CInputDriverPlugin::DestroyArgMatrix( char** argv, int argc )
{GUCEF_TRACE;

    for ( int i=0; i<argc; ++i )
    {
        delete [](argv[ i ]);
    }
    delete []argv;
}

/*-------------------------------------------------------------------------*/

CInputContext*
CInputDriverPlugin::CreateContext( const CORE::CValueList& params )
{GUCEF_TRACE;

    CPluginInputContext* context( new CPluginInputContext( this, params ) );

    TInputCallbacks callbacks;
    callbacks.onMouseButtonDown  = OnMouseButtonDown;
    callbacks.onMouseButtonUp    = OnMouseButtonUp;
    callbacks.onMouseMove        = OnMouseMove;
    callbacks.onMouseVarChanged  = OnMouseVarChanged;
    callbacks.onKeyboardKeyDown  = OnKeyboardKeyDown;
    callbacks.onKeyboardKeyUp    = OnKeyboardKeyUp;
    callbacks.onDeviceBooleanOff = OnDeviceBooleanOff;
    callbacks.onDeviceBooleanOn  = OnDeviceBooleanOn;
    callbacks.onDeviceVarChanged = OnDeviceVarChanged;
    callbacks.onMouseAttached    = OnMouseAttached;
    callbacks.onKeyboardAttached = OnKeyboardAttached;
    callbacks.onDeviceAttached   = OnDeviceAttached;
    callbacks.onMouseDetached    = OnMouseDetached;
    callbacks.onKeyboardDetached = OnKeyboardDetached;
    callbacks.onDeviceDetached   = OnDeviceDetached;
    callbacks.userData = context;
    callbacks.onKeyboardText     = OnKeyboardText;

    char** argv = 0;
    int argc = 0;
    CreateArgMatrix( params, argv, argc );
    if ( ( (TINPUTDRIVERPLUGFPTR_CreateContext) m_fptable[ INPUTDRIVERPLUG_CREATECONTEXT ] )( m_plugdata, context->GetContextDataPtr(), argc, const_cast<const char**>( argv ), &callbacks ) )
    {
        DestroyArgMatrix( argv, argc );
        return context;
    }
    delete context;
    DestroyArgMatrix( argv, argc );
    return NULL;
}

/*-------------------------------------------------------------------------*/

void
CInputDriverPlugin::DeleteContext( CInputContext* context )
{GUCEF_TRACE;

    ( (TINPUTDRIVERPLUGFPTR_DestroyContext) m_fptable[ INPUTDRIVERPLUG_DESTROYCONTEXT ] )( m_plugdata, static_cast<CPluginInputContext*>( context )->GetContextData() );
    delete static_cast<CPluginInputContext*>( context );
}

/*-------------------------------------------------------------------------*/

CORE::TPluginMetaDataPtr
CInputDriverPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CInputDriverPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_sohandle;
}

/*-------------------------------------------------------------------------*/

CString
CInputDriverPlugin::GetName( void ) const
{GUCEF_TRACE;

    if ( NULL != m_sohandle )
    {
        return ( (TINPUTDRIVERPLUGFPTR_Name) m_fptable[ INPUTDRIVERPLUG_NAME ] )( m_plugdata );
    }
    return CString();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
