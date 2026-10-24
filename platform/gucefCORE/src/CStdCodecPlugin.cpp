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
#include <stddef.h>
#include <assert.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#include "gucefCORE_CTVariantMapAdapter_Cpp2C.h"
#define GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#endif /* GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H ? */

#ifndef GUCEF_CORE_C_API_IMP_H
#include "gucefCORE_c_api_impl.h"
#define GUCEF_CORE_C_API_IMP_H
#endif /* GUCEF_CORE_C_API_IMP_H ? */

#ifndef GUCEF_CORE_CSTDCODECPLUGINITEM_H
#include "CStdCodecPluginItem.h"
#define GUCEF_CORE_CSTDCODECPLUGINITEM_H
#endif /* GUCEF_CORE_CSTDCODECPLUGINITEM_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "CStdCodecPlugin.h"

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

typedef CTVariantMapAdapterCpp2C< CString, CString > TPluginParamMapAdapter;
typedef TPluginParamMapAdapter::TLinkedMapType TPluginParamMap;

/** The metadata parameters of the plugin as text, kept as long as the plugin is linked because it may keep them */
struct CStdCodecPlugin::SPluginParams
{
    TPluginParamMap params;
    TPluginParamMapAdapter adapter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CStdCodecPlugin::CodecPriorityParamName = "codecPriority";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CStdCodecPlugin::CStdCodecPlugin( void )
    : CICodecPlugin()           ,
      m_codecSet()              ,
      m_codecList()             ,
      m_soHandle( GUCEF_NULL )  ,
      m_pluginData( GUCEF_NULL ),
      m_metaData()              ,
      m_pluginParams( GUCEF_NULL ),
      m_apiVersion( 0 )
{GUCEF_TRACE;

    memset( &m_api, 0, sizeof( m_api ) );
    memset( &m_coreCApi, 0, sizeof( m_coreCApi ) );
}

/*-------------------------------------------------------------------------*/

CStdCodecPlugin::~CStdCodecPlugin()
{GUCEF_TRACE;

    Unlink( true );
    GUCEF_DELETE m_pluginParams;
    m_pluginParams = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TPluginMetaDataPtr
CStdCodecPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CStdCodecPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_soHandle;
}

/*-------------------------------------------------------------------------*/

const CString&
CStdCodecPlugin::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CStdCodecPlugin";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CStdCodecPlugin::GetApiVersion( void ) const
{GUCEF_TRACE;

    return m_apiVersion;
}

/*-------------------------------------------------------------------------*/

CICodecPlugin::CCodecPtr
CStdCodecPlugin::GetCodecPtr( const CICodec* codec ) const
{GUCEF_TRACE;

    CCodecSet::const_iterator i = m_codecSet.begin();
    while ( i != m_codecSet.end() )
    {
        const CCodecFamilySet& familySet = (*i).second;
        CCodecFamilySet::const_iterator n = familySet.begin();
        while ( n != familySet.end() )
        {
            if ( (*n).second.GetPointerAlways() == codec )
                return (*n).second;
            ++n;
        }
        ++i;
    }
    return CCodecPtr();
}

/*-------------------------------------------------------------------------*/

Int32
CStdCodecPlugin::GetCodecPriority( const TCodecPluginLink& codecLink ) const
{GUCEF_TRACE;

    Int32 priority = CCodecRegistry::PluginCodecPriority;
    if ( 0 != codecLink.priorityHint )
        priority = codecLink.priorityHint;

    if ( GUCEF_NULL != m_pluginParams )
    {
        TPluginParamMap::const_iterator i = m_pluginParams->params.find( CodecPriorityParamName );
        if ( i != m_pluginParams->params.end() && !(*i).second.IsNULLOrEmpty() )
            priority = StringToInt32( (*i).second, priority );
    }
    return priority;
}

/*-------------------------------------------------------------------------*/

void
CStdCodecPlugin::LinkCodecSet( void )
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return;

    CCodecRegistry& codecRegistry = CCoreGlobal::Instance()->GetCodecRegistry();
    UInt32 codecCount = m_api.GetCodecCount( m_pluginData );
    for ( UInt32 i=0; i<codecCount; ++i )
    {
        const TCodecPluginLink* pluginLink = GUCEF_NULL;
        if ( GUCEF_CODEC_STATUS_OK != m_api.GetCodecLink( m_pluginData, i, &pluginLink ) || GUCEF_NULL == pluginLink )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: The plugin gave no codec link for index " << i << " of " << codecCount );
            continue;
        }

