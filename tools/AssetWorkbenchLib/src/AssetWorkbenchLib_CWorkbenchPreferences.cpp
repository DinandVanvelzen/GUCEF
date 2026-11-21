/*
 *  AssetWorkbenchLib: the preferences of the owner that the settings window keeps
 *
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

#include "AssetWorkbenchLib_CWorkbenchPreferences.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWorkbenchPreferences::PrivatePluginsDirName = "PrivatePluginsDir";
const CORE::CString CWorkbenchPreferences::DisabledStoresName = "DisabledStores";
const CORE::CString CWorkbenchPreferences::ShortcutsName = "Shortcuts";
const CORE::CString CWorkbenchPreferences::SettingsVersionName = "SettingsVersion";
const CORE::CString CWorkbenchPreferences::OpenPackPresentationName = "OpenPackPresentation";
const CORE::CString CWorkbenchPreferences::OpenPacksInResourceViewName = "OpenPacksInResourceView";
const CORE::CString CWorkbenchPreferences::ResourceViewShownName = "ResourceViewShown";
const CORE::CString CWorkbenchPreferences::AutomaticThumbnailsName = "AutomaticThumbnails";
const CORE::CString CWorkbenchPreferences::PreferCookedOutputName = "PreferCookedOutput";
const CORE::CString CWorkbenchPreferences::PackPresentationsName = "PackPresentations";
const CORE::CString CWorkbenchPreferences::BulkTemplatesName = "BulkTemplates";
const CORE::CString CWorkbenchPreferences::MaxSceneSecondsName = "MaxSceneSeconds";
const CORE::CString CWorkbenchPreferences::RemoteEndpointName = "RemoteEndpoint";
const CORE::CString CWorkbenchPreferences::RemotePairingPortName = "RemotePairingPort";
const CORE::CString CWorkbenchPreferences::RemoteLanDiscoveryName = "RemoteLanDiscovery";
const CORE::CString CWorkbenchPreferences::RemoteConnectAtStartName = "RemoteConnectAtStart";
const CORE::CString CWorkbenchPreferences::DefaultRemoteEndpoint = "worker.lan:47400";
const UInt32 CWorkbenchPreferences::DefaultRemotePairingPort;
const UInt32 CWorkbenchPreferences::DefaultMaxSceneSeconds;

/*-------------------------------------------------------------------------*/

