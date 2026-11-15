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

#include <math.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#include "gucefGUI_CPropertyInspectorForm.h"

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

const CORE::CEvent CPropertyInspectorForm::PropertyChangedEvent = "GUCEF::GUI::CPropertyInspectorForm::PropertyChangedEvent";
const CORE::CEvent CPropertyInspectorForm::AppliedEvent = "GUCEF::GUI::CPropertyInspectorForm::AppliedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The room a level of the tree takes in the first column */
const char* const IndentText = "  ";

typedef CPropertyRowDescriptor TDescriptor;

/** The node of a path of nodes below the root, NULL when there is none; "" is the root itself */
CORE::CDataNode*
FindNodeByPath( CORE::CDataNode& root, const CString& nodePath )
{
    if ( nodePath.IsNULLOrEmpty() )
        return &root;
    const CString::StringVector parts = nodePath.ParseElements( '/', false );
    CORE::CDataNode* current = &root;
    for ( UInt32 p=0; p<(UInt32)parts.size(); ++p )
    {
        CString name = parts[ p ];
        UInt32 wanted = 0;
        const Int32 open = name.HasChar( '[', false );
        if ( open > 0 && name.Length() > (UInt32) open + 1 && ']' == name[ name.Length() - 1 ] )
        {
            wanted = CORE::StringToUInt32( name.SubstrFromRange( (UInt32) open + 1, name.Length() - 1 ), 0 );
            name = name.SubstrToIndex( (UInt32) open, true );
        }
        CORE::CDataNode* found = GUCEF_NULL;
        UInt32 seen = 0;
        for ( CORE::CDataNode::iterator i = current->Begin(); i != current->End(); ++i )
        {
            if ( (*i)->GetName() == name )
            {
                if ( seen == wanted )
                {
                    found = (*i);
                    break;
                }
                ++seen;
            }
        }
        if ( GUCEF_NULL == found )
            return GUCEF_NULL;
        current = found;
    }
    return current;
}

/** The path of a child node: its name, with its place among the nodes of that name when it is not the first */
CString
ChildName( const CString& name, UInt32 occurrence )
{
    return 0 == occurrence ? name : name + "[" + CORE::UInt32ToString( occurrence ) + "]";
}

CString
JoinNodePath( const CString& parent, const CString& child )
{
    return parent.IsNULLOrEmpty() ? child : parent + "/" + child;
}

/** A node that holds one value and nothing else */
bool
IsValueNode( const CORE::CDataNode& node )
{
    return !node.HasChildren() && node.AttributeBegin() == node.AttributeEnd() && node.HasValue();
}

