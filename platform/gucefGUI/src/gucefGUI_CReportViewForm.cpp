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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#include "gucefGUI_CReportViewForm.h"

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

const CORE::CEvent CReportViewForm::EntryActivatedEvent = "GUCEF::GUI::CReportViewForm::EntryActivatedEvent";

namespace {

/** The checkboxes of the severities, by severity */
const char* const SeverityCheckboxes[] = { "ShowInfo", "ShowWarning", "ShowError" };

const char* const SeverityTexts[] = { "Info", "Warning", "Error" };

/** What the rows of a severity are drawn in, by default */
const CColour DefaultSeverityColours[] = { CColour( 150, 200, 255, 255 ), CColour( 240, 190, 60, 255 ), CColour( 240, 100, 90, 255 ) };

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReportViewForm::SEntry::SEntry( void )
    : severity( SEVERITY_INFO )
    , asset()
    , message()
    , details()
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CReportViewForm::SEntry::SEntry( ESeverity entrySeverity, const CString& entryAsset, const CString& entryMessage, const CString& entryDetails )
    : severity( entrySeverity )
    , asset( entryAsset )
    , message( entryMessage )
    , details( entryDetails )
    , values()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CReportViewForm::SEntry::GetValue( const CString& key ) const
{GUCEF_TRACE;

    if ( "severity" == key )
        return SeverityToString( severity );
    if ( "asset" == key )
        return asset;
    if ( "message" == key )
        return message;
    if ( "details" == key )
        return details;
    GUCEF::map< CString, CString >::const_iterator i = values.find( key );
    return i != values.end() ? (*i).second : CString::Empty;
}

/*-------------------------------------------------------------------------*/

CReportViewForm::SColumn::SColumn( void )
    : key()
    , title()
    , width( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CReportViewForm::SColumn::SColumn( const CString& columnKey, const CString& columnTitle, Int32 columnWidth )
    : key( columnKey )
    , title( columnTitle )
    , width( columnWidth )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::RegisterEvents( void )
{GUCEF_TRACE;

    EntryActivatedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CReportViewForm::CReportViewForm( void )
    : CFormEx()
    , m_columns()
    , m_entries()
    , m_shown()
    , m_filter()
    , m_filterLower()
    , m_dirty( false )
    , m_waitedInMs( 0 )
    , m_selectedEntry( -1 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        m_severityShown[ i ] = true;
        m_severityColours[ i ] = DefaultSeverityColours[ i ];
    }
    SetColumns( TColumnVector() );
}

/*-------------------------------------------------------------------------*/

CReportViewForm::CReportViewForm( CIGUIContext* context )
    : CFormEx( context )
    , m_columns()
    , m_entries()
    , m_shown()
    , m_filter()
    , m_filterLower()
    , m_dirty( false )
    , m_waitedInMs( 0 )
    , m_selectedEntry( -1 )
    , m_updatingWidgets( false )
    , m_frameSubscribed( false )
{GUCEF_TRACE;

    RegisterEvents();
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        m_severityShown[ i ] = true;
        m_severityColours[ i ] = DefaultSeverityColours[ i ];
    }
    SetColumns( TColumnVector() );
}

/*-------------------------------------------------------------------------*/

CReportViewForm::~CReportViewForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "ReportViewForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "ReportViewForm", "0,0 0,0", "1,0 1,0" );

