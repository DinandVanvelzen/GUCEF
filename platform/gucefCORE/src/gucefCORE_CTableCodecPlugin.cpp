/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#include <string.h>
#include <exception>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#include "gucefCORE_CTableCodecPlugin.h"

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

enum ETableCodecPluginFuncPtrType
{
    TCPLUGINFUNCPTR_APIVERSION  ,
    TCPLUGINFUNCPTR_INIT        ,
    TCPLUGINFUNCPTR_CODECCOUNT  ,
    TCPLUGINFUNCPTR_CREATECODEC ,
    TCPLUGINFUNCPTR_SHUTDOWN    ,
    TCPLUGINFUNCPTR_DESCRIPTION ,
    TCPLUGINFUNCPTR_COPYRIGHT   ,
    TCPLUGINFUNCPTR_VERSION     ,

    TCPLUGINFUNCPTR_COUNT
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The modules that are linked and not shut down: a module is initialized once at a time */
class CActiveModules
{
    public:

    bool TryAdd( void* moduleHandle )
    {GUCEF_TRACE;

        MT::CObjectScopeLock lock( &m_lock );
        for ( TModuleVector::iterator i = m_modules.begin(); i != m_modules.end(); ++i )
            if ( *i == moduleHandle )
                return false;
        m_modules.push_back( moduleHandle );
        return true;
    }

    void Remove( void* moduleHandle )
    {GUCEF_TRACE;

        MT::CObjectScopeLock lock( &m_lock );
        for ( TModuleVector::iterator i = m_modules.begin(); i != m_modules.end(); ++i )
        {
            if ( *i == moduleHandle )
            {
                m_modules.erase( i );
                return;
            }
        }
    }

    private:

    typedef GUCEF::vector< void* > TModuleVector;

    MT::CMutex m_lock;
    TModuleVector m_modules;
};

CActiveModules&
GetActiveModules( void )
{GUCEF_TRACE;

    static CActiveModules activeModules;
    return activeModules;
}

CString
ApiVersionToString( UInt32 apiVersion )
{GUCEF_TRACE;

    return UInt32ToString( apiVersion >> 16 ) + "." + UInt32ToString( apiVersion & 0xFFFF );
}

}

/*-------------------------------------------------------------------------*/

CTableCodecPlugin::CTableCodecPlugin( void )
    : CIPlugin()
    , m_moduleHandle( GUCEF_NULL )
    , m_lifetime( GUCEF_NEW CTableCodecModuleLifetime() )
    , m_codecs()
    , m_registeredTypeNames()
    , m_metaData()
    , m_initialized( false )
{GUCEF_TRACE;

    memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
}

/*-------------------------------------------------------------------------*/

CTableCodecPlugin::~CTableCodecPlugin()
{GUCEF_TRACE;

    // A module that is still in use stays as it is, its code is needed by whoever holds its codecs. The module stays loaded
    // by the plugin control too, see CIPlugin::IsModuleInUse()
    if ( m_initialized && !IsModuleInUse() )
        OnBeforeModuleUnload();
}

/*-------------------------------------------------------------------------*/