CWorkbenchPreferences::CWorkbenchPreferences( void )
    : settingsVersion( 0 )
    , privatePluginsDir()
    , disabledStores()
    , shortcuts()
    , openPackPresentation( true )
    , maxSceneSeconds( DefaultMaxSceneSeconds )
    , automaticThumbnails( true )
    , preferCookedOutput( true )
    , openPacksInResourceView( true )
    , resourceViewShown( false )
    , remoteEndpoint( DefaultRemoteEndpoint )
    , remotePairingPort( DefaultRemotePairingPort )
    , remoteLanDiscovery( true )
    , remoteConnectAtStart( false )
    , packPresentations()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchPreferences::GetPackPresentation( const CORE::CString& packKey, CORE::CString& presentationId ) const
{GUCEF_TRACE;

    const CORE::CString prefix = packKey + "=";
    for ( size_t i=0; i<packPresentations.size(); ++i )
    {
        if ( 0 == packPresentations[ i ].HasSubstr( prefix ) )
        {
            presentationId = packPresentations[ i ].SubstrFromRange( prefix.Length(), packPresentations[ i ].Length() );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::ClearPackPresentation( const CORE::CString& packKey )
{GUCEF_TRACE;

    const CORE::CString prefix = packKey + "=";
    CORE::CString::StringVector kept;
    for ( size_t i=0; i<packPresentations.size(); ++i )
        if ( 0 != packPresentations[ i ].HasSubstr( prefix ) )
            kept.push_back( packPresentations[ i ] );
    packPresentations = kept;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::SetPackPresentation( const CORE::CString& packKey, const CORE::CString& presentationId )
{GUCEF_TRACE;

    ClearPackPresentation( packKey );
    packPresentations.push_back( packKey + "=" + presentationId );
    // the list does not grow without bound: the choices of the oldest packs go first
    while ( packPresentations.size() > MaxPackPresentations )
        packPresentations.erase( packPresentations.begin() );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchPreferences::IsStoreEnabled( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<disabledStores.size(); ++i )
        if ( disabledStores[ i ] == storeId )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::SetStoreEnabled( const CORE::CString& storeId, bool enabled )
{GUCEF_TRACE;

    CORE::CString::StringVector kept;
    for ( size_t i=0; i<disabledStores.size(); ++i )
        if ( disabledStores[ i ] != storeId )
            kept.push_back( disabledStores[ i ] );
    if ( !enabled )
        kept.push_back( storeId );
    disabledStores = kept;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::CaptureShortcuts( const GUI::CKeyboardShortcutMap& map, const CORE::CString::StringVector& leaveOut )
{GUCEF_TRACE;

    shortcuts.clear();
    CORE::CString::StringVector ids;
    map.GetActionIds( ids );
    for ( size_t i=0; i<ids.size(); ++i )
    {
        bool skip = false;
        for ( size_t l=0; l<leaveOut.size(); ++l )
            skip = skip || leaveOut[ l ] == ids[ i ];
        INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
        UInt32 modifiers = 0;
        if ( !skip && map.GetChord( ids[ i ], key, modifiers ) )
            shortcuts.push_back( ids[ i ] + "=" + GUI::CKeyboardShortcutMap::DescribeChord( key, modifiers ) );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchPreferences::ApplyShortcuts( GUI::CKeyboardShortcutMap& map ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "Shortcuts" );
    for ( size_t i=0; i<shortcuts.size(); ++i )
    {
        const Int32 equals = shortcuts[ i ].HasChar( '=' );
        if ( equals > 0 )
            node.SetAttribute( shortcuts[ i ].SubstrToIndex( (UInt32) equals, true ), shortcuts[ i ].SubstrToIndex( (UInt32) equals + 1, false ) );
    }
    return map.LoadBindings( node );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchPreferences::GetBulkTemplate( const CORE::CString& name, CBulkConversionSettings& settings ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<bulkTemplates.size(); ++i )
    {
        if ( bulkTemplates[ i ].name == name )
        {
            settings = bulkTemplates[ i ];
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchPreferences::SetBulkTemplate( const CBulkConversionSettings& settings )
{GUCEF_TRACE;

    if ( settings.name.IsNULLOrEmpty() )
        return false;
    for ( size_t i=0; i<bulkTemplates.size(); ++i )
    {
        if ( bulkTemplates[ i ].name == settings.name )
        {
            bulkTemplates[ i ] = settings;
            return true;
        }
    }
    if ( bulkTemplates.size() >= (size_t) MaxBulkTemplates )
        return false;
    bulkTemplates.push_back( settings );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchPreferences::RemoveBulkTemplate( const CORE::CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<bulkTemplates.size(); ++i )
    {
        if ( bulkTemplates[ i ].name == name )
        {
            bulkTemplates.erase( bulkTemplates.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::GetBulkTemplateNames( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    for ( size_t i=0; i<bulkTemplates.size(); ++i )
        names.push_back( bulkTemplates[ i ].name );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::Save( CORE::CDataNode& section ) const
{GUCEF_TRACE;

    section.SetAttribute( SettingsVersionName, settingsVersion );
    section.SetAttribute( PrivatePluginsDirName, privatePluginsDir );
    section.DelChild( DisabledStoresName );
    section.SetValuesOfChildByName( DisabledStoresName, disabledStores );
    section.DelChild( ShortcutsName );
    section.SetValuesOfChildByName( ShortcutsName, shortcuts );
    section.SetAttribute( OpenPackPresentationName, openPackPresentation );
    section.SetAttribute( OpenPacksInResourceViewName, openPacksInResourceView );
    section.SetAttribute( ResourceViewShownName, resourceViewShown );
    section.SetAttribute( AutomaticThumbnailsName, automaticThumbnails );
    section.SetAttribute( PreferCookedOutputName, preferCookedOutput );
    section.SetAttribute( MaxSceneSecondsName, maxSceneSeconds );
    section.SetAttribute( RemoteEndpointName, remoteEndpoint );
    section.SetAttribute( RemotePairingPortName, remotePairingPort );
    section.SetAttribute( RemoteLanDiscoveryName, remoteLanDiscovery );
    section.SetAttribute( RemoteConnectAtStartName, remoteConnectAtStart );
    section.DelChild( PackPresentationsName );
    section.SetValuesOfChildByName( PackPresentationsName, packPresentations );
    section.DelChild( BulkTemplatesName );
    CORE::CDataNode* templates = section.AddChild( BulkTemplatesName, GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<bulkTemplates.size(); ++i )
        bulkTemplates[ i ].Serialize( *templates->AddChild( "Template", GUCEF_DATATYPE_OBJECT ), CORE::CDataNodeSerializableSettings() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchPreferences::Load( const CORE::CDataNode& section )
{GUCEF_TRACE;

    settingsVersion = section.GetAttributeValueOrChildValueByName( SettingsVersionName ).AsUInt32( 0, true );
    privatePluginsDir = section.GetAttributeValueOrChildValueByName( PrivatePluginsDirName ).AsString( CORE::CString::Empty, true );
    disabledStores.clear();
    section.GetValuesOfChildByName( DisabledStoresName, disabledStores );
    shortcuts.clear();
    section.GetValuesOfChildByName( ShortcutsName, shortcuts );
    openPackPresentation = section.GetAttributeValueOrChildValueByName( OpenPackPresentationName ).AsBool( true, true );
    openPacksInResourceView = section.GetAttributeValueOrChildValueByName( OpenPacksInResourceViewName ).AsBool( true, true );
    resourceViewShown = section.GetAttributeValueOrChildValueByName( ResourceViewShownName ).AsBool( false, true );
    automaticThumbnails = section.GetAttributeValueOrChildValueByName( AutomaticThumbnailsName ).AsBool( true, true );
    preferCookedOutput = section.GetAttributeValueOrChildValueByName( PreferCookedOutputName ).AsBool( true, true );
    maxSceneSeconds = section.GetAttributeValueOrChildValueByName( MaxSceneSecondsName ).AsUInt32( DefaultMaxSceneSeconds, true );
    remoteEndpoint = section.GetAttributeValueOrChildValueByName( RemoteEndpointName ).AsString( DefaultRemoteEndpoint, true );
    remotePairingPort = section.GetAttributeValueOrChildValueByName( RemotePairingPortName ).AsUInt32( DefaultRemotePairingPort, true );
    if ( 0 == remotePairingPort || remotePairingPort > 65535 )
        remotePairingPort = DefaultRemotePairingPort;
    remoteLanDiscovery = section.GetAttributeValueOrChildValueByName( RemoteLanDiscoveryName ).AsBool( true, true );
    remoteConnectAtStart = section.GetAttributeValueOrChildValueByName( RemoteConnectAtStartName ).AsBool( false, true );
    if ( 0 == maxSceneSeconds )
        maxSceneSeconds = DefaultMaxSceneSeconds;
    packPresentations.clear();
    section.GetValuesOfChildByName( PackPresentationsName, packPresentations );
    bulkTemplates.clear();
    const CORE::CDataNode* templates = section.FindChild( BulkTemplatesName );
    if ( GUCEF_NULL != templates )
    {
        CORE::CDataNode::const_iterator t = templates->ConstBegin();
        for ( ; t != templates->ConstEnd() && bulkTemplates.size() < (size_t) MaxBulkTemplates; ++t )
        {
            CBulkConversionSettings loaded;
            if ( loaded.Deserialize( *(*t), CORE::CDataNodeSerializableSettings() ) && !loaded.name.IsNULLOrEmpty() )
                bulkTemplates.push_back( loaded );
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