        // Only what the plugin's struct covers is read, the rest stays absent. The functions up to decode are the minimum
        const size_t minimumLinkSize = offsetof( TCodecPluginLink, decode ) + sizeof( pluginLink->decode );
        if ( pluginLink->structSize < minimumLinkSize || GUCEF_NULL == pluginLink->codecFamily || GUCEF_NULL == pluginLink->codecType )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: Skipping the codec link at index " << i << ": it is incomplete (size " << pluginLink->structSize << ")" );
            continue;
        }
        TCodecPluginLink codecLink;
        memset( &codecLink, 0, sizeof( codecLink ) );
        memcpy( &codecLink, pluginLink, pluginLink->structSize < sizeof( codecLink ) ? pluginLink->structSize : sizeof( codecLink ) );

        CString familyName( codecLink.codecFamily );
        CString typeName( codecLink.codecType );
        CCodecPtr codecItem = CCodecPtr( GUCEF_NEW CStdCodecPluginItem( this, m_pluginData, codecLink ) );
        CCodecFamilySet& familySet = m_codecSet[ familyName ];
        familySet[ typeName ] = codecItem;
        CCodecFamilyList& familyList = m_codecList[ familyName ];
        familyList.push_back( typeName );

        // Now we register the codec in the GUCEF codec registry, next to other providers of the same type
        Int32 priority = GetCodecPriority( codecLink );
        if ( !codecRegistry.RegisterCodec( familyName, typeName, codecItem, priority ) )
        {
            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: Unable to register codec \"" << typeName <<
                "\" for codec family \"" << familyName << "\"" );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStdCodecPlugin::UnlinkCodecSet( void )
{GUCEF_TRACE;

    if ( !IsLoaded() )
        return;

    CCodecRegistry& codecRegistry = CCoreGlobal::Instance()->GetCodecRegistry();
    CCodecSet::iterator i = m_codecSet.begin();
    while ( i != m_codecSet.end() )
    {
        CCodecFamilySet& codecFamily = (*i).second;
        CCodecFamilySet::iterator n = codecFamily.begin();
        while ( n != codecFamily.end() )
        {
            // only this provider is taken out, others of the same type stay
            codecRegistry.UnregisterCodec( (*n).second );
            ++n;
        }
        ++i;
    }

    m_codecSet.clear();
    m_codecList.clear();
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::GetCodecs( CCodecSet& codecSet )
{GUCEF_TRACE;

    codecSet = m_codecSet;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::GetCodecList( CCodecList& codecList )
{GUCEF_TRACE;

    codecList = m_codecList;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Encode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        CCodecSet::iterator i = m_codecSet.find( familyName );
        if ( m_codecSet.end() != i )
        {
            CCodecFamilySet& familySet = (*i).second;
            CCodecFamilySet::iterator n = familySet.find( typeName );
            if ( familySet.end() != n )
            {
                return (*n).second->Encode( source ,
                                            dest   );
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Decode( CIOAccess& source         ,
                         CIOAccess& dest           ,
                         const CString& familyName ,
                         const CString& typeName   )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        CCodecSet::iterator i = m_codecSet.find( familyName );
        if ( m_codecSet.end() != i )
        {
            CCodecFamilySet& familySet = (*i).second;
            CCodecFamilySet::iterator n = familySet.find( typeName );
            if ( familySet.end() != n )
            {
                return (*n).second->Decode( source ,
                                            dest   );
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPlugin::GetDescription( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
        return m_api.Description( m_pluginData );
    return CString();
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPlugin::GetCopyright( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
        return m_api.Copyright( m_pluginData );
    return CString();
}

/*-------------------------------------------------------------------------*/

TVersion
CStdCodecPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
        return m_api.Version( m_pluginData );

    TVersion defaultVersion = { -1, -1, -1, -1 };
    return defaultVersion;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_soHandle != GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Link( void* modulePtr                   ,
                       TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == modulePtr )
        return false;

    CString moduleName = pluginMetaData.IsNULL() ? PointerToString( modulePtr ) : pluginMetaData->GetModuleFilename();

    // The version first: nothing else of a plugin for another interface is called, its functions may not match
    TGucefCoreCodecPluginApi api;
    memset( &api, 0, sizeof( api ) );
    api.GetApiVersion = (TCODECPLUGFPTR_GetApiVersion) GetFunctionAddress( modulePtr, "CODECPLUGIN_GetApiVersion", 0 ).funcPtr;
    if ( GUCEF_NULL == api.GetApiVersion )
    {
        // Without any of the codec exports it is simply another kind of module, which is tried as one
        if ( GUCEF_NULL != GetFunctionAddress( modulePtr, "CODECPLUGIN_Init", 3*sizeof(void*) ).funcPtr )
        {
            GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Refusing module \"" << moduleName << "\": it has no CODECPLUGIN_GetApiVersion, so it was built for a codec plugin interface older than " <<
                GUCEF_CODECPLUGIN_API_VERSION_MAJOR << "." << GUCEF_CODECPLUGIN_API_VERSION_MINOR << " which this host has" );
        }
        return false;
    }

    api.Init = (TCODECPLUGFPTR_Init) GetFunctionAddress( modulePtr, "CODECPLUGIN_Init", 3*sizeof(void*) ).funcPtr;
    api.Shutdown = (TCODECPLUGFPTR_Shutdown) GetFunctionAddress( modulePtr, "CODECPLUGIN_Shutdown", sizeof(void*) ).funcPtr;
    api.Description = (TCODECPLUGFPTR_Description) GetFunctionAddress( modulePtr, "CODECPLUGIN_Description", sizeof(void*) ).funcPtr;
    api.Copyright = (TCODECPLUGFPTR_Copyright) GetFunctionAddress( modulePtr, "CODECPLUGIN_Copyright", sizeof(void*) ).funcPtr;
    api.Version = (TCODECPLUGFPTR_Version) GetFunctionAddress( modulePtr, "CODECPLUGIN_Version", sizeof(void*) ).funcPtr;
    api.GetCodecCount = (TCODECPLUGFPTR_GetCodecCount) GetFunctionAddress( modulePtr, "CODECPLUGIN_GetCodecCount", sizeof(void*) ).funcPtr;
    api.GetCodecLink = (TCODECPLUGFPTR_GetCodecLink) GetFunctionAddress( modulePtr, "CODECPLUGIN_GetCodecLink", 2*sizeof(void*)+sizeof(UInt32) ).funcPtr;

    return LinkApi( modulePtr, api, pluginMetaData );
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::LinkApi( void* modulePtr                       ,
                          const TGucefCoreCodecPluginApi& api   ,
                          TPluginMetaDataPtr pluginMetaData     )
{GUCEF_TRACE;

    // Unloading->Loading must be done explicitly
    if ( IsLoaded() || GUCEF_NULL == modulePtr )
        return false;

    CString moduleName = pluginMetaData.IsNULL() ? PointerToString( modulePtr ) : pluginMetaData->GetModuleFilename();
    if ( GUCEF_NULL == api.GetApiVersion )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Refusing module \"" << moduleName << "\": it does not tell its codec plugin interface version" );
        return false;
    }

    UInt32 apiVersion = api.GetApiVersion();
    if ( GUCEF_CODECPLUGIN_API_VERSION_MAJOR != GUCEF_CODECPLUGIN_API_VERSION_GET_MAJOR( apiVersion ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Refusing module \"" << moduleName << "\": it was built for codec plugin interface " <<
            GUCEF_CODECPLUGIN_API_VERSION_GET_MAJOR( apiVersion ) << "." << GUCEF_CODECPLUGIN_API_VERSION_GET_MINOR( apiVersion ) <<
            ", this host has " << GUCEF_CODECPLUGIN_API_VERSION_MAJOR << "." << GUCEF_CODECPLUGIN_API_VERSION_MINOR );
        return false;
    }

    if ( GUCEF_NULL == api.Init || GUCEF_NULL == api.Shutdown || GUCEF_NULL == api.Description || GUCEF_NULL == api.Copyright ||
         GUCEF_NULL == api.Version || GUCEF_NULL == api.GetCodecCount || GUCEF_NULL == api.GetCodecLink )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Refusing module \"" << moduleName << "\": it lacks one or more of the required codec plugin exports" );
        return false;
    }

    // The metadata parameters reach the plugin as text, they stay valid until Shutdown
    GUCEF_DELETE m_pluginParams;
    m_pluginParams = GUCEF_NEW SPluginParams();
    if ( !pluginMetaData.IsNULL() )
    {
        CValueList params;
        pluginMetaData->GetParams( params );
        UInt32 keyCount = params.GetKeyCount();
        for ( UInt32 i=0; i<keyCount; ++i )
            m_pluginParams->params[ params.GetKey( i ).AsString() ] = params.GetValue( i ).AsString();
    }
    m_pluginParams->adapter.SetMapAccessPtr( (const TPluginParamMap*) &m_pluginParams->params );
    GUCEF_CORE_LinkCApi( &m_coreCApi );

    m_api = api;
    m_apiVersion = apiVersion;
    m_soHandle = modulePtr;

    // We will now try to initialize the module
    TCodecStatus status = m_api.Init( &m_pluginData, &m_coreCApi, m_pluginParams->adapter.GetCStyleAccess() );
    if ( GUCEF_CODEC_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Init of module \"" << moduleName << "\" failed with status " << (Int32) status );
        m_soHandle = GUCEF_NULL;
        m_pluginData = GUCEF_NULL;
        m_apiVersion = 0;
        memset( &m_api, 0, sizeof( m_api ) );
        return false;
    }

    // We have loaded & linked our plugin module
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Successfully loaded module \"" << moduleName << "\" (codec plugin interface " <<
        GUCEF_CODECPLUGIN_API_VERSION_GET_MAJOR( apiVersion ) << "." << GUCEF_CODECPLUGIN_API_VERSION_GET_MINOR( apiVersion ) << ") and invoked Init()" );

    // Copy the given metadata and update it with info from the actual module
    if ( !pluginMetaData.IsNULL() )
        m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );
    else
        m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData() );
    m_metaData->SetDescription( GetDescription() );
    m_metaData->SetCopyright( GetCopyright() );
    m_metaData->SetVersion( GetVersion() );

    // The module and been successfully loaded and linked
    // we will now generate a list of codec's
    LinkCodecSet();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Unlink( void )
{GUCEF_TRACE;

    return Unlink( false );
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Unlink( bool forceEvenIfInUse )
{GUCEF_TRACE;

    // Check if we have something to unload
    if ( m_soHandle == GUCEF_NULL ) return true;

    if ( !forceEvenIfInUse )
    {
        // Check for outstanding references to our codec's: anything beyond our own and the registry's means
        // the code of the module is still in use (a codec or an instance of it)
        CCodecRegistry& codecRegistry = CCoreGlobal::Instance()->GetCodecRegistry();
        CCodecSet::iterator i = m_codecSet.begin();
        while ( i != m_codecSet.end() )
        {
            CCodecFamilySet& codecFamily = (*i).second;
            CCodecFamilySet::iterator n = codecFamily.begin();
            while ( n != codecFamily.end() )
            {
                UInt32 heldByRegistry = codecRegistry.GetHeldReferenceCount( (*n).second.GetPointerAlways() );
                if ( (*n).second.GetReferenceCount() > 1 + heldByRegistry )
                {
                    // Cannot unload the module if someone is still using one of it's codec's
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Unable to unlink codec module since there are outstanding references to the codecs from module: " << PointerToString( m_soHandle ) );
                    return false;
                }
                ++n;
            }
            ++i;
        }
    }

    // Free all codec links
    UnlinkCodecSet();

    // We will now try to shut the module down
    TCodecStatus status = m_api.Shutdown( m_pluginData );
    if ( GUCEF_CODEC_STATUS_OK != status )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Shutdown of module " << PointerToString( m_soHandle ) << " failed with status " << (Int32) status );
        return false;
    }

    // Module shutdown complete + no outstanding references,.. unload module
    memset( &m_api, 0, sizeof( m_api ) );
    m_soHandle = GUCEF_NULL;
    m_pluginData = GUCEF_NULL;
    m_apiVersion = 0;
    GUCEF_DELETE m_pluginParams;
    m_pluginParams = GUCEF_NULL;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
