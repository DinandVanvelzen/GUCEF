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
#include <assert.h>

#ifndef GUCEF_CORE_CODECPLUGINLINK_H
#include "CodecPluginLink.h"
#define GUCEF_CORE_CODECPLUGINLINK_H
#endif /* GUCEF_CORE_CODECPLUGINLINK_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

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

enum
{
        STDCODEC_INIT       = 0         ,
        STDCODEC_SHUTDOWN               ,
        STDCODEC_COPYRIGHT              ,
        STDCODEC_VERSION                ,
        STDCODEC_DESCRIPTION            ,

        STDCODEC_CODECSETBEGIN          ,
        STDCODEC_CODECLINK              ,
        STDCODEC_FREECODECLINK          ,
        STDCODEC_FREECODECITERATOR      ,
        STDCODEC_CODECSETNEXT           ,

        STDCODEC_FUNCTIONTABLESIZE
};

/*-------------------------------------------------------------------------*/

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Init ) ( void** plugdata, const int argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Shutdown ) ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Copyright ) ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef TVersion ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Version ) ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef const char* ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_Description ) ( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetCodecSetBegin ) ( void* plugdata, void** iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetCodecLink ) ( void* plugdata, void* iterator, TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_FreeCodecLink ) ( void* plugdata, TCodecPluginLink* codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_FreeCodecIterator ) ( void* plugdata, void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef UInt32 ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TCODECPLUGFPTR_GetCodecSetNextItem ) ( void* plugdata, void* iterator ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CStdCodecPlugin::CStdCodecPlugin( void )
    : CICodecPlugin()    ,
      m_soHandle( NULL ) ,
      m_pluginData()     ,
      m_codecSet()       ,
      m_codecList()
{GUCEF_TRACE;

    memset( m_fpTable, 0, STDCODEC_FUNCTIONTABLESIZE );
}

/*-------------------------------------------------------------------------*/

CStdCodecPlugin::~CStdCodecPlugin()
{GUCEF_TRACE;

    Unlink( true );
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

void
CStdCodecPlugin::LinkCodecSet( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        CCodecRegistry& codecRegistry = CCoreGlobal::Instance()->GetCodecRegistry();
        void* iterator = NULL;
        if ( ( (TCODECPLUGFPTR_GetCodecSetBegin) m_fpTable[ STDCODEC_CODECSETBEGIN ] )( m_pluginData, &iterator ) != 0 )
        {
            do
            {
                TCodecPluginLink* codecLink = NULL;
                if ( ( (TCODECPLUGFPTR_GetCodecLink) m_fpTable[ STDCODEC_CODECLINK ] )( m_pluginData, iterator, &codecLink ) != 0 )
                {
                    assert( codecLink != NULL );
                    CCodecPtr codecItem = CCodecPtr( GUCEF_NEW CStdCodecPluginItem( m_pluginData, codecLink ) );
                    CCodecFamilySet& familySet = m_codecSet[ codecLink->codecFamily ];
                    familySet.insert( std::pair< CString, CCodecPtr >( codecLink->codecType, codecItem ) );
                    CCodecFamilyList& familyList = m_codecList[ codecLink->codecFamily ];
                    familyList.push_back( codecLink->codecType );

                    // Now we register the codecs from this plugin in the GUCEF codec registry
                    // First we check if we have to add a new registry for this family
                    CCodecRegistry::TCodecFamilyRegistryPtr familyRegistry;
                    if ( !codecRegistry.TryLookup( codecLink->codecFamily ,
                                                   familyRegistry         ,
                                                   true                   ) )
                    {
                        familyRegistry = CCodecRegistry::TCodecFamilyRegistryTypedPtr( GUCEF_NEW CCodecRegistry::TCodecFamilyRegistry() );
                        if ( !codecRegistry.TryRegister( codecLink->codecFamily, familyRegistry ) )
                        {
                            // race condition?
                            if ( !codecRegistry.TryLookup( codecLink->codecFamily ,
                                                           familyRegistry         ,
                                                           true                   ) )
                            {
                                GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: Unable to register new codec family \"" + 
                                    CString( codecLink->codecFamily ) + "\"" );
                            }
                        }
                    }

                    if ( familyRegistry )
                    {
                        // we now have access to a registry for this codec family
                        // We will add this codec if there no codec already registered with such a name
                        if ( familyRegistry->TryRegister( codecLink->codecType, codecItem ) )
                        {
                            GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: Registered new codec \"" + CString( codecLink->codecType ) + 
                                    "\" for codec family \"" + CString( codecLink->codecFamily ) + "\"" );
                        }
                        else
                        {
                            GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin:LinkCodecSet: Unable to register new codec \"" + CString( codecLink->codecType ) + 
                                    "\" for codec family \"" + CString( codecLink->codecFamily ) + "\"" );
                        }
                    }
                }
            }
            while ( ( (TCODECPLUGFPTR_GetCodecSetNextItem) m_fpTable[ STDCODEC_CODECSETNEXT ] )( m_pluginData, iterator ) != 0 );

            ( (TCODECPLUGFPTR_FreeCodecIterator) m_fpTable[ STDCODEC_FREECODECITERATOR ] )( m_pluginData, iterator );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CStdCodecPlugin::UnlinkCodecSet( void )
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        CCodecRegistry& codecRegistry = CCoreGlobal::Instance()->GetCodecRegistry();
        CStdCodecPluginItem* pluginItem = NULL;
        CCodecSet::iterator i = m_codecSet.begin();
        while ( i != m_codecSet.end() )
        {
            CCodecFamilySet& codecFamily = (*i).second;
            CCodecFamilySet::iterator n = codecFamily.begin();
            while ( n != codecFamily.end() )
            {
                pluginItem = static_cast< CStdCodecPluginItem* >( &( *( (*n).second ) ) );
                TCodecPluginLink* codecLink = pluginItem->GetCodecPluginLink();

                // Now we unregister the codecs from this plugin from the GUCEF codec registry
                // First we check if family registry still exists
                if ( codecRegistry.IsRegistered( codecLink->codecFamily ) )
                {
                    // we now have access to a registry for this codec family
                    // We will remove the codec if registered
                    CCodecRegistry::TCodecFamilyRegistryPtr familyRegistry = codecRegistry.Lookup( codecLink->codecFamily );
                    familyRegistry->TryUnregister( codecLink->codecType );
                }

                ( (TCODECPLUGFPTR_FreeCodecLink) m_fpTable[ STDCODEC_FREECODECLINK ] )( pluginItem->GetPluginData()      ,
                                                                                        pluginItem->GetCodecPluginLink() );
                ++n;
            }
            ++i;
        }

        m_codecSet.clear();
        m_codecList.clear();
    }
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
        return false;
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
        return false;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPlugin::GetDescription( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        return ( (TCODECPLUGFPTR_Description) m_fpTable[ STDCODEC_DESCRIPTION ] )( m_pluginData );
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPlugin::GetCopyright( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        return ( (TCODECPLUGFPTR_Copyright) m_fpTable[ STDCODEC_COPYRIGHT ] )( m_pluginData );
    }

    return CString();
}

