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

#ifndef GUCEF_GUI_CSPLITTER_H
#include "gucefGUI_CSplitter.h"
#define GUCEF_GUI_CSPLITTER_H
#endif /* GUCEF_GUI_CSPLITTER_H ? */

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#include "gucefGUI_CFoldablePanel.h"
#define GUCEF_GUI_CFOLDABLEPANEL_H
#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */

#include "gucefGUI_CLayoutState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const SplitterNodeName = "Splitter";
const char* const PanelNodeName = "Panel";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutState::CLayoutState( void )
    : m_ratios()
    , m_folded()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayoutState::CLayoutState( const CLayoutState& src )
    : m_ratios( src.m_ratios )
    , m_folded( src.m_folded )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayoutState&
CLayoutState::operator=( const CLayoutState& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_ratios = src.m_ratios;
        m_folded = src.m_folded;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CLayoutState::~CLayoutState()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::operator==( const CLayoutState& other ) const
{GUCEF_TRACE;

    return m_ratios == other.m_ratios && m_folded == other.m_folded;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::operator!=( const CLayoutState& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

void
CLayoutState::Clear( void )
{GUCEF_TRACE;

    m_ratios.clear();
    m_folded.clear();
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_ratios.empty() && m_folded.empty();
}

/*-------------------------------------------------------------------------*/

void
CLayoutState::SetSplitRatio( const CString& splitterName, Float32 ratio )
{GUCEF_TRACE;

    if ( ratio < 0.0f ) ratio = 0.0f;
    if ( ratio > 1.0f ) ratio = 1.0f;
    m_ratios[ splitterName ] = ratio;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::TryGetSplitRatio( const CString& splitterName, Float32& ratio ) const
{GUCEF_TRACE;

    TRatioMap::const_iterator i = m_ratios.find( splitterName );
    if ( i == m_ratios.end() )
        return false;
    ratio = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::RemoveSplitRatio( const CString& splitterName )
{GUCEF_TRACE;

    return m_ratios.erase( splitterName ) > 0;
}

/*-------------------------------------------------------------------------*/

void
CLayoutState::SetFolded( const CString& panelName, bool folded )
{GUCEF_TRACE;

    m_folded[ panelName ] = folded;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::TryGetFolded( const CString& panelName, bool& folded ) const
{GUCEF_TRACE;

    TFoldedMap::const_iterator i = m_folded.find( panelName );
    if ( i == m_folded.end() )
        return false;
    folded = i->second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::RemoveFolded( const CString& panelName )
{GUCEF_TRACE;

    return m_folded.erase( panelName ) > 0;
}

/*-------------------------------------------------------------------------*/

const CLayoutState::TRatioMap&
CLayoutState::GetSplitRatios( void ) const
{GUCEF_TRACE;

    return m_ratios;
}

/*-------------------------------------------------------------------------*/

const CLayoutState::TFoldedMap&
CLayoutState::GetFolded( void ) const
{GUCEF_TRACE;

    return m_folded;
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutState::Capture( const CWidget& root )
{GUCEF_TRACE;

    Clear();
    const CSplitter* splitter = CWidget::As< CSplitter >( &root );
    if ( GUCEF_NULL != splitter && !root.GetName().IsNULLOrEmpty() )
        m_ratios[ root.GetName() ] = splitter->GetSplitRatio();
    const CFoldablePanel* panel = CWidget::As< CFoldablePanel >( &root );
    if ( GUCEF_NULL != panel && !root.GetName().IsNULLOrEmpty() )
        m_folded[ root.GetName() ] = panel->IsFolded();

    const CWidget::TWidgetVector& children = root.GetChildren();
    for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
    {
        CLayoutState below;
        below.Capture( *children[ i ] );
        for ( TRatioMap::const_iterator r=below.m_ratios.begin(); r!=below.m_ratios.end(); ++r )
            m_ratios[ r->first ] = r->second;
        for ( TFoldedMap::const_iterator f=below.m_folded.begin(); f!=below.m_folded.end(); ++f )
            m_folded[ f->first ] = f->second;
    }
    return (UInt32) ( m_ratios.size() + m_folded.size() );
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutState::Apply( CWidget& root ) const
{GUCEF_TRACE;

    UInt32 applied = 0;
    if ( !root.GetName().IsNULLOrEmpty() )
    {
        CSplitter* splitter = CWidget::As< CSplitter >( &root );
        Float32 ratio = 0.0f;
        if ( GUCEF_NULL != splitter && TryGetSplitRatio( root.GetName(), ratio ) )
        {
            splitter->SetSplitRatio( ratio );
            ++applied;
        }
        CFoldablePanel* panel = CWidget::As< CFoldablePanel >( &root );
        bool folded = false;
        if ( GUCEF_NULL != panel && TryGetFolded( root.GetName(), folded ) )
        {
            panel->SetFolded( folded );
            ++applied;
        }
    }
    const CWidget::TWidgetVector children = root.GetChildren();
    for ( UInt32 i=0; i<(UInt32) children.size(); ++i )
        applied += Apply( *children[ i ] );
    return applied;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::Save( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    for ( TRatioMap::const_iterator r=m_ratios.begin(); r!=m_ratios.end(); ++r )
    {
        CORE::CDataNode* entry = node.AddChild( SplitterNodeName );
        if ( GUCEF_NULL == entry )
            return false;
        entry->SetAttribute( "name", r->first );
        entry->SetAttribute( "ratio", r->second );
    }
    for ( TFoldedMap::const_iterator f=m_folded.begin(); f!=m_folded.end(); ++f )
    {
        CORE::CDataNode* entry = node.AddChild( PanelNodeName );
        if ( GUCEF_NULL == entry )
            return false;
        entry->SetAttribute( "name", f->first );
        entry->SetAttribute( "folded", f->second );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLayoutState::Load( const CORE::CDataNode& node )
{GUCEF_TRACE;

    Clear();
    CORE::CDataNode::const_iterator i = node.ConstBegin();
    while ( i != node.ConstEnd() )
    {
        const CORE::CDataNode* entry = (*i);
        ++i;
        const CORE::CDataNode::TKeyValuePair* nameAttribute = entry->GetAttribute( "name" );
        if ( GUCEF_NULL == nameAttribute )
            continue;
        const CString name = nameAttribute->second.AsString( CString::Empty, true );
        if ( name.IsNULLOrEmpty() )
            continue;
        if ( entry->GetName() == SplitterNodeName )
        {
            // a ratio that is no number is not an entry
            const CORE::CDataNode::TKeyValuePair* ratioAttribute = entry->GetAttribute( "ratio" );
            if ( GUCEF_NULL == ratioAttribute )
                continue;
            const Float32 ratio = ratioAttribute->second.AsFloat32( -1.0f, true );
            if ( ratio >= 0.0f && ratio <= 1.0f )
                m_ratios[ name ] = ratio;
        }
        else
        if ( entry->GetName() == PanelNodeName )
        {
            const CORE::CDataNode::TKeyValuePair* foldedAttribute = entry->GetAttribute( "folded" );
            if ( GUCEF_NULL != foldedAttribute )
                m_folded[ name ] = foldedAttribute->second.AsBool( false, true );
        }
    }
    return !IsEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