/** The variant for the text, of the type of the variant the value had, as far as the text fits it */
CORE::CVariant
MakeValue( const CORE::CVariant& original, const CString& text )
{
    if ( original.IsBoolean() || original.IsInteger() || original.IsFloat() )
    {
        CORE::CVariant typed( original.GetTypeId(), text );
        if ( original.IsBoolean() )
            return typed;
        Float64 wanted = 0.0;
        if ( CPropertyRowDescriptor::TextToNumber( text, wanted ) )
        {
            const Float64 got = typed.AsFloat64();
            if ( fabs( got - wanted ) <= 1e-9 * ( fabs( wanted ) + 1.0 ) )
                return typed;
        }
    }
    return CORE::CVariant( text );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      THE MODEL                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A row of the list: a group or a value, and where it is in the tree */
struct SPropertyRow
{
    bool isGroup;
    CString path;
    CString label;
    Int32 depth;
    CString nodePath;
    CString attribute;
    bool isAttribute;
    CString text;                   /**< the value as text */
    bool hasDescriptor;
    TDescriptor descriptor;
    CTableViewColumn column;        /**< the definition of the value cell */
    bool expanded;

    SPropertyRow( void )
        : isGroup( false ), path(), label(), depth( 0 ), nodePath(), attribute(), isAttribute( false ), text()
        , hasDescriptor( false ), descriptor(), column(), expanded( true ) {}
};
typedef GUCEF::vector< SPropertyRow > TPropertyRowVector;

/**
 *  The rows of the inspector for the grid. A row has a definition of its own (GetCellColumn()): its value is a number,
 *  a bool, a choice or a text, so the editors of the grid are the editors of the kind of the value
 */
class CPropertyInspectorModel : public CITableViewModel
{
    public:

    explicit CPropertyInspectorModel( CPropertyInspectorForm* owner )
        : CITableViewModel(), m_owner( owner ), m_rows(), m_visible(), m_nameColumn( "Name", CORE::DATATABLE_TYPE_UTF8, 0, true )
        , m_valueColumn( "Value", CORE::DATATABLE_TYPE_UTF8, 0, true )
    {
    }

    virtual ~CPropertyInspectorModel() GUCEF_VIRTUAL_OVERRIDE
    {
    }

    void Detach( void )
    {
        m_owner = GUCEF_NULL;
    }

    /** Takes the rows; the ones below a folded group are not shown */
    void SetRows( const TPropertyRowVector& rows )
    {
        m_rows = rows;
        m_visible.clear();
        Int32 hiddenBelow = -1;
        for ( UInt32 i=0; i<(UInt32)m_rows.size(); ++i )
        {
            if ( hiddenBelow >= 0 && m_rows[ i ].depth > hiddenBelow )
                continue;
            hiddenBelow = -1;
            m_visible.push_back( i );
            if ( m_rows[ i ].isGroup && !m_rows[ i ].expanded )
                hiddenBelow = m_rows[ i ].depth;
        }
        NotifyObservers( ModelResetEvent );
    }

    const SPropertyRow* FindRowByPath( const CString& path ) const
    {
        for ( UInt32 i=0; i<(UInt32)m_rows.size(); ++i )
            if ( m_rows[ i ].path == path )
                return &m_rows[ i ];
        return GUCEF_NULL;
    }

    const SPropertyRow* GetShownRow( UInt32 row ) const
    {
        return row < (UInt32) m_visible.size() ? &m_rows[ m_visible[ row ] ] : GUCEF_NULL;
    }

    Int32 FindShownRow( const CString& path ) const
    {
        for ( UInt32 i=0; i<(UInt32)m_visible.size(); ++i )
            if ( m_rows[ m_visible[ i ] ].path == path )
                return (Int32) i;
        return -1;
    }

    /** A value changed: the text of the row, and the cell is drawn again */
    void ChangeText( const CString& path, const CString& text )
    {
        for ( UInt32 i=0; i<(UInt32)m_rows.size(); ++i )
        {
            if ( m_rows[ i ].path != path )
                continue;
            m_rows[ i ].text = text;
            const Int32 shown = FindShownRow( path );
            if ( shown >= 0 )
            {
                TTableViewRangeEventData range( CTableViewRange( (UInt32) shown, 1, 1, 1 ) );
                NotifyObservers( CellsChangedEvent, &range );
            }
            return;
        }
    }

    virtual UInt32 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return (UInt32) m_visible.size();
    }

    virtual UInt32 GetColumnCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return 2;
    }

    virtual const CTableViewColumn& GetColumn( UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return 0 == column ? m_nameColumn : m_valueColumn;
    }

    virtual const CTableViewColumn& GetCellColumn( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        if ( GUCEF_NULL == entry || 0 == column )
            return GetColumn( column );
        return entry->column;
    }

    virtual bool GetCellValue( UInt32 row, UInt32 column, CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        if ( GUCEF_NULL == entry || column > 1 )
            return false;
        if ( 0 == column )
        {
            value = CORE::CVariant( LabelOf( *entry ) );
            return true;
        }
        if ( entry->isGroup )
            return false;
        switch ( entry->column.type )
        {
            case CORE::DATATABLE_TYPE_BOOL:
            {
                bool on = false;
                CString ignored;
                if ( entry->descriptor.Validate( entry->text, ignored ) )
                    on = "true" == entry->descriptor.Normalize( entry->text );
                value = CORE::CVariant( on );
                return true;
            }
            case CORE::DATATABLE_TYPE_INT64:
            {
                Float64 number = 0.0;
                CPropertyRowDescriptor::TextToNumber( entry->text, number );
                value = CORE::CVariant( (Int64) number );
                return true;
            }
            case CORE::DATATABLE_TYPE_FLOAT64:
            {
                Float64 number = 0.0;
                CPropertyRowDescriptor::TextToNumber( entry->text, number );
                value = CORE::CVariant( number );
                return true;
            }
            default:
                value = CORE::CVariant( entry->text );
                return true;
        }
    }

    virtual bool GetCellContent( UInt32 row, UInt32 column, SCellContent& content ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        content.kind = SCellContent::KIND_TEXT;
        content.isNumber = false;
        content.text.Clear();
        if ( GUCEF_NULL == entry || column > 1 )
            return false;
        if ( 0 == column )
        {
            content.text = LabelOf( *entry );
            return true;
        }
        if ( entry->isGroup )
            return true;
        if ( CORE::DATATABLE_TYPE_BOOL == entry->column.type )
        {
            bool on = false;
            CString ignored;
            if ( entry->descriptor.Validate( entry->text, ignored ) )
                on = "true" == entry->descriptor.Normalize( entry->text );
            content.kind = on ? SCellContent::KIND_CHECK_ON : SCellContent::KIND_CHECK_OFF;
            content.text = on ? "true" : "false";
            return true;
        }
        content.text = entry->hasDescriptor ? entry->descriptor.FormatForDisplay( entry->text ) : entry->text;
        return true;
    }

    virtual bool GetRowKey( UInt32 row, CORE::CVariant& key ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        if ( GUCEF_NULL == entry )
            return false;
        key = CORE::CVariant( entry->path );
        return true;
    }

    virtual bool FindRowByKey( const CORE::CVariant& key, UInt32& row ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const Int32 found = FindShownRow( key.AsString() );
        if ( found < 0 )
            return false;
        row = (UInt32) found;
        return true;
    }

    virtual bool IsCellReadOnly( UInt32 row, UInt32 column ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        return GUCEF_NULL == entry || 0 == column || entry->isGroup || !entry->hasDescriptor || entry->descriptor.readOnly;
    }

    virtual CTableEditResult ValidateCellEdit( UInt32 row, UInt32 column, const CORE::CVariant& value ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        if ( GUCEF_NULL == entry || IsCellReadOnly( row, column ) )
            return CTableEditResult::ReadOnly();
        CString problem;
        if ( !entry->descriptor.Validate( TextOf( *entry, value ), problem ) )
            return CTableEditResult::Refused( problem );
        return CTableEditResult::Accepted();
    }

    virtual CTableEditResult SetCellValue( UInt32 row, UInt32 column, const CORE::CVariant& value ) GUCEF_VIRTUAL_OVERRIDE
    {
        const SPropertyRow* entry = GetShownRow( row );
        if ( GUCEF_NULL == entry || IsCellReadOnly( row, column ) || GUCEF_NULL == m_owner )
            return CTableEditResult::ReadOnly();
        const CString path = entry->path;
        CString problem;
        if ( !m_owner->ChangeValue( path, TextOf( *entry, value ), &problem, true ) )
            return CTableEditResult::Refused( problem );
        return CTableEditResult::Accepted();
    }

    private:

    static CString LabelOf( const SPropertyRow& entry )
    {
        CString label;
        for ( Int32 d=0; d<entry.depth; ++d )
            label += IndentText;
        if ( entry.isGroup )
            label += entry.expanded ? "- " : "+ ";
        return label + entry.label;
    }

    /** What a value the grid parsed is as text for the descriptor */
    static CString TextOf( const SPropertyRow& entry, const CORE::CVariant& value )
    {
        switch ( entry.column.type )
        {
            case CORE::DATATABLE_TYPE_BOOL:     return value.AsBool() ? "true" : "false";
            case CORE::DATATABLE_TYPE_INT64:    return CORE::Int64ToString( value.AsInt64() );
            case CORE::DATATABLE_TYPE_FLOAT64:  return CPropertyRowDescriptor::NumberToText( value.AsFloat64(), entry.descriptor.decimals );
            default:                            return value.AsString();
        }
    }

    CPropertyInspectorModel( const CPropertyInspectorModel& src );
    CPropertyInspectorModel& operator=( const CPropertyInspectorModel& src );

    CPropertyInspectorForm* m_owner;
    TPropertyRowVector m_rows;
    GUCEF::vector< UInt32 > m_visible;
    CTableViewColumn m_nameColumn;
    CTableViewColumn m_valueColumn;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::RegisterEvents( void )
{GUCEF_TRACE;

    PropertyChangedEvent.Initialize();
    AppliedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CPropertyInspectorForm::CPropertyInspectorForm( void )
    : CFormEx()
    , m_data()
    , m_source( GUCEF_NULL )
    , m_descriptors()
    , m_collapsed()
    , m_model( GUCEF_NULL )
    , m_modelPtr()
    , m_pathField( GUCEF_NULL )
    , m_liveEditing( false )
    , m_modified( false )
    , m_updatingWidgets( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_model = GUCEF_NEW CPropertyInspectorModel( this );
    m_modelPtr = TTableViewModelPtr( m_model );
}

/*-------------------------------------------------------------------------*/

CPropertyInspectorForm::CPropertyInspectorForm( CIGUIContext* context )
    : CFormEx( context )
    , m_data()
    , m_source( GUCEF_NULL )
    , m_descriptors()
    , m_collapsed()
    , m_model( GUCEF_NULL )
    , m_modelPtr()
    , m_pathField( GUCEF_NULL )
    , m_liveEditing( false )
    , m_modified( false )
    , m_updatingWidgets( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_model = GUCEF_NEW CPropertyInspectorModel( this );
    m_modelPtr = TTableViewModelPtr( m_model );
}

/*-------------------------------------------------------------------------*/

CPropertyInspectorForm::~CPropertyInspectorForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_pathField;
    m_pathField = GUCEF_NULL;
    if ( GUCEF_NULL != m_model )
        m_model->Detach();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title )
{GUCEF_TRACE;

    CORE::CDataNode* content = GUCEF_NULL;
    if ( asWindow )
    {
        CORE::CDataNode* window = CFormLayout::AddWidgetNode( layout, "Window", "PropertyInspectorForm", "0.1,0 0.1,0", "0.8,0 0.8,0" );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_Title, title );
        CFormLayout::SetWidgetNodeProperty( *window, EWidgetProperty_TitleHeight, "28" );
        content = CFormLayout::AddWidgetNode( *window, "Panel", "Content", "0,0 0,28", "1,0 1,-28" );
    }
    else
        content = CFormLayout::AddWidgetNode( layout, "Panel", "PropertyInspectorForm", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* grid = CFormLayout::AddWidgetNode( *content, "GridView", "Properties", "0,0 0,0", "1,0 1,-88" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_Columns, "Name\nValue" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_FitColumns, "true" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_ColumnWidths, "2,3" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_SelectionMode, "Cell" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_CellEditing, "true" );
    CFormLayout::SetWidgetNodeProperty( *grid, EWidgetProperty_TextOverflow, "Ellipsis" );

    // the path of a value that is a path: a field with a button, while the cursor is on it
    CORE::CDataNode* pathEditor = CFormLayout::AddWidgetNode( *content, "Panel", "PathEditor", "0,0 1,-86", "1,0 0,40" );
    CFormLayout::SetWidgetNodeProperty( *pathEditor, EWidgetProperty_Visible, "false" );
    CORE::CDataNode* pathLabel = CFormLayout::AddWidgetNode( *pathEditor, "Label", "PathLabel", "0,6 0,8", "0,150 0,24" );
    CFormLayout::SetWidgetNodeProperty( *pathLabel, EWidgetProperty_Text, "Path" );
    CFormLayout::AddWidgetNode( *pathEditor, "Panel", "PathHolder", "0,160 0,3", "1,-166 0,34" );

    CORE::CDataNode* apply = CFormLayout::AddWidgetNode( *content, "Button", "Apply", "1,-206 1,-40", "0,96 0,34" );
    CFormLayout::SetWidgetNodeProperty( *apply, EWidgetProperty_Text, "Apply" );
    CFormLayout::SetWidgetNodeProperty( *apply, EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* revert = CFormLayout::AddWidgetNode( *content, "Button", "Revert", "1,-104 1,-40", "0,96 0,34" );
    CFormLayout::SetWidgetNodeProperty( *revert, EWidgetProperty_Text, "Revert" );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::Build( const CString& title )
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
CPropertyInspectorForm::Build( CWidget* placeholder )
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
CPropertyInspectorForm::OnPreLayoutLoad( void )
{GUCEF_TRACE;

    // the path field is made in the layout that goes now: it takes its widgets away while they are there
    GUCEF_DELETE m_pathField;
    m_pathField = GUCEF_NULL;
    CFormEx::OnPreLayoutLoad();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CFormEx::OnPostLayoutLoad();

    CWidget* widget = GetWidget( "Apply" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CPropertyInspectorForm::OnApplyActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Revert" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CPropertyInspectorForm::OnRevertActivated );
        SubscribeTo( widget, CWidget::ActivatedEvent, callback );
    }
    CGridView* grid = GetGrid();
    if ( GUCEF_NULL != grid )
    {
        grid->SetModel( m_modelPtr );
        TEventCallback clickedCallback( this, &CPropertyInspectorForm::OnGridItemClicked );
        SubscribeTo( grid, CGridView::GridItemClickedEvent, clickedCallback );
        TEventCallback keyCallback( this, &CPropertyInspectorForm::OnGridKeyDown );
        SubscribeTo( grid, CWidget::KeyDownEvent, keyCallback );
        TEventCallback moveCallback( this, &CPropertyInspectorForm::OnGridMouseMove );
        SubscribeTo( grid, CWidget::MouseMoveEvent, moveCallback );
        TEventCallback cursorCallback( this, &CPropertyInspectorForm::OnGridCursorChanged );
        SubscribeTo( grid, CGridView::CellCursorChangedEvent, cursorCallback );
    }

    CWidget* holder = GetWidget( "PathHolder" );
    if ( GUCEF_NULL != holder )
    {
        m_pathField = GUCEF_NEW CPathFieldForm( GetContext(), CPathFieldForm::PATHKIND_FILE );
        if ( m_pathField->Build( holder ) )
        {
            TEventCallback chosenCallback( this, &CPropertyInspectorForm::OnPathChosen );
            SubscribeTo( m_pathField, CPathFieldForm::PathChosenEvent, chosenCallback );
            CWidget* line = m_pathField->GetWidget( "Path" );
            if ( GUCEF_NULL != line )
            {
                TEventCallback enteredCallback( this, &CPropertyInspectorForm::OnPathEntered );
                SubscribeTo( line, CEditbox::TextEnteredEvent, enteredCallback );
            }
        }
    }
    RebuildRows();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

CGridView*
CPropertyInspectorForm::GetGrid( void )
{GUCEF_TRACE;

    return GetWidgetAs< CGridView >( "Properties" );
}

/*-------------------------------------------------------------------------*/

CPathFieldForm*
CPropertyInspectorForm::GetPathField( void )
{GUCEF_TRACE;

    return m_pathField;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::SetDescriptor( const CString& path, const CPropertyRowDescriptor& descriptor )
{GUCEF_TRACE;

    m_descriptors[ path ] = descriptor;
    RebuildRows();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::SetDescriptors( const TDescriptorMap& descriptors )
{GUCEF_TRACE;

    m_descriptors = descriptors;
    RebuildRows();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::ClearDescriptors( void )
{GUCEF_TRACE;

    m_descriptors.clear();
    RebuildRows();
}

/*-------------------------------------------------------------------------*/

const CPropertyRowDescriptor*
CPropertyInspectorForm::GetDescriptor( const CString& path ) const
{GUCEF_TRACE;

    TDescriptorMap::const_iterator i = m_descriptors.find( path );
    return i != m_descriptors.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::SetData( CORE::CDataNode* source )
{GUCEF_TRACE;

    m_source = source;
    m_data.Clear();
    if ( GUCEF_NULL != source )
        m_data = *source;
    m_modified = false;
    RebuildRows();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::SetDataCopy( const CORE::CDataNode& data )
{GUCEF_TRACE;

    m_source = GUCEF_NULL;
    m_data = data;
    m_modified = false;
    RebuildRows();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
CPropertyInspectorForm::GetData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::IsModified( void ) const
{GUCEF_TRACE;

    return m_modified;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::WriteToSource( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_source )
        return;
    *m_source = m_data;
    m_modified = false;
    ShowButtons();
    NotifyObservers( AppliedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::Apply( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_source )
        return false;
    WriteToSource();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::Revert( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_source )
        return false;
    SetData( m_source );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::SetLiveEditing( bool liveEditing )
{GUCEF_TRACE;

    m_liveEditing = liveEditing;
    if ( m_liveEditing && m_modified && GUCEF_NULL != m_source )
        WriteToSource();
    ShowButtons();
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::IsLiveEditing( void ) const
{GUCEF_TRACE;

    return m_liveEditing;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::ShowButtons( void )
{GUCEF_TRACE;

    const bool canApply = GUCEF_NULL != m_source && m_modified && !m_liveEditing;
    CWidget* apply = GetWidget( "Apply" );
    if ( GUCEF_NULL != apply )
        apply->SetBoolProperty( EWidgetProperty_Enabled, canApply );
    CWidget* revert = GetWidget( "Revert" );
    if ( GUCEF_NULL != revert )
        revert->SetBoolProperty( EWidgetProperty_Enabled, canApply );
}

/*-------------------------------------------------------------------------*/

namespace {

/** Adds the rows of a node: its attributes, then its nodes, a group for each node that has more than a value */
void
AppendRows( const CORE::CDataNode& node                                      ,
            const CString& nodePath                                          ,
            Int32 depth                                                      ,
            const CPropertyInspectorForm::TDescriptorMap& descriptors        ,
            const GUCEF::set< CString >& collapsed                           ,
            TPropertyRowVector& rows                                         )
{
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
    {
        SPropertyRow row;
        row.isGroup = false;
        row.nodePath = nodePath;
        row.attribute = (*a).first;
        row.isAttribute = true;
        row.path = nodePath + "@" + (*a).first;
        row.label = (*a).first;
        row.depth = depth;
        row.text = (*a).second.AsString();
        rows.push_back( row );
    }

    GUCEF::map< CString, UInt32 > occurrences;
    for ( CORE::CDataNode::const_iterator c = node.ConstBegin(); c != node.ConstEnd(); ++c )
    {
        const CORE::CDataNode* child = (*c);
        const UInt32 occurrence = occurrences[ child->GetName() ]++;
        const CString name = ChildName( child->GetName(), occurrence );
        const CString childPath = JoinNodePath( nodePath, name );
        SPropertyRow row;
        row.nodePath = childPath;
        row.path = childPath;
        row.label = name;
        row.depth = depth;
        if ( IsValueNode( *child ) )
        {
            row.isGroup = false;
            row.isAttribute = false;
            row.text = child->GetValue().AsString();
            rows.push_back( row );
        }
        else
        {
            row.isGroup = true;
            row.expanded = collapsed.find( childPath ) == collapsed.end();
            rows.push_back( row );
            AppendRows( *child, childPath, depth + 1, descriptors, collapsed, rows );
        }
    }
}

}

void
CPropertyInspectorForm::RebuildRows( void )
{GUCEF_TRACE;

    TPropertyRowVector rows;
    AppendRows( m_data, CString::Empty, 0, m_descriptors, m_collapsed, rows );

    // what the application says about a value: its descriptor, and the definition of the cell that goes with it
    for ( UInt32 i=0; i<(UInt32)rows.size(); ++i )
    {
        SPropertyRow& row = rows[ i ];
        row.column = CTableViewColumn( "Value", CORE::DATATABLE_TYPE_UTF8, 0, true );
        if ( row.isGroup )
            continue;
        TDescriptorMap::const_iterator d = m_descriptors.find( row.path );
        if ( d == m_descriptors.end() )
            continue;
        row.hasDescriptor = true;
        row.descriptor = (*d).second;
        row.text = row.descriptor.Normalize( row.text );        // the text of the value as its kind writes it
        if ( !row.descriptor.label.IsNULLOrEmpty() )
            row.label = row.descriptor.label;
        row.column.readOnly = row.descriptor.readOnly;
        switch ( row.descriptor.type )
        {
            case TDescriptor::VALUETYPE_NUMBER:
            {
                row.column.type = 0 == row.descriptor.decimals ? CORE::DATATABLE_TYPE_INT64 : CORE::DATATABLE_TYPE_FLOAT64;
                row.column.decimals = row.descriptor.decimals;
                if ( !row.descriptor.unit.IsNULLOrEmpty() )
                    row.column.suffix = " " + row.descriptor.unit;
                break;
            }
            case TDescriptor::VALUETYPE_BOOL:
                row.column.type = CORE::DATATABLE_TYPE_BOOL;
                break;
            case TDescriptor::VALUETYPE_CHOICE:
                row.column.choices = row.descriptor.choices;
                break;
            default:
                break;
        }
    }
    m_model->SetRows( rows );
    UpdatePathEditor();
}

/*-------------------------------------------------------------------------*/

UInt32
CPropertyInspectorForm::GetRowCount( void ) const
{GUCEF_TRACE;

    return m_model->GetRowCount();
}

/*-------------------------------------------------------------------------*/

CString
CPropertyInspectorForm::GetRowPath( UInt32 row ) const
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->GetShownRow( row );
    return GUCEF_NULL != entry ? entry->path : CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CPropertyInspectorForm::FindRow( const CString& path ) const
{GUCEF_TRACE;

    return m_model->FindShownRow( path );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::IsGroupRow( UInt32 row ) const
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->GetShownRow( row );
    return GUCEF_NULL != entry && entry->isGroup;
}

/*-------------------------------------------------------------------------*/

CString
CPropertyInspectorForm::GetValue( const CString& path ) const
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->FindRowByPath( path );
    return GUCEF_NULL != entry && !entry->isGroup ? entry->text : CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::SetValue( const CString& path, const CString& text, CString* problem )
{GUCEF_TRACE;

    return ChangeValue( path, text, problem, false );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::ChangeValue( const CString& path, const CString& text, CString* problem, bool byUser )
{GUCEF_TRACE;

    CString reason;
    const SPropertyRow* entry = m_model->FindRowByPath( path );
    if ( GUCEF_NULL == entry || entry->isGroup )
        reason = "There is no value at this path";
    else
    if ( !entry->hasDescriptor )
        reason = "The value cannot be changed";
    else
    if ( entry->descriptor.readOnly )
        reason = "The value is read only";
    else
        entry->descriptor.Validate( text, reason );
    if ( !reason.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != problem )
            *problem = reason;
        return false;
    }

    const CString oldText = entry->text;
    const CString newText = entry->descriptor.Normalize( text );
    if ( newText == oldText )
        return true;

    // into the copy of the tree, in the type the value had
    CORE::CDataNode* node = FindNodeByPath( m_data, entry->nodePath );
    if ( GUCEF_NULL == node )
    {
        if ( GUCEF_NULL != problem )
            *problem = "The value is not in the tree any more";
        return false;
    }
    if ( entry->isAttribute )
    {
        const CORE::CVariant& original = node->GetAttributeValue( entry->attribute );
        node->SetAttribute( entry->attribute, MakeValue( original, newText ) );
    }
    else
        node->SetValue( MakeValue( node->GetValue(), newText ) );

    m_model->ChangeText( path, newText );
    m_modified = true;
    UpdatePathEditor();

    TPropertyChange change;
    change.path = path;
    change.oldValue = oldText;
    change.newValue = newText;
    TPropertyChangedEventData eventData( change );
    NotifyObservers( PropertyChangedEvent, &eventData );

    // the grid shows a value the user changed like the widgets of the other forms: a change by the user
    CGridView* grid = GetGrid();
    if ( byUser && GUCEF_NULL != grid )
    {
        const CString propertyName = "Value:" + path;
        grid->ChangeValueByUser( propertyName, newText );
        grid->ResetProperty( propertyName );
    }
    if ( m_liveEditing && GUCEF_NULL != m_source )
        WriteToSource();
    else
        ShowButtons();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::SetGroupExpanded( const CString& path, bool expanded )
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->FindRowByPath( path );
    if ( GUCEF_NULL == entry || !entry->isGroup )
        return false;
    if ( entry->expanded == expanded )
        return true;
    if ( expanded )
        m_collapsed.erase( path );
    else
        m_collapsed.insert( path );
    RebuildRows();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::IsGroupExpanded( const CString& path ) const
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->FindRowByPath( path );
    return GUCEF_NULL != entry && entry->isGroup && entry->expanded;
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::ToggleGroup( Int32 row )
{GUCEF_TRACE;

    const SPropertyRow* entry = m_model->GetShownRow( (UInt32) row );
    if ( GUCEF_NULL == entry || !entry->isGroup )
        return false;
    return SetGroupExpanded( CString( entry->path ), !entry->expanded );
}

/*-------------------------------------------------------------------------*/

bool
CPropertyInspectorForm::SelectPath( const CString& path )
{GUCEF_TRACE;

    const Int32 row = m_model->FindShownRow( path );
    CGridView* grid = GetGrid();
    if ( row < 0 || GUCEF_NULL == grid )
        return false;
    const SPropertyRow* entry = m_model->GetShownRow( (UInt32) row );
    grid->SetCursor( row, GUCEF_NULL != entry && entry->isGroup ? 0 : 1 );
    UpdatePathEditor();
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CPropertyInspectorForm::GetSelectedPath( void ) const
{GUCEF_TRACE;

    CGridView* grid = const_cast< CPropertyInspectorForm* >( this )->GetGrid();
    if ( GUCEF_NULL == grid || grid->GetCursorRow() < 0 )
        return CString::Empty;
    return GetRowPath( (UInt32) grid->GetCursorRow() );
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::UpdatePathEditor( void )
{GUCEF_TRACE;

    CWidget* strip = GetWidget( "PathEditor" );
    if ( GUCEF_NULL == strip )
        return;
    const CString path = GetSelectedPath();
    const SPropertyRow* entry = path.IsNULLOrEmpty() ? GUCEF_NULL : m_model->FindRowByPath( path );
    const bool isPath = GUCEF_NULL != entry && !entry->isGroup && entry->hasDescriptor && !entry->descriptor.readOnly &&
                        TDescriptor::VALUETYPE_PATH == entry->descriptor.type && GUCEF_NULL != m_pathField;
    strip->SetBoolProperty( EWidgetProperty_Visible, isPath );
    if ( !isPath )
        return;
    CWidget* label = GetWidget( "PathLabel" );
    if ( GUCEF_NULL != label )
        label->SetProperty( EWidgetProperty_Text, entry->label );
    if ( m_pathField->GetPath() != entry->text )
        m_pathField->SetPath( entry->text );
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnApplyActivated( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Apply();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnRevertActivated( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Revert();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnGridItemClicked( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    CGridView::TGridItemClickedEventData* clicked = static_cast< CGridView::TGridItemClickedEventData* >( eventData );
    ToggleGroup( (Int32) clicked->GetData().row );
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnGridKeyDown( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CKeyEventData* key = static_cast< CKeyEventData* >( eventData );
    CGridView* grid = GetGrid();
    if ( GUCEF_NULL == key || key->handled || GUCEF_NULL == grid || EVENTPHASE_TARGET != key->phase || grid->IsEditing() )
        return;
    const SPropertyRow* entry = m_model->GetShownRow( (UInt32) ( grid->GetCursorRow() < 0 ? 0x7fffffff : grid->GetCursorRow() ) );
    if ( GUCEF_NULL == entry || !entry->isGroup )
        return;
    const CString path = entry->path;
    const bool expanded = entry->expanded;
    if ( INPUT::KEYCODE_RETURN == key->keyCode || INPUT::KEYCODE_SPACE == key->keyCode )
        SetGroupExpanded( path, !expanded );
    else
    if ( INPUT::KEYCODE_RIGHT == key->keyCode && !expanded )
        SetGroupExpanded( path, true );
    else
    if ( INPUT::KEYCODE_LEFT == key->keyCode && expanded )
        SetGroupExpanded( path, false );
    else
        return;
    key->handled = true;
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnGridMouseMove( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CPointerEventData* pointer = static_cast< CPointerEventData* >( eventData );
    CGridView* grid = GetGrid();
    if ( GUCEF_NULL == pointer || GUCEF_NULL == grid )
        return;

    // the tooltip of the descriptor of the row under the pointer
    const Int32 row = grid->GetRowAt( pointer->localY );
    const SPropertyRow* entry = row >= 0 ? m_model->GetShownRow( (UInt32) row ) : GUCEF_NULL;
    const CString tooltip = GUCEF_NULL != entry && entry->hasDescriptor ? entry->descriptor.tooltip : CString::Empty;
    if ( grid->GetProperty( EWidgetProperty_Tooltip ) != tooltip )
        grid->SetProperty( EWidgetProperty_Tooltip, tooltip );
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnGridCursorChanged( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdatePathEditor();
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnPathChosen( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CString path = GetSelectedPath();
    if ( !path.IsNULLOrEmpty() && GUCEF_NULL != m_pathField )
        ChangeValue( path, m_pathField->GetPath(), GUCEF_NULL, true );
}

/*-------------------------------------------------------------------------*/

void
CPropertyInspectorForm::OnPathEntered( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    OnPathChosen( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CString&
CPropertyInspectorForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CPropertyInspectorForm";
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