TPluginMetaDataPtr
CTableCodecPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CTableCodecPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecPlugin::IsLinked( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecPlugin::IsApiVersionCompatible( UInt32 pluginApiVersion, UInt32 hostApiVersion )
{GUCEF_TRACE;

    return ( pluginApiVersion >> 16 ) == ( hostApiVersion >> 16 ) &&
           ( pluginApiVersion & 0xFFFF ) <= ( hostApiVersion & 0xFFFF );
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecPlugin::Link( void* modulePtr                   ,
                         TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( m_initialized || GUCEF_NULL == modulePtr || pluginMetaData.IsNULL() )
        return false;

    // The version comes first: it is the one export that says the module is a table codec plugin. Modules that are not,
    // for example when a plugin manager is found by trial and error, are left alone
    TDefaultFuncPtr apiVersionFunc = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_GetApiVersion", 0 ).funcPtr;
    if ( GUCEF_NULL == apiVersionFunc )
        return false;

    UInt32 apiVersion = 0;
    try
    {
        apiVersion = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_GetApiVersion >( apiVersionFunc )();
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Exception while asking for the API version of module " << PointerToString( modulePtr ) );
        return false;
    }

    if ( !IsApiVersionCompatible( apiVersion, GUCEF_TABLECODECPLUGIN_API_VERSION ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Module " << pluginMetaData->GetModuleFilename() << " has table codec plugin API version " <<
                ApiVersionToString( apiVersion ) << " which does not work with API version " << ApiVersionToString( GUCEF_TABLECODECPLUGIN_API_VERSION ) <<
                " of this gucefCORE, the module is refused" );
        return false;
    }

    TDefaultFuncPtr funcPointers[ TCPLUGINFUNCPTR_COUNT ];
    memset( funcPointers, 0, sizeof( funcPointers ) );
    funcPointers[ TCPLUGINFUNCPTR_APIVERSION ] = apiVersionFunc;
    funcPointers[ TCPLUGINFUNCPTR_INIT ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_Init", sizeof( void* ) ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_CODECCOUNT ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_GetCodecCount", 0 ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_CREATECODEC ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_CreateCodec", sizeof( UInt32 ) ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_SHUTDOWN ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_Shutdown", 0 ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_DESCRIPTION ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_Description", 0 ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_COPYRIGHT ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_Copyright", 0 ).funcPtr;
    funcPointers[ TCPLUGINFUNCPTR_VERSION ] = GetFunctionAddress( modulePtr, "TABLECODECPLUGIN_Version", sizeof( void* ) ).funcPtr;

    for ( int i = 0; i < TCPLUGINFUNCPTR_COUNT; ++i )
    {
        if ( GUCEF_NULL == funcPointers[ i ] )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Failed to locate all required exports in module " << pluginMetaData->GetModuleFilename() );
            return false;
        }
    }

    if ( !GetActiveModules().TryAdd( modulePtr ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Module " << pluginMetaData->GetModuleFilename() <<
                " is still in use from an earlier load, its codecs are referenced. It is not initialized a second time" );
        return false;
    }

    m_moduleHandle = modulePtr;
    memcpy( m_funcPointers, funcPointers, sizeof( m_funcPointers ) );
    m_lifetime->SetModuleApiVersion( apiVersion );

    CValueList pluginParams;
    pluginMetaData->GetParams( pluginParams );

    bool initOk = false;
    try
    {
        Int32 initStatus = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_Init >( m_funcPointers[ TCPLUGINFUNCPTR_INIT ] )( &pluginParams );
        initOk = ( 0 == initStatus );
        if ( !initOk )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Init of module " << pluginMetaData->GetModuleFilename() << " failed with status " << initStatus );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Exception while invoking Init on module " << pluginMetaData->GetModuleFilename() << ". what: " << e.what() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Unknown exception while invoking Init on module " << pluginMetaData->GetModuleFilename() );
    }

    m_initialized = initOk;
    if ( initOk )
    {
        try
        {
            UInt32 codecCount = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_GetCodecCount >( m_funcPointers[ TCPLUGINFUNCPTR_CODECCOUNT ] )();
            for ( UInt32 i = 0; i < codecCount; ++i )
            {
                CITableCodec* codec = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_CreateCodec >( m_funcPointers[ TCPLUGINFUNCPTR_CREATECODEC ] )( i );
                if ( GUCEF_NULL == codec )
                {
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Module " << pluginMetaData->GetModuleFilename() << " made no codec for index " << i );
                    continue;
                }
                // behind a guard that counts the objects the codec makes, the module stays loaded while one is alive
                CTSharedPtr< CITableCodec, MT::CMutex > ownedCodec( GUCEF_NEW CGuardedTableCodec( codec, m_lifetime, false ) );
                m_codecs.push_back( TTableCodecPtr( ownedCodec ) );
                m_registeredTypeNames.push_back( CString() );
            }

            m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );
            const char* description = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_Description >( m_funcPointers[ TCPLUGINFUNCPTR_DESCRIPTION ] )();
            const char* copyright = reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_Copyright >( m_funcPointers[ TCPLUGINFUNCPTR_COPYRIGHT ] )();
            TVersion version;
            memset( &version, 0, sizeof( version ) );
            reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_Version >( m_funcPointers[ TCPLUGINFUNCPTR_VERSION ] )( &version );
            m_metaData->SetDescription( GUCEF_NULL != description ? CString( description ) : CString() );
            m_metaData->SetCopyright( GUCEF_NULL != copyright ? CString( copyright ) : CString() );
            m_metaData->SetVersion( version );
            m_metaData->SetPluginType( "GucefTableCodecPlugin" );

            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Linked module " << pluginMetaData->GetModuleFilename() << " with API version " <<
                    ApiVersionToString( apiVersion ) << " and " << (UInt32) m_codecs.size() << " codecs" );
            return true;
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Exception while making the codecs of module " << pluginMetaData->GetModuleFilename() );
        }
    }

    // The module is not used after this, whatever its Shutdown does
    if ( initOk || GUCEF_NULL != m_funcPointers[ TCPLUGINFUNCPTR_SHUTDOWN ] )
    {
        m_codecs.clear();
        m_registeredTypeNames.clear();
        m_initialized = true;
        Shutdown();
    }
    GetActiveModules().Remove( modulePtr );
    memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
    m_moduleHandle = GUCEF_NULL;
    m_metaData.Unlink();
    m_initialized = false;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CTableCodecPlugin::RegisterCodecs( CTableCodecRegistry& registry )
{GUCEF_TRACE;

    UInt32 registered = 0;
    for ( size_t i = 0; i < m_codecs.size(); ++i )
    {
        CString typeName;
        try
        {
            typeName = m_codecs[ i ]->GetTypeName();
        }
        catch ( ... )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Exception while asking a codec for its type name" );
            continue;
        }

        if ( typeName.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: A codec has no type name and is left out" );
            continue;
        }

        if ( registry.TryRegister( typeName, m_codecs[ i ] ) )
        {
            m_registeredTypeNames[ i ] = typeName;
            ++registered;
        }
        else
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: A table codec with type name \"" << typeName << "\" is already registered, the one of this plugin is left out" );
        }
    }
    return registered;
}