/*-------------------------------------------------------------------------*/

TVersion
CStdCodecPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    if ( IsLoaded() )
    {
        return ( (TCODECPLUGFPTR_Version) m_fpTable[ STDCODEC_VERSION ] )( m_pluginData );
    }

    TVersion defaultVersion = { -1, -1, -1, -1 };
    return defaultVersion;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::IsLoaded( void ) const
{GUCEF_TRACE;

    return m_soHandle != NULL;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPlugin::Link( void* modulePtr                   ,
                       TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    // make sure no module is already loaded
    if ( modulePtr == NULL ) return false;

    // let's see if it is a codec plugin
    m_fpTable[ STDCODEC_DESCRIPTION ] = GetFunctionAddress( modulePtr                 ,
                                                            "CODECPLUGIN_Description" ,
                                                            sizeof(void*)             ).funcPtr;
    m_fpTable[ STDCODEC_COPYRIGHT ] = GetFunctionAddress( modulePtr               ,
                                                          "CODECPLUGIN_Copyright" ,
                                                          sizeof(void*)           ).funcPtr;
    m_fpTable[ STDCODEC_VERSION ] = GetFunctionAddress( modulePtr             ,
                                                        "CODECPLUGIN_Version" ,
                                                        sizeof(void*)         ).funcPtr;
    m_fpTable[ STDCODEC_INIT ] = GetFunctionAddress( modulePtr                       ,
                                                     "CODECPLUGIN_Init"              ,
                                                     2*sizeof(void*)+sizeof(char***) ).funcPtr;
    m_fpTable[ STDCODEC_SHUTDOWN ] = GetFunctionAddress( modulePtr              ,
                                                         "CODECPLUGIN_Shutdown" ,
                                                         sizeof(void*)          ).funcPtr;
    m_fpTable[ STDCODEC_CODECSETBEGIN ] = GetFunctionAddress( modulePtr                      ,
                                                              "CODECPLUGIN_GetCodecSetBegin" ,
                                                              sizeof(void*)+sizeof(void**)   ).funcPtr;
    m_fpTable[ STDCODEC_CODECLINK ] = GetFunctionAddress( modulePtr                                  ,
                                                          "CODECPLUGIN_GetCodecLink"                 ,
                                                          2*sizeof(void*)+sizeof(TCodecPluginLink**) ).funcPtr;
    m_fpTable[ STDCODEC_FREECODECLINK ] = GetFunctionAddress( modulePtr                               ,
                                                              "CODECPLUGIN_FreeCodecLink"             ,
                                                              sizeof(void*)+sizeof(TCodecPluginLink*) ).funcPtr;
    m_fpTable[ STDCODEC_FREECODECITERATOR ] = GetFunctionAddress( modulePtr                       ,
                                                                  "CODECPLUGIN_FreeCodecIterator" ,
                                                                  2*sizeof(void*)                 ).funcPtr;
    m_fpTable[ STDCODEC_CODECSETNEXT ] = GetFunctionAddress( modulePtr                         ,
                                                             "CODECPLUGIN_GetCodecSetNextItem" ,
                                                             2*sizeof(void*)                   ).funcPtr;

    // Verify that we have obtained a function address for each of the functions
    if ( ( !m_fpTable[ STDCODEC_INIT ] )              ||
         ( !m_fpTable[ STDCODEC_SHUTDOWN ] )          ||
         ( !m_fpTable[ STDCODEC_DESCRIPTION ] )       ||
         ( !m_fpTable[ STDCODEC_COPYRIGHT ] )         ||
         ( !m_fpTable[ STDCODEC_VERSION ] )           ||
         ( !m_fpTable[ STDCODEC_CODECSETBEGIN ] )     ||
         ( !m_fpTable[ STDCODEC_CODECLINK ] )         ||
         ( !m_fpTable[ STDCODEC_FREECODECLINK ] )     ||
         ( !m_fpTable[ STDCODEC_FREECODECITERATOR ] ) ||
         ( !m_fpTable[ STDCODEC_CODECSETNEXT ] )       )
    {
            m_soHandle = NULL;
            memset( m_fpTable, 0, STDCODEC_FUNCTIONTABLESIZE );
            return false;
    }

    m_soHandle = modulePtr;

    // We will now try to initialize the module
    if ( ( (TCODECPLUGFPTR_Init) m_fpTable[ STDCODEC_INIT ] )( &m_pluginData, 0, NULL ) == 0 )
    {
        m_soHandle = NULL;
        memset( m_fpTable, 0, STDCODEC_FUNCTIONTABLESIZE );
        return false;
    }

    // We have loaded & linked our plugin module
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Successfully loaded module and invoked Init() using module: " + PointerToString( modulePtr ) );

    // Copy the given metadata and update it with info from the actual module
    m_metaData = TPluginMetaDataStoragePtr( GUCEF_NEW CPluginMetaData( *pluginMetaData ) );
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
    if ( m_soHandle == NULL ) return true;

    if ( !forceEvenIfInUse )
    {
        // Check for outstanding references to our codec's
        CCodecSet::iterator i = m_codecSet.begin();
        while ( i != m_codecSet.end() )
        {
            CCodecFamilySet& codecFamily = (*i).second;
            CCodecFamilySet::iterator n = codecFamily.begin();
            while ( n != codecFamily.end() )
            {
                // Why a reference check on 2: 
                //      We know this class has a reference to the codec linking it into the plugin, thats 1
                //      When codecs are successfully loaded they are registered in the global codec registry for their
                //      respective families, thats 2.
                //      Anyone using a codec would need to have an additional reference outstanding thus > 2
                //      When unloading a plugin you want to always make sure nobody is using the functionality/code
                //      from that plugin because any reference to it would become invalid leading to crashes.
                if ( (*n).second.GetReferenceCount() > 2 )
                {
                    // Cannot unload the module if someone is still using one of it's codec's
                    GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "StdCodecPlugin: Unable to unlink codec module since there are outstanding references to the codecs from module: " + PointerToString( m_soHandle ) );
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
    if ( ( (TCODECPLUGFPTR_Shutdown) m_fpTable[ STDCODEC_SHUTDOWN ] )( m_pluginData ) == 0 )
    {
        return false;
    }

    // Module shutdown complete + no outstanding references,.. unload module
    memset( m_fpTable, 0, STDCODEC_FUNCTIONTABLESIZE );
    m_soHandle = NULL;
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
