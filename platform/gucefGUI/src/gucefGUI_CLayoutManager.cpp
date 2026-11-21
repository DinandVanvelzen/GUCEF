/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CSPLITTER_H
#include "gucefGUI_CSplitter.h"
#define GUCEF_GUI_CSPLITTER_H
#endif /* GUCEF_GUI_CSPLITTER_H ? */

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#include "gucefGUI_CFoldablePanel.h"
#define GUCEF_GUI_CFOLDABLEPANEL_H
#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */

#include "gucefGUI_CLayoutManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CLayoutManager::LayoutChangedEvent = "GUCEF::GUI::CLayoutManager::LayoutChangedEvent";
const CORE::CEvent CLayoutManager::PresetAppliedEvent = "GUCEF::GUI::CLayoutManager::PresetAppliedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLayoutManager::RegisterEvents( void )
{GUCEF_TRACE;

    LayoutChangedEvent.Initialize();
    PresetAppliedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLayoutManager::CLayoutManager( void )
    : CORE::CObservingNotifier()
    , m_root( GUCEF_NULL )
    , m_watched()
    , m_presets()
    , m_activePreset()
    , m_loaded()
    , m_loadedActive()
    , m_hasLoaded( false )
    , m_applying( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLayoutManager::~CLayoutManager()
{GUCEF_TRACE;

    Detach();
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::Watch( CWidget& widget )
{GUCEF_TRACE;

    if ( widget.IsRegisteredAs( "Splitter" ) && m_watched.insert( &widget ).second )
    {
        TEventCallback callback( this, &CLayoutManager::OnLayoutWidgetChanged );
        SubscribeTo( &widget, CSplitter::SizeChangedEvent, callback );
    }
    else
    if ( widget.IsRegisteredAs( "FoldablePanel" ) && m_watched.insert( &widget ).second )
    {
        TEventCallback callback( this, &CLayoutManager::OnLayoutWidgetChanged );
        SubscribeTo( &widget, CFoldablePanel::FoldedChangedEvent, callback );
    }
    const CWidget::TWidgetVector children = widget.GetChildren();
    for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
        Watch( *children[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::Attach( CWidget* root )
{GUCEF_TRACE;

    Detach();
    if ( GUCEF_NULL == root )
        return false;
    m_root = root;
    Watch( *m_root );
    if ( m_hasLoaded )
    {
        // the layout of the last session
        ApplyState( m_loaded, m_loadedActive );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::Detach( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    m_watched.clear();
    m_root = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::Rescan( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_root )
        Watch( *m_root );
}

/*-------------------------------------------------------------------------*/

CWidget*
CLayoutManager::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutManager::GetWatchedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_watched.size();
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::CaptureState( CLayoutState& state ) const
{GUCEF_TRACE;

    state.Clear();
    if ( GUCEF_NULL != m_root )
        state.Capture( *m_root );
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutManager::ApplyState( const CLayoutState& state, const CString& presetName )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_root )
        return 0;
    // the changes the state makes are not changes of the user
    const bool wasApplying = m_applying;
    m_applying = true;
    const UInt32 applied = state.Apply( *m_root );
    m_applying = wasApplying;
    m_activePreset = presetName;
    return applied;
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::AddPreset( const CString& name, const CLayoutState& state, bool builtIn )
{GUCEF_TRACE;

    SPreset& preset = m_presets[ name ];
    preset.state = state;
    preset.builtIn = builtIn;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::AddPresetFromCurrent( const CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || GUCEF_NULL == m_root )
        return false;
    CLayoutState state;
    CaptureState( state );
    AddPreset( name, state, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::RemovePreset( const CString& name )
{GUCEF_TRACE;

    return m_presets.erase( name ) > 0;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::HasPreset( const CString& name ) const
{GUCEF_TRACE;

    return m_presets.find( name ) != m_presets.end();
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::GetPreset( const CString& name, CLayoutState& state ) const
{GUCEF_TRACE;

    TPresetMap::const_iterator i = m_presets.find( name );
    if ( i == m_presets.end() )
        return false;
    state = i->second.state;
    return true;
}

/*-------------------------------------------------------------------------*/

CLayoutManager::TStringVector
CLayoutManager::GetPresetNames( void ) const
{GUCEF_TRACE;

    TStringVector names;
    for ( TPresetMap::const_iterator i=m_presets.begin(); i!=m_presets.end(); ++i )
        names.push_back( i->first );
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::ApplyPreset( const CString& name )
{GUCEF_TRACE;

    TPresetMap::const_iterator i = m_presets.find( name );
    if ( i == m_presets.end() || GUCEF_NULL == m_root )
        return false;
    ApplyState( i->second.state, name );
    CORE::TCloneableString applied( name );
    NotifyObservers( PresetAppliedEvent, &applied );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CLayoutManager::GetActivePreset( void ) const
{GUCEF_TRACE;

    return m_activePreset;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::Save( CORE::CDataNode& section, const CString& name ) const
{GUCEF_TRACE;

    section.DelChild( name );
    CORE::CDataNode* node = section.AddChild( name );
    if ( GUCEF_NULL == node )
        return false;
    node->SetAttribute( "active", m_activePreset );

    CLayoutState current;
    CaptureState( current );
    if ( current.IsEmpty() && m_hasLoaded )
        current = m_loaded;
    CORE::CDataNode* currentNode = node->AddChild( "Current" );
    if ( GUCEF_NULL == currentNode || !current.Save( *currentNode ) )
        return false;

    for ( TPresetMap::const_iterator i=m_presets.begin(); i!=m_presets.end(); ++i )
    {
        if ( i->second.builtIn )
            continue;
        CORE::CDataNode* presetNode = node->AddChild( "Preset" );
        if ( GUCEF_NULL == presetNode )
            return false;
        presetNode->SetAttribute( "name", i->first );
        if ( !i->second.state.Save( *presetNode ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutManager::Load( const CORE::CDataNode& section, const CString& name )
{GUCEF_TRACE;

    const CORE::CDataNode* node = section.FindChild( name );
    if ( GUCEF_NULL == node )
        return false;

    // the presets of the user, whatever else is in the node is not ours to judge
    CORE::CDataNode::const_iterator i = node->ConstBegin();
    while ( i != node->ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() != "Preset" )
            continue;
        const CORE::CDataNode::TKeyValuePair* nameAttribute = child->GetAttribute( "name" );
        if ( GUCEF_NULL == nameAttribute )
            continue;
        const CString presetName = nameAttribute->second.AsString( CString::Empty, true );
        CLayoutState state;
        if ( !presetName.IsNULLOrEmpty() && state.Load( *child ) )
            AddPreset( presetName, state, false );
    }

    const CORE::CDataNode* currentNode = node->FindChild( "Current" );
    m_loaded.Clear();
    m_hasLoaded = GUCEF_NULL != currentNode && m_loaded.Load( *currentNode );
    const CORE::CDataNode::TKeyValuePair* activeAttribute = node->GetAttribute( "active" );
    m_loadedActive = GUCEF_NULL != activeAttribute ? activeAttribute->second.AsString( CString::Empty, true ) : CString::Empty;
    if ( !m_loadedActive.IsNULLOrEmpty() && !HasPreset( m_loadedActive ) )
        m_loadedActive.Clear();
    if ( m_hasLoaded && GUCEF_NULL != m_root )
        ApplyState( m_loaded, m_loadedActive );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLayoutManager::OnLayoutWidgetChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_applying )
        return;
    // by hand: no preset is on any more
    m_activePreset.Clear();
    NotifyObservers( LayoutChangedEvent );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