    // the severities and the filter above the entries
    static const char* const positions[] = { "0,6 0,4", "0,100 0,4", "0,214 0,4" };
    static const char* const sizes[] = { "0,90 0,28", "0,110 0,28", "0,90 0,28" };
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        CORE::CDataNode* box = CFormLayout::AddWidgetNode( *content, "Checkbox", SeverityCheckboxes[ i ], positions[ i ], sizes[ i ] );
        CFormLayout::SetWidgetNodeProperty( *box, EWidgetProperty_Text, SeverityTexts[ i ] );
        CFormLayout::SetWidgetNodeProperty( *box, EWidgetProperty_Checked, "true" );
    }
    CORE::CDataNode* filterLabel = CFormLayout::AddWidgetNode( *content, "Label", "FilterLabel", "0,314 0,8", "0,50 0,24" );
    CFormLayout::SetWidgetNodeProperty( *filterLabel, EWidgetProperty_Text, "Filter" );
    CFormLayout::AddWidgetNode( *content, "Editbox", "Filter", "0,368 0,4", "1,-374 0,28" );

    // the entries take 70 % of the height (less the filter row and the status line), the details the rest below the status: in a
    // low host the grid keeps rows in view instead of only its header (a pixel-sized details box left the grid 26 px in 234)
    CORE::CDataNode* entries = CFormLayout::AddWidgetNode( *content, "GridView", "Entries", "0,0 0,36", "1,0 0.7,-44" );
    CFormLayout::SetWidgetNodeProperty( *entries, EWidgetProperty_FitColumns, "true" );
    CFormLayout::SetWidgetNodeProperty( *entries, EWidgetProperty_TextOverflow, "Ellipsis" );
    CFormLayout::AddWidgetNode( *content, "Label", "Status", "0,6 0.7,-4", "1,-12 0,24" );
    CORE::CDataNode* details = CFormLayout::AddWidgetNode( *content, "Textbox", "Details", "0,0 0.7,24", "1,0 0.3,-24" );
    CFormLayout::SetWidgetNodeProperty( *details, EWidgetProperty_ReadOnly, "true" );
    CFormLayout::SetWidgetNodeProperty( *details, EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::Build( const CString& title )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetContext() )
        return false;
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, true, title );
    if ( !LoadLayout( layout ) )
        return false;
    return Show();
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::Build( CWidget* placeholder )
{GUCEF_TRACE;

    if ( GUCEF_NULL == placeholder )
        return false;
    SetContext( placeholder->GetContext() );
    SetParent( placeholder );
    CORE::CDataNode layout;
    CFormLayout::InitLayout( layout );
    BuildLayout( layout, false, CString::Empty );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        CWidget* box = GetWidget( SeverityCheckboxes[ i ] );
        if ( GUCEF_NULL != box )
        {
            TEventCallback checkedCallback( this, &CReportViewForm::OnSeverityChecked );
            SubscribeTo( box, CCheckbox::BoxCheckedEvent, checkedCallback );
            TEventCallback uncheckedCallback( this, &CReportViewForm::OnSeverityUnchecked );
            SubscribeTo( box, CCheckbox::BoxUncheckedEvent, uncheckedCallback );
        }
    }
    CWidget* widget = GetWidget( "Filter" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CReportViewForm::OnFilterTextChanged );
        SubscribeTo( widget, CEditbox::TextChangedEvent, callback );
    }
    widget = GetWidget( "Entries" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback selectionCallback( this, &CReportViewForm::OnEntriesSelectionChanged );
        SubscribeTo( widget, CGridView::SelectionChangedEvent, selectionCallback );
        TEventCallback itemCallback( this, &CReportViewForm::OnEntriesItemActivated );
        SubscribeTo( widget, CWidget::ItemActivatedEvent, itemCallback );
    }
    CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_frameSubscribed )
    {
        m_frameSubscribed = true;
        TEventCallback callback( this, &CReportViewForm::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, callback );
    }

    m_updatingWidgets = true;
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        CWidget* box = GetWidget( SeverityCheckboxes[ i ] );
        if ( GUCEF_NULL != box )
            box->SetBoolProperty( EWidgetProperty_Checked, m_severityShown[ i ] );
    }
    widget = GetWidget( "Filter" );
    if ( GUCEF_NULL != widget )
        widget->SetProperty( EWidgetProperty_Text, m_filter );
    m_updatingWidgets = false;
    ShowColumns();
    ShowRows();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetColumns( const TColumnVector& columns )
{GUCEF_TRACE;

    if ( columns.empty() )
    {
        m_columns.clear();
        m_columns.push_back( TColumn( "severity", "Severity", 12 ) );
        m_columns.push_back( TColumn( "asset", "Asset", 25 ) );
        m_columns.push_back( TColumn( "message", "Message", 38 ) );
        m_columns.push_back( TColumn( "details", "Details", 25 ) );
    }
    else
        m_columns = columns;
    m_dirty = true;
    ShowColumns();
    ShowRows();
}

/*-------------------------------------------------------------------------*/