/*-------------------------------------------------------------------------*/

void
CTableCodecPlugin::UnregisterCodecs( CTableCodecRegistry& registry )
{GUCEF_TRACE;

    for ( size_t i = 0; i < m_codecs.size(); ++i )
    {
        if ( m_registeredTypeNames[ i ].IsNULLOrEmpty() )
            continue;

        // Only what this plugin registered: the name may have been taken by another codec since
        CTableCodecRegistry::TTableCodecPtr registered;
        if ( registry.TryLookup( m_registeredTypeNames[ i ], registered ) && registered.GetPointer() == m_codecs[ i ].GetPointer() )
            registry.TryUnregister( m_registeredTypeNames[ i ] );
        registered.Unlink();
        m_registeredTypeNames[ i ].Clear();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CTableCodecPlugin::GetCodecCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_codecs.size();
}

/*-------------------------------------------------------------------------*/

void
CTableCodecPlugin::GetRegisteredTypeNames( CString::StringVector& typeNames ) const
{GUCEF_TRACE;

    for ( size_t i = 0; i < m_registeredTypeNames.size(); ++i )
        if ( !m_registeredTypeNames[ i ].IsNULLOrEmpty() )
            typeNames.push_back( m_registeredTypeNames[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecPlugin::IsModuleInUse( void ) const
{GUCEF_TRACE;

    if ( !m_initialized )
        return false;

    // Objects made by the code of the module (readers, writers, metadata, clones of codecs) keep it as well, whether or not
    // the codec they came from is still held
    if ( m_lifetime->GetLiveObjectCount() > 0 )
        return true;

    // Only this plugin holds a codec when its reference count is 1. The registry holds the others while registered,
    // so this is meaningful once UnregisterCodecs() ran
    for ( size_t i = 0; i < m_codecs.size(); ++i )
        if ( m_codecs[ i ].GetReferenceCount() > 1 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTableCodecPlugin::OnBeforeModuleUnload( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;

    // The codecs go first: their code is in the module
    m_codecs.clear();
    m_registeredTypeNames.clear();
    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CTableCodecPlugin::Shutdown( void )
{GUCEF_TRACE;

    if ( !m_initialized )
        return;

    try
    {
        reinterpret_cast< TGUCEFTABLECODECPLUGFPTR_Shutdown >( m_funcPointers[ TCPLUGINFUNCPTR_SHUTDOWN ] )();
    }
    catch ( const std::exception& e )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Exception while invoking Shutdown on module " << PointerToString( m_moduleHandle ) << ". what: " << e.what() );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPlugin: Unknown exception while invoking Shutdown on module " << PointerToString( m_moduleHandle ) );
    }

    GetActiveModules().Remove( m_moduleHandle );
    memset( m_funcPointers, 0, sizeof( m_funcPointers ) );
    m_moduleHandle = GUCEF_NULL;
    m_metaData.Unlink();
    m_initialized = false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