const CReportViewForm::TColumnVector&
CReportViewForm::GetColumns( void ) const
{GUCEF_TRACE;

    return m_columns;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ShowColumns( void )
{GUCEF_TRACE;

    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL == grid )
        return;
    CString titles;
    GUCEF::vector< Int32 > widths;
    for ( UInt32 i=0; i<(UInt32)m_columns.size(); ++i )
    {
        titles += ( i > 0 ? "\n" : "" ) + m_columns[ i ].title;
        widths.push_back( m_columns[ i ].width > 0 ? m_columns[ i ].width : 1 );
    }
    m_updatingWidgets = true;
    grid->SetProperty( EWidgetProperty_Columns, titles );
    grid->SetIntListProperty( EWidgetProperty_ColumnWidths, widths );
    m_updatingWidgets = false;
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::IsShown( const TEntry& entry ) const
{GUCEF_TRACE;

    if ( !m_severityShown[ entry.severity ] )
        return false;
    if ( m_filterLower.IsNULLOrEmpty() )
        return true;
    for ( UInt32 i=0; i<(UInt32)m_columns.size(); ++i )
        if ( entry.GetValue( m_columns[ i ].key ).Lowercase().HasSubstr( m_filterLower ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::RebuildShown( void )
{GUCEF_TRACE;

    m_shown.clear();
    for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
        if ( IsShown( m_entries[ i ] ) )
            m_shown.push_back( i );
    if ( m_selectedEntry >= 0 )
    {
        bool stillShown = false;
        for ( UInt32 i=0; i<(UInt32)m_shown.size() && !stillShown; ++i )
            stillShown = (Int32) m_shown[ i ] == m_selectedEntry;
        if ( !stillShown )
            m_selectedEntry = -1;
    }
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ShowRows( void )
{GUCEF_TRACE;

    m_dirty = false;
    m_waitedInMs = 0;
    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL != grid )
    {
        CGridView::TRowVector rows;
        CString colours;
        Int32 selectedRow = -1;
        for ( UInt32 r=0; r<(UInt32)m_shown.size(); ++r )
        {
            const TEntry& entry = m_entries[ m_shown[ r ] ];
            CGridView::TStringVector cells;
            for ( UInt32 c=0; c<(UInt32)m_columns.size(); ++c )
            {
                // a row of the grid is a line of text with tabs between the cells: a cell is one line
                cells.push_back( entry.GetValue( m_columns[ c ].key ).ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' ).ReplaceChar( '\t', ' ' ) );
            }
            rows.push_back( cells );
            colours += ( r > 0 ? "\n" : "" ) + m_severityColours[ entry.severity ].ToString();
            if ( (Int32) m_shown[ r ] == m_selectedEntry )
                selectedRow = (Int32) r;
        }
        m_updatingWidgets = true;
        grid->SetRows( rows );
        grid->SetProperty( EWidgetProperty_ItemColours, colours );
        if ( selectedRow >= 0 )
            grid->SetSelectedRow( selectedRow );
        m_updatingWidgets = false;
    }
    ShowStatus();
    ShowDetails();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ShowStatus( void )
{GUCEF_TRACE;

    CWidget* status = GetWidget( "Status" );
    if ( GUCEF_NULL == status )
        return;
    CString text = CORE::UInt32ToString( (UInt32) m_shown.size() ) + " of " + CORE::UInt32ToString( (UInt32) m_entries.size() ) + " shown";
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
        text += ", " + CORE::UInt32ToString( GetEntryCount( (ESeverity) i ) ) + " " + CString( SeverityTexts[ i ] ).Lowercase();
    status->SetProperty( EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ShowDetails( void )
{GUCEF_TRACE;

    CWidget* details = GetWidget( "Details" );
    if ( GUCEF_NULL == details )
        return;
    CString text;
    if ( m_selectedEntry >= 0 && m_selectedEntry < (Int32) m_entries.size() )
    {
        const TEntry& entry = m_entries[ m_selectedEntry ];
        text = SeverityToString( entry.severity ) + ": " + entry.message;
        if ( !entry.asset.IsNULLOrEmpty() )
            text += "\n" + entry.asset;
        if ( !entry.details.IsNULLOrEmpty() )
            text += "\n\n" + entry.details;
    }
    details->SetProperty( EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetEntries( const CORE::CDataNode& list )
{GUCEF_TRACE;

    TEntryVector entries;
    for ( CORE::CDataNode::const_iterator i = list.ConstBegin(); i != list.ConstEnd(); ++i )
    {
        const CORE::CDataNode* node = (*i);
        TEntry entry;
        for ( CORE::CDataNode::TAttributeMap::const_iterator a = node->AttributeBegin(); a != node->AttributeEnd(); ++a )
        {
            const CString& name = (*a).first;
            const CString value = (*a).second.AsString();
            if ( "severity" == name )
            {
                if ( !SeverityFromString( value, entry.severity ) )
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ReportViewForm: The severity \"" << value << "\" is no severity, the entry is an info" );
            }
            else
            if ( "asset" == name )
                entry.asset = value;
            else
            if ( "message" == name )
                entry.message = value;
            else
            if ( "details" == name )
                entry.details = value;
            else
                entry.values[ name ] = value;
        }
        entries.push_back( entry );
    }
    SetEntries( entries );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetEntries( const TEntryVector& entries )
{GUCEF_TRACE;

    m_entries = entries;
    m_selectedEntry = -1;
    RebuildShown();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::AddEntry( const TEntry& entry )
{GUCEF_TRACE;

    m_entries.push_back( entry );
    if ( IsShown( entry ) )
        m_shown.push_back( (UInt32) m_entries.size() - 1 );
    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::AddEntry( ESeverity severity, const CString& asset, const CString& message, const CString& details )
{GUCEF_TRACE;

    AddEntry( TEntry( severity, asset, message, details ) );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ClearEntries( void )
{GUCEF_TRACE;

    SetEntries( TEntryVector() );
}

/*-------------------------------------------------------------------------*/

UInt32
CReportViewForm::GetEntryCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

const CReportViewForm::TEntry&
CReportViewForm::GetEntry( UInt32 index ) const
{GUCEF_TRACE;

    static const TEntry none;
    return index < (UInt32) m_entries.size() ? m_entries[ index ] : none;
}

/*-------------------------------------------------------------------------*/

UInt32
CReportViewForm::GetEntryCount( ESeverity severity ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)m_entries.size(); ++i )
        if ( m_entries[ i ].severity == severity )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetSeverityShown( ESeverity severity, bool shown )
{GUCEF_TRACE;

    if ( (UInt32) severity >= (UInt32) SEVERITY_COUNT || m_severityShown[ severity ] == shown )
        return;
    m_severityShown[ severity ] = shown;
    CWidget* box = GetWidget( SeverityCheckboxes[ severity ] );
    if ( GUCEF_NULL != box )
    {
        m_updatingWidgets = true;
        box->SetBoolProperty( EWidgetProperty_Checked, shown );
        m_updatingWidgets = false;
    }
    RebuildShown();
    Refresh();
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::IsSeverityShown( ESeverity severity ) const
{GUCEF_TRACE;

    return (UInt32) severity < (UInt32) SEVERITY_COUNT && m_severityShown[ severity ];
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetFilter( const CString& text )
{GUCEF_TRACE;

    if ( text == m_filter )
        return;
    m_filter = text;
    m_filterLower = text.Lowercase();
    CWidget* filter = GetWidget( "Filter" );
    if ( GUCEF_NULL != filter && filter->GetProperty( EWidgetProperty_Text ) != text )
    {
        m_updatingWidgets = true;
        filter->SetProperty( EWidgetProperty_Text, text );
        m_updatingWidgets = false;
    }
    RebuildShown();
    Refresh();
}

/*-------------------------------------------------------------------------*/

const CString&
CReportViewForm::GetFilter( void ) const
{GUCEF_TRACE;

    return m_filter;
}

/*-------------------------------------------------------------------------*/

UInt32
CReportViewForm::GetShownCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_shown.size();
}

/*-------------------------------------------------------------------------*/

Int32
CReportViewForm::GetShownEntryIndex( UInt32 row ) const
{GUCEF_TRACE;

    return row < (UInt32) m_shown.size() ? (Int32) m_shown[ row ] : -1;
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::SelectEntry( Int32 index )
{GUCEF_TRACE;

    if ( m_dirty )
        Refresh();
    for ( UInt32 r=0; r<(UInt32)m_shown.size(); ++r )
    {
        if ( (Int32) m_shown[ r ] == index )
        {
            CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
            if ( GUCEF_NULL != grid )
            {
                m_updatingWidgets = true;
                grid->SetSelectedRow( (Int32) r );
                grid->ScrollToRow( (Int32) r );
                m_updatingWidgets = false;
            }
            m_selectedEntry = index;
            ShowDetails();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

Int32
CReportViewForm::GetSelectedEntry( void ) const
{GUCEF_TRACE;

    return m_selectedEntry;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::Refresh( void )
{GUCEF_TRACE;

    ShowRows();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::Update( UInt32 deltaInMs )
{GUCEF_TRACE;

    if ( !m_dirty )
        return;
    m_waitedInMs += deltaInMs;
    if ( m_waitedInMs >= (UInt32) RefreshIntervalInMs )
        Refresh();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::SetSeverityColour( ESeverity severity, const CColour& colour )
{GUCEF_TRACE;

    if ( (UInt32) severity >= (UInt32) SEVERITY_COUNT )
        return;
    m_severityColours[ severity ] = colour;
    m_dirty = true;
    Refresh();
}

/*-------------------------------------------------------------------------*/

CColour
CReportViewForm::GetSeverityColour( ESeverity severity ) const
{GUCEF_TRACE;

    return (UInt32) severity < (UInt32) SEVERITY_COUNT ? m_severityColours[ severity ] : CColour();
}

/*-------------------------------------------------------------------------*/

CString
CReportViewForm::SeverityToString( ESeverity severity )
{GUCEF_TRACE;

    return (UInt32) severity < (UInt32) SEVERITY_COUNT ? CString( SeverityTexts[ severity ] ).Lowercase() : CString( "info" );
}

/*-------------------------------------------------------------------------*/

bool
CReportViewForm::SeverityFromString( const CString& text, ESeverity& severity )
{GUCEF_TRACE;

    const CString lower = text.Lowercase();
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        if ( lower == CString( SeverityTexts[ i ] ).Lowercase() || lower == CORE::UInt32ToString( i ) )
        {
            severity = (ESeverity) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::ChangeSeverityShown( CORE::CNotifier* checkbox, bool shown )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    for ( UInt32 i=0; i<(UInt32) SEVERITY_COUNT; ++i )
    {
        if ( checkbox == GetWidget( SeverityCheckboxes[ i ] ) )
        {
            SetSeverityShown( (ESeverity) i, shown );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnSeverityChecked( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ChangeSeverityShown( notifier, true );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnSeverityUnchecked( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ChangeSeverityShown( notifier, false );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnFilterTextChanged( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CWidget* filter = GetWidget( "Filter" );
    if ( GUCEF_NULL != filter )
        SetFilter( filter->GetProperty( EWidgetProperty_Text ) );
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnEntriesSelectionChanged( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updatingWidgets )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL == grid )
        return;
    const Int32 row = grid->GetSelectedRow();
    m_selectedEntry = row >= 0 && row < (Int32) m_shown.size() ? (Int32) m_shown[ row ] : -1;
    ShowDetails();
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnEntriesItemActivated( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CRoutedEventData* routed = static_cast< CRoutedEventData* >( eventData );
    if ( GUCEF_NULL != routed && EVENTPHASE_TUNNEL == routed->phase )
        return;
    CGridView* grid = GetWidgetAs< CGridView >( "Entries" );
    if ( GUCEF_NULL == grid )
        return;
    const Int32 row = grid->GetSelectedRow();
    if ( row >= 0 && row < (Int32) m_shown.size() )
    {
        CORE::TCloneableInt32 data( (Int32) m_shown[ row ] );
        NotifyObservers( EntryActivatedEvent, &data );
    }
}

/*-------------------------------------------------------------------------*/

void
CReportViewForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
        Update( static_cast< CORE::TCloneableUInt32* >( eventData )->GetData() );
}

/*-------------------------------------------------------------------------*/

const CString&
CReportViewForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CReportViewForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
