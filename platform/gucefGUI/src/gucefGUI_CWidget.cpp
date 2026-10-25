/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#include "gucefGUI_CWidget.h"

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

const CORE::CEvent CWidget::PropertiesLoadedEvent = "GUCEF::GUI::CWidget::PropertiesLoadedEvent";
const CORE::CEvent CWidget::PropertiesSavedEvent = "GUCEF::GUI::CWidget::PropertiesSavedEvent";
const CORE::CEvent CWidget::PropertyChangedEvent = "GUCEF::GUI::CWidget::PropertyChangedEvent";
const CORE::CEvent CWidget::MouseMoveEvent = "GUCEF::GUI::CWidget::MouseMoveEvent";
const CORE::CEvent CWidget::MouseDownEvent = "GUCEF::GUI::CWidget::MouseDownEvent";
const CORE::CEvent CWidget::MouseUpEvent = "GUCEF::GUI::CWidget::MouseUpEvent";
const CORE::CEvent CWidget::MouseClickEvent = "GUCEF::GUI::CWidget::MouseClickEvent";
const CORE::CEvent CWidget::MouseDoubleClickEvent = "GUCEF::GUI::CWidget::MouseDoubleClickEvent";
const CORE::CEvent CWidget::MouseTripleClickEvent = "GUCEF::GUI::CWidget::MouseTripleClickEvent";
const CORE::CEvent CWidget::MouseFocusReceivedEvent = "GUCEF::GUI::CWidget::MouseFocusReceivedEvent";
const CORE::CEvent CWidget::MouseFocusLostEvent = "GUCEF::GUI::CWidget::MouseFocusLostEvent";
const CORE::CEvent CWidget::MouseCaptureLostEvent = "GUCEF::GUI::CWidget::MouseCaptureLostEvent";
const CORE::CEvent CWidget::MouseWheelEvent = "GUCEF::GUI::CWidget::MouseWheelEvent";
const CORE::CEvent CWidget::KeyDownEvent = "GUCEF::GUI::CWidget::KeyDownEvent";
const CORE::CEvent CWidget::KeyUpEvent = "GUCEF::GUI::CWidget::KeyUpEvent";
const CORE::CEvent CWidget::TextInputEvent = "GUCEF::GUI::CWidget::TextInputEvent";
const CORE::CEvent CWidget::KeyboardFocusReceivedEvent = "GUCEF::GUI::CWidget::KeyboardFocusReceivedEvent";
const CORE::CEvent CWidget::KeyboardFocusLostEvent = "GUCEF::GUI::CWidget::KeyboardFocusLostEvent";
const CORE::CEvent CWidget::NavigationEvent = "GUCEF::GUI::CWidget::NavigationEvent";
const CORE::CEvent CWidget::ActivatedEvent = "GUCEF::GUI::CWidget::ActivatedEvent";
const CORE::CEvent CWidget::ValueChangedEvent = "GUCEF::GUI::CWidget::ValueChangedEvent";
const CORE::CEvent CWidget::ItemActivatedEvent = "GUCEF::GUI::CWidget::ItemActivatedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The number of bits set in a word */
UInt32
CountBits( UInt32 word )
{
    word = word - ( ( word >> 1 ) & 0x55555555 );
    word = ( word & 0x33333333 ) + ( ( word >> 2 ) & 0x33333333 );
    return ( ( ( word + ( word >> 4 ) ) & 0x0F0F0F0F ) * 0x01010101 ) >> 24;
}

/** Escapes a value so it fits on one line of a properties file */
CString
EscapeLine( const CString& value )
{
    CString result;
    TCodePointVector codePoints;
    DecodeUtf8( value, codePoints );
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        UInt32 cp = codePoints[ i ];
        if ( '\\' == cp )
            result += "\\\\";
        else if ( '\n' == cp )
            result += "\\n";
        else
            result.Append( cp );
    }
    return result;
}

CString
UnescapeLine( const CString& value )
{
    CString result;
    TCodePointVector codePoints;
    DecodeUtf8( value, codePoints );
    for ( UInt32 i=0; i<(UInt32)codePoints.size(); ++i )
    {
        UInt32 cp = codePoints[ i ];
        if ( '\\' == cp && i+1 < (UInt32)codePoints.size() )
        {
            ++i;
            if ( 'n' == codePoints[ i ] )
                result += '\n';
            else
                result.Append( codePoints[ i ] );
        }
        else
            result.Append( cp );
    }
    return result;
}

/** Sets the local coordinates of pointer like event data for the widget that is notified */
void
SetLocalCoordinates( CRoutedEventData& eventData, const CWidget& widget )
{
    eventData.SetLocalOrigin( widget.GetRect().left, widget.GetRect().top );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CWidget::RegisterEvents( void )
{GUCEF_TRACE;

    PropertiesLoadedEvent.Initialize();
    PropertiesSavedEvent.Initialize();
    PropertyChangedEvent.Initialize();
    MouseMoveEvent.Initialize();
    MouseDownEvent.Initialize();
    MouseUpEvent.Initialize();
    MouseClickEvent.Initialize();
    MouseDoubleClickEvent.Initialize();
    MouseTripleClickEvent.Initialize();
    MouseFocusReceivedEvent.Initialize();
    MouseFocusLostEvent.Initialize();
    MouseCaptureLostEvent.Initialize();
    MouseWheelEvent.Initialize();
    KeyDownEvent.Initialize();
    KeyUpEvent.Initialize();
    TextInputEvent.Initialize();
    KeyboardFocusReceivedEvent.Initialize();
    KeyboardFocusLostEvent.Initialize();
    NavigationEvent.Initialize();
    ActivatedEvent.Initialize();
    ValueChangedEvent.Initialize();
    ItemActivatedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWidget::CWidget( void )
    : CORE::CObservingNotifier()
    , m_widgetTypeName( "Widget" )
    , m_name()
    , m_impPtr( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_handler( GUCEF_NULL )
    , m_parent( GUCEF_NULL )
    , m_children()
    , m_explicitMask()
    , m_explicit()
    , m_customProperties()
    , m_dirtyMask()
    , m_dirtyCustomProperties()
    , m_resolvedType( GUCEF_NULL )
    , m_resolvedGeneration( 0 )
    , m_applierOverrides()
    , m_rect()
    , m_state( 0 )
    , m_stateDirty( false )
    , m_parentDirty( false )
    , m_childOrderDirty( false )
    , m_rectDirty( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWidget::CWidget( const CString& widgetTypeName )
    : CORE::CObservingNotifier()
    , m_widgetTypeName( widgetTypeName )
    , m_name()
    , m_impPtr( GUCEF_NULL )
    , m_context( GUCEF_NULL )
    , m_handler( GUCEF_NULL )
    , m_parent( GUCEF_NULL )
    , m_children()
    , m_explicitMask()
    , m_explicit()
    , m_customProperties()
    , m_dirtyMask()
    , m_dirtyCustomProperties()
    , m_resolvedType( GUCEF_NULL )
    , m_resolvedGeneration( 0 )
    , m_applierOverrides()
    , m_rect()
    , m_state( 0 )
    , m_stateDirty( false )
    , m_parentDirty( false )
    , m_childOrderDirty( false )
    , m_rectDirty( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWidget::~CWidget()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_context )
        m_context->OnWidgetDestroyed( this );

    // the context destroys children before parents; for widgets made without a context the
    // children are only cut loose, they belong to whoever made them
    for ( UInt32 i=0; i<(UInt32)m_children.size(); ++i )
        m_children[ i ]->m_parent = GUCEF_NULL;
    m_children.clear();
    DetachFromParent();
}

/*-------------------------------------------------------------------------*/

void
CWidget::OnChildrenChanged( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::DetachFromParent( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_parent )
        return;
    TWidgetVector& siblings = m_parent->m_children;
    for ( TWidgetVector::iterator i=siblings.begin(); i!=siblings.end(); ++i )
    {
        if ( (*i) == this )
        {
            siblings.erase( i );
            break;
        }
    }
    m_parent = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetVisibility( const bool isVisible )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Visible, isVisible );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsVisible( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Visible );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ShowModal( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_parent && GUCEF_NULL != m_context )
    {
        SetVisibility( true );
        return m_context->ShowRoot( this, true );
    }
    return SetVisibility( true );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetEnabled( const bool isEnabled )
{GUCEF_TRACE;

    return SetBoolProperty( EWidgetProperty_Enabled, isEnabled );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsEnabled( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Enabled );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetPosition( const CUVector& newPos )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Position, newPos.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetPosition( CUVector& pos ) const
{GUCEF_TRACE;

    return CUVector::FromString( GetPropertyRef( EWidgetProperty_Position ), pos );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetSize( const CUVector& newSize )
{GUCEF_TRACE;

    return SetProperty( EWidgetProperty_Size, newSize.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetSize( CUVector& size ) const
{GUCEF_TRACE;

    const CString& value = GetPropertyRef( EWidgetProperty_Size );
    if ( value.IsNULLOrEmpty() )
    {
        // no size set: the natural size
        Int32 w = 0, h = 0;
        GetNaturalSize( w, h );
        size = CUVector( CUDim( 0.0f, w ), CUDim( 0.0f, h ) );
        return true;
    }
    return CUVector::FromString( value, size );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetWidth( const CUDim& newWidth )
{GUCEF_TRACE;

    CUVector size;
    GetSize( size );
    size.SetX( newWidth );
    return SetSize( size );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetWidth( CUDim& width ) const
{GUCEF_TRACE;

    CUVector size;
    if ( !GetSize( size ) )
        return false;
    width = size.GetX();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetHeight( const CUDim& newHeight )
{GUCEF_TRACE;

    CUVector size;
    GetSize( size );
    size.SetY( newHeight );
    return SetSize( size );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetHeight( CUDim& height ) const
{GUCEF_TRACE;

    CUVector size;
    if ( !GetSize( size ) )
        return false;
    height = size.GetY();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetParentWidget( CWidget* parentWidget )
{GUCEF_TRACE;

    if ( parentWidget == m_parent )
        return true;
    if ( parentWidget == this )
        return false;
    if ( GUCEF_NULL != parentWidget && GUCEF_NULL != m_context && parentWidget->m_context != m_context )
        return false;

    // no cycles: the new parent may not be below this widget
    for ( CWidget* p=parentWidget; GUCEF_NULL != p; p=p->m_parent )
        if ( p == this )
            return false;

    CWidget* oldParent = m_parent;
    DetachFromParent();
    if ( GUCEF_NULL != parentWidget )
    {
        parentWidget->m_children.push_back( this );
        m_parent = parentWidget;
    }
    if ( GUCEF_NULL != oldParent )
        oldParent->OnChildrenChanged();
    if ( GUCEF_NULL != parentWidget )
        parentWidget->OnChildrenChanged();
    m_parentDirty = true;
    if ( GUCEF_NULL != m_context )
    {
        m_context->OnWidgetParentChanged( this, oldParent );
        m_context->OnWidgetDirty( this );
    }

    // in its new tree an option finds its check box, a check box the options that depend on it
    if ( GUCEF_NULL != parentWidget )
        ApplyEnabledByLinksOfSubtree();
    return true;
}

/*-------------------------------------------------------------------------*/

CWidget*
CWidget::FindInTree( const CString& widgetName ) const
{GUCEF_TRACE;

    if ( widgetName.IsNULLOrEmpty() )
        return GUCEF_NULL;
    const CWidget* root = this;
    while ( GUCEF_NULL != root->m_parent )
        root = root->m_parent;
    if ( root->m_name == widgetName )
        return const_cast< CWidget* >( root );
    return root->FindWidget( widgetName );
}

/*-------------------------------------------------------------------------*/

void
CWidget::ApplyEnabledBy( void )
{GUCEF_TRACE;

    CString source = GetProperty( EWidgetProperty_EnabledBy );
    if ( source.IsNULLOrEmpty() )
        return;
    CWidget* checkbox = FindInTree( source );
    if ( GUCEF_NULL != checkbox )
        SetEnabled( checkbox->GetBoolProperty( EWidgetProperty_Checked ) );
}

/*-------------------------------------------------------------------------*/

void
CWidget::UpdateEnabledByDependents( void )
{GUCEF_TRACE;

    if ( m_name.IsNULLOrEmpty() )
        return;
    const CWidget* root = this;
    while ( GUCEF_NULL != root->m_parent )
        root = root->m_parent;

    const bool checked = GetBoolProperty( EWidgetProperty_Checked );
    TWidgetVector pending;
    pending.push_back( const_cast< CWidget* >( root ) );
    for ( UInt32 i=0; i<(UInt32)pending.size(); ++i )
    {
        CWidget* candidate = pending[ i ];
        if ( candidate != this && candidate->GetPropertyRef( EWidgetProperty_EnabledBy ) == m_name )
            candidate->SetEnabled( checked );
        pending.insert( pending.end(), candidate->m_children.begin(), candidate->m_children.end() );
    }
}

/*-------------------------------------------------------------------------*/

void
CWidget::ApplyEnabledByLinksOfSubtree( void )
{GUCEF_TRACE;

    // only widgets with EnabledBy or a Checked look further, the rest of the tree is not walked for them
    TWidgetVector pending;
    pending.push_back( this );
    for ( UInt32 i=0; i<(UInt32)pending.size(); ++i )
    {
        CWidget* widget = pending[ i ];
        if ( !widget->GetPropertyRef( EWidgetProperty_EnabledBy ).IsNULLOrEmpty() )
            widget->ApplyEnabledBy();
        if ( GUCEF_NULL != widget->GetPropertyDescriptor( EWidgetProperty_Checked ) )
            widget->UpdateEnabledByDependents();
        pending.insert( pending.end(), widget->m_children.begin(), widget->m_children.end() );
    }
}

/*-------------------------------------------------------------------------*/

CWidget*
CWidget::GetParentWidget( void ) const
{GUCEF_TRACE;

    return m_parent;
}

/*-------------------------------------------------------------------------*/

CWidget*
CWidget::GetChildWidget( const CString& widgetName ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_children.size(); ++i )
        if ( m_children[ i ]->m_name == widgetName )
            return m_children[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetChildWidgetSet( TWidgetSet& childSet ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_children.size(); ++i )
        childSet.insert( m_children[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

const CWidget::TWidgetVector&
CWidget::GetChildren( void ) const
{GUCEF_TRACE;

    return m_children;
}

/*-------------------------------------------------------------------------*/

CWidget*
CWidget::FindWidget( const CString& nameOrPath ) const
{GUCEF_TRACE;

    if ( nameOrPath.IsNULLOrEmpty() )
        return GUCEF_NULL;

    if ( nameOrPath.HasChar( '/' ) >= 0 )
    {
        CString::StringVector parts = nameOrPath.ParseElements( '/', false );
        const CWidget* current = this;
        for ( UInt32 i=0; i<(UInt32)parts.size() && GUCEF_NULL != current; ++i )
            current = current->GetChildWidget( parts[ i ] );
        return const_cast< CWidget* >( current == this ? GUCEF_NULL : current );
    }

    for ( UInt32 i=0; i<(UInt32)m_children.size(); ++i )
    {
        CWidget* child = m_children[ i ];
        if ( child->m_name == nameOrPath )
            return child;
        CWidget* found = child->FindWidget( nameOrPath );
        if ( GUCEF_NULL != found )
            return found;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CString
CWidget::GetNamePath( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_parent )
        return m_name;
    return m_parent->GetNamePath() + '/' + m_name;
}

/*-------------------------------------------------------------------------*/

CString
CWidget::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWidget::SetName( const CString& name )
{GUCEF_TRACE;

    m_name = name;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

CWidget::SPropertyMask::SPropertyMask( void )
{
    for ( UInt32 i=0; i<(UInt32)PROPERTYMASKWORDS; ++i )
        words[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SPropertyMask::Test( EWidgetProperty property ) const
{
    return 0 != ( words[ (UInt32) property >> 5 ] & ( (UInt32) 1 << ( (UInt32) property & 31 ) ) );
}

/*-------------------------------------------------------------------------*/

void
CWidget::SPropertyMask::Set( EWidgetProperty property )
{
    words[ (UInt32) property >> 5 ] |= ( (UInt32) 1 << ( (UInt32) property & 31 ) );
}

/*-------------------------------------------------------------------------*/

void
CWidget::SPropertyMask::Reset( EWidgetProperty property )
{
    words[ (UInt32) property >> 5 ] &= ~( (UInt32) 1 << ( (UInt32) property & 31 ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SPropertyMask::IsEmpty( void ) const
{
    for ( UInt32 i=0; i<(UInt32)PROPERTYMASKWORDS; ++i )
        if ( 0 != words[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CWidget::SPropertyMask::CountBelow( EWidgetProperty property ) const
{
    UInt32 count = 0;
    const UInt32 word = (UInt32) property >> 5;
    for ( UInt32 i=0; i<word; ++i )
        count += CountBits( words[ i ] );
    count += CountBits( words[ word ] & ( ( (UInt32) 1 << ( (UInt32) property & 31 ) ) - 1 ) );
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CWidget::SPropertyMask::NextSet( UInt32 from ) const
{
    while ( from < (UInt32) EWidgetProperty_Count )
    {
        const UInt32 word = from >> 5;
        UInt32 bits = words[ word ] >> ( from & 31 );
        if ( 0 == bits )
        {
            from = ( word + 1 ) << 5;
            continue;
        }
        while ( 0 == ( bits & 1 ) )
        {
            bits >>= 1;
            ++from;
        }
        return from;
    }
    return (UInt32) EWidgetProperty_Count;
}

/*-------------------------------------------------------------------------*/

CWidget::SExplicitProperty::SExplicitProperty( EWidgetProperty id )
    : property( id )
    , value()
{
}

/*-------------------------------------------------------------------------*/

const CResolvedWidgetType*
CWidget::GetResolvedType( void ) const
{
    // the registry changes rarely: until it does the answer is the pointer held
    CWidgetTypeRegistry& registry = CGuiGlobal::Instance()->GetWidgetTypeRegistry();
    if ( GUCEF_NULL != m_resolvedType && m_resolvedGeneration == registry.GetGeneration() )
        return m_resolvedType;
    UInt32 generation = registry.GetGeneration();
    m_resolvedType = registry.ResolveType( m_widgetTypeName );
    m_resolvedGeneration = generation;
    return m_resolvedType;
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CWidget::GetPropertyDescriptor( EWidgetProperty property ) const
{GUCEF_TRACE;

    const CResolvedWidgetType* type = GetResolvedType();
    return GUCEF_NULL == type ? GUCEF_NULL : type->FindProperty( property );
}

/*-------------------------------------------------------------------------*/

const CPropertyDescriptor*
CWidget::GetPropertyDescriptor( const CString& name ) const
{GUCEF_TRACE;

    const CResolvedWidgetType* type = GetResolvedType();
    return GUCEF_NULL == type ? GUCEF_NULL : type->FindProperty( name );
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue*
CWidget::FindExplicitValue( EWidgetProperty property, const CString& customName ) const
{
    if ( EWidgetProperty_Unknown == property )
    {
        TCustomPropertyMap::const_iterator i = m_customProperties.find( customName );
        return i == m_customProperties.end() ? GUCEF_NULL : &(*i).second;
    }
    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count || !m_explicitMask.Test( property ) )
        return GUCEF_NULL;
    return &m_explicit[ m_explicitMask.CountBelow( property ) ].value;
}

/*-------------------------------------------------------------------------*/

CWidgetPropertyValue*
CWidget::FindExplicitValue( EWidgetProperty property, const CString& customName )
{
    return const_cast< CWidgetPropertyValue* >( static_cast< const CWidget* >( this )->FindExplicitValue( property, customName ) );
}

/*-------------------------------------------------------------------------*/

const CWidgetPropertyValue&
CWidget::ResolveValue( EWidgetProperty property ) const
{
    static const CWidgetPropertyValue emptyValue;
    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return emptyValue;

    if ( m_explicitMask.Test( property ) )
        return m_explicit[ m_explicitMask.CountBelow( property ) ].value;

    // Visible, Enabled and Checked are the state a theme selects on, not something a theme sets
    if ( GUCEF_NULL != m_context && !IsWidgetStateProperty( property ) )
    {
        const CWidgetPropertyValue* themeValue = m_context->FindThemeValue( *this, property );
        if ( GUCEF_NULL != themeValue )
            return *themeValue;
    }

    const CResolvedWidgetType* type = GetResolvedType();
    if ( GUCEF_NULL != type )
    {
        const CWidgetPropertyValue* defaultValue = type->FindDefault( property );
        if ( GUCEF_NULL != defaultValue )
            return *defaultValue;
    }
    return emptyValue;
}

/*-------------------------------------------------------------------------*/

const CString&
CWidget::ResolveCustomText( const CString& name ) const
{
    TCustomPropertyMap::const_iterator i = m_customProperties.find( name );
    if ( i != m_customProperties.end() )
        return (*i).second.GetText();

    if ( GUCEF_NULL != m_context )
    {
        const CWidgetPropertyValue* themeValue = m_context->FindThemeValue( *this, name );
        if ( GUCEF_NULL != themeValue )
            return themeValue->GetText();
    }

    const CPropertyDescriptor* descriptor = GetPropertyDescriptor( name );
    if ( GUCEF_NULL != descriptor )
        return descriptor->defaultValue;
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetPropertyImpl( EWidgetProperty property, const CString& customName, const CString& value )
{GUCEF_TRACE;

    const bool isCustom = EWidgetProperty_Unknown == property;
    if ( isCustom && customName.IsNULLOrEmpty() )
        return false;
    const CString& name = isCustom ? customName : WidgetPropertyToString( property );

    CString normalized;
    const CString* newText = &value;
    const CResolvedWidgetType* type = GetResolvedType();
    const CPropertyDescriptor* descriptor = GUCEF_NULL == type ? GUCEF_NULL : ( isCustom ? type->FindProperty( customName ) : type->FindProperty( property ) );
    if ( GUCEF_NULL != descriptor && !value.IsNULLOrEmpty() )
    {
        if ( !descriptor->Validate( value, normalized ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Widget \"" << GetNamePath() << "\" (" << m_widgetTypeName << "): the value \"" << value
                               << "\" does not fit property \"" << name << "\" of type " << CPropertyDescriptor::ValueTypeToString( descriptor->valueType ) );
            return false;
        }
        newText = &normalized;
    }

    CWidgetPropertyValue* existing = FindExplicitValue( property, customName );
    if ( GUCEF_NULL != existing && existing->GetText() == *newText )
        return true;

    CString oldValue = isCustom ? ResolveCustomText( customName ) : ResolveValue( property ).GetText();
    if ( GUCEF_NULL == existing )
    {
        if ( isCustom )
            existing = &m_customProperties[ customName ];
        else
        {
            UInt32 place = m_explicitMask.CountBelow( property );
            m_explicit.insert( m_explicit.begin() + place, SExplicitProperty( property ) );
            m_explicitMask.Set( property );
            existing = &m_explicit[ place ].value;
        }
    }
    existing->SetText( *newText );
    CString newValue = isCustom ? ResolveCustomText( customName ) : ResolveValue( property ).GetText();

    if ( isCustom )
        MarkDirty( customName );
    else
        MarkDirty( property );
    if ( oldValue != newValue )
    {
        if ( GUCEF_NULL != descriptor && descriptor->HasFlag( CPropertyDescriptor::FLAG_AFFECTSLAYOUT ) && GUCEF_NULL != m_context )
            m_context->InvalidateLayout();
        if ( EWidgetProperty_Visible == property && GUCEF_NULL != m_context )
            m_context->OnWidgetVisibilityChanged( this );
        if ( EWidgetProperty_Enabled == property || EWidgetProperty_Checked == property )
        {
            m_stateDirty = true;
            if ( GUCEF_NULL != m_context )
                m_context->MarkThemeDependentDirty( *this );
        }

        if ( isCustom )
            OnCustomPropertyChanged( customName, oldValue, newValue );
        else
            OnPropertyChanged( property, oldValue, newValue );

        // options that depend on a check box follow it
        if ( EWidgetProperty_Checked == property )
            UpdateEnabledByDependents();
        else
        if ( EWidgetProperty_EnabledBy == property )
            ApplyEnabledBy();

        if ( isCustom )
        {
            CPropertyChangedEventData eventData( customName );
            NotifyObservers( PropertyChangedEvent, &eventData );
        }
        else
        {
            CPropertyChangedEventData eventData( property );
            NotifyObservers( PropertyChangedEvent, &eventData );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetProperty( EWidgetProperty property, const CString& value )
{GUCEF_TRACE;

    if ( EWidgetProperty_Unknown == property || (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return false;
    return SetPropertyImpl( property, CString::Empty, value );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetProperty( const CString& name, const CString& value )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    return SetPropertyImpl( StringToWidgetProperty( name ), name, value );
}

/*-------------------------------------------------------------------------*/

const CString&
CWidget::GetPropertyRef( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).GetText();
}

/*-------------------------------------------------------------------------*/

const CString&
CWidget::GetPropertyRef( const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return ResolveValue( property ).GetText();
    return ResolveCustomText( name );
}

/*-------------------------------------------------------------------------*/

CString
CWidget::GetProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).GetText();
}

/*-------------------------------------------------------------------------*/

CString
CWidget::GetProperty( const CString& name ) const
{GUCEF_TRACE;

    return GetPropertyRef( name );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetExplicitProperty( EWidgetProperty property, CString& value ) const
{GUCEF_TRACE;

    const CWidgetPropertyValue* explicitValue = FindExplicitValue( property, CString::Empty );
    if ( GUCEF_NULL == explicitValue )
        return false;
    value = explicitValue->GetText();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetExplicitProperty( const CString& name, CString& value ) const
{GUCEF_TRACE;

    const CWidgetPropertyValue* explicitValue = FindExplicitValue( StringToWidgetProperty( name ), name );
    if ( GUCEF_NULL == explicitValue )
        return false;
    value = explicitValue->GetText();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsPropertySet( EWidgetProperty property ) const
{GUCEF_TRACE;

    return GUCEF_NULL != FindExplicitValue( property, CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsPropertySet( const CString& name ) const
{GUCEF_TRACE;

    return GUCEF_NULL != FindExplicitValue( StringToWidgetProperty( name ), name );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ResetPropertyImpl( EWidgetProperty property, const CString& customName )
{GUCEF_TRACE;

    const bool isCustom = EWidgetProperty_Unknown == property;
    if ( GUCEF_NULL == FindExplicitValue( property, customName ) )
        return true;

    CString oldValue = isCustom ? ResolveCustomText( customName ) : ResolveValue( property ).GetText();
    if ( isCustom )
        m_customProperties.erase( customName );
    else
    {
        m_explicit.erase( m_explicit.begin() + m_explicitMask.CountBelow( property ) );
        m_explicitMask.Reset( property );
    }
    CString newValue = isCustom ? ResolveCustomText( customName ) : ResolveValue( property ).GetText();
    if ( isCustom )
        MarkDirty( customName );
    else
        MarkDirty( property );
    if ( oldValue != newValue )
    {
        const CPropertyDescriptor* descriptor = isCustom ? GetPropertyDescriptor( customName ) : GetPropertyDescriptor( property );
        if ( GUCEF_NULL != descriptor && descriptor->HasFlag( CPropertyDescriptor::FLAG_AFFECTSLAYOUT ) && GUCEF_NULL != m_context )
            m_context->InvalidateLayout();
        if ( EWidgetProperty_Visible == property && GUCEF_NULL != m_context )
            m_context->OnWidgetVisibilityChanged( this );
        if ( isCustom )
        {
            OnCustomPropertyChanged( customName, oldValue, newValue );
            CPropertyChangedEventData eventData( customName );
            NotifyObservers( PropertyChangedEvent, &eventData );
        }
        else
        {
            OnPropertyChanged( property, oldValue, newValue );
            CPropertyChangedEventData eventData( property );
            NotifyObservers( PropertyChangedEvent, &eventData );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ResetProperty( EWidgetProperty property )
{GUCEF_TRACE;

    if ( EWidgetProperty_Unknown == property || (UInt32) property >= (UInt32) EWidgetProperty_Count )
        return true;
    return ResetPropertyImpl( property, CString::Empty );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ResetProperty( const CString& name )
{GUCEF_TRACE;

    return ResetPropertyImpl( StringToWidgetProperty( name ), name );
}

/*-------------------------------------------------------------------------*/

CWidget::TPropertyMap
CWidget::GetExplicitProperties( void ) const
{GUCEF_TRACE;

    TPropertyMap properties;
    for ( UInt32 i=0; i<(UInt32)m_explicit.size(); ++i )
        properties[ WidgetPropertyToString( m_explicit[ i ].property ) ] = m_explicit[ i ].value.GetText();
    for ( TCustomPropertyMap::const_iterator c=m_customProperties.begin(); c!=m_customProperties.end(); ++c )
        properties[ (*c).first ] = (*c).second.GetText();
    return properties;
}

/*-------------------------------------------------------------------------*/

CWidget::TStringVector
CWidget::GetPropertyNames( void ) const
{GUCEF_TRACE;

    TStringVector names;
    const CResolvedWidgetType* type = GetResolvedType();
    if ( GUCEF_NULL != type )
    {
        const CResolvedWidgetType::TDescriptorVector& descriptors = type->GetProperties();
        for ( UInt32 i=0; i<(UInt32)descriptors.size(); ++i )
            names.push_back( descriptors[ i ]->name );
    }

    // properties set on the widget the type does not know (newer versions, backend hints)
    TPropertyMap explicitProperties = GetExplicitProperties();
    TPropertyMap::const_iterator p = explicitProperties.begin();
    while ( p != explicitProperties.end() )
    {
        bool known = false;
        for ( UInt32 n=0; n<(UInt32)names.size(); ++n )
        {
            if ( names[ n ] == (*p).first )
            {
                known = true;
                break;
            }
        }
        if ( !known )
            names.push_back( (*p).first );
        ++p;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetBoolProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).AsBool();
}

/*-------------------------------------------------------------------------*/

Int32
CWidget::GetIntProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).AsInt();
}

/*-------------------------------------------------------------------------*/

Float32
CWidget::GetFloatProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).AsFloat();
}

/*-------------------------------------------------------------------------*/

CColour
CWidget::GetColourProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return ResolveValue( property ).AsColour();
}

/*-------------------------------------------------------------------------*/

bool
CWidget::TryGetColourProperty( EWidgetProperty property, CColour& colour ) const
{GUCEF_TRACE;

    return ResolveValue( property ).TryAsColour( colour );
}

/*-------------------------------------------------------------------------*/

CUDim
CWidget::GetUDimProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    CUDim dim;
    CUDim::FromString( GetPropertyRef( property ), dim );
    return dim;
}

/*-------------------------------------------------------------------------*/

CUVector
CWidget::GetUVectorProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    CUVector vec;
    CUVector::FromString( GetPropertyRef( property ), vec );
    return vec;
}

/*-------------------------------------------------------------------------*/

CRect
CWidget::GetRectProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    CRect rect;
    CRect::FromString( GetPropertyRef( property ), rect );
    return rect;
}

/*-------------------------------------------------------------------------*/

CWidget::TStringVector
CWidget::GetStringListProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return SplitStringList( GetPropertyRef( property ) );
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< Int32 >
CWidget::GetIntListProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return SplitIntList( GetPropertyRef( property ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetBoolProperty( const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return ResolveValue( property ).AsBool();
    return CORE::StringToBool( ResolveCustomText( name ), false );
}

/*-------------------------------------------------------------------------*/

Int32
CWidget::GetIntProperty( const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return ResolveValue( property ).AsInt();
    return CORE::StringToInt32( ResolveCustomText( name ), 0 );
}

/*-------------------------------------------------------------------------*/

Float32
CWidget::GetFloatProperty( const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return ResolveValue( property ).AsFloat();
    return CORE::StringToFloat( ResolveCustomText( name ), 0.0f );
}

/*-------------------------------------------------------------------------*/

CColour
CWidget::GetColourProperty( const CString& name ) const
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        return ResolveValue( property ).AsColour();
    CColour colour( 0, 0, 0, 0 );
    CColour::FromString( ResolveCustomText( name ), colour );
    return colour;
}

/*-------------------------------------------------------------------------*/

CUDim
CWidget::GetUDimProperty( const CString& name ) const
{GUCEF_TRACE;

    CUDim dim;
    CUDim::FromString( GetPropertyRef( name ), dim );
    return dim;
}

/*-------------------------------------------------------------------------*/

CUVector
CWidget::GetUVectorProperty( const CString& name ) const
{GUCEF_TRACE;

    CUVector vec;
    CUVector::FromString( GetPropertyRef( name ), vec );
    return vec;
}

/*-------------------------------------------------------------------------*/

CRect
CWidget::GetRectProperty( const CString& name ) const
{GUCEF_TRACE;

    CRect rect;
    CRect::FromString( GetPropertyRef( name ), rect );
    return rect;
}

/*-------------------------------------------------------------------------*/

CWidget::TStringVector
CWidget::GetStringListProperty( const CString& name ) const
{GUCEF_TRACE;

    return SplitStringList( GetPropertyRef( name ) );
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< Int32 >
CWidget::GetIntListProperty( const CString& name ) const
{GUCEF_TRACE;

    return SplitIntList( GetPropertyRef( name ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetBoolProperty( EWidgetProperty property, bool value )
{GUCEF_TRACE;

    return SetProperty( property, value ? "true" : "false" );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetIntProperty( EWidgetProperty property, Int32 value )
{GUCEF_TRACE;

    return SetProperty( property, CORE::Int32ToString( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetFloatProperty( EWidgetProperty property, Float32 value )
{GUCEF_TRACE;

    return SetProperty( property, CORE::FloatToString( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetColourProperty( EWidgetProperty property, const CColour& value )
{GUCEF_TRACE;

    return SetProperty( property, value.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetStringListProperty( EWidgetProperty property, const TStringVector& value )
{GUCEF_TRACE;

    return SetProperty( property, JoinStringList( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetIntListProperty( EWidgetProperty property, const GUCEF::vector< Int32 >& value )
{GUCEF_TRACE;

    return SetProperty( property, JoinIntList( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetBoolProperty( const CString& name, bool value )
{GUCEF_TRACE;

    return SetProperty( name, value ? "true" : "false" );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetIntProperty( const CString& name, Int32 value )
{GUCEF_TRACE;

    return SetProperty( name, CORE::Int32ToString( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetFloatProperty( const CString& name, Float32 value )
{GUCEF_TRACE;

    return SetProperty( name, CORE::FloatToString( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetColourProperty( const CString& name, const CColour& value )
{GUCEF_TRACE;

    return SetProperty( name, value.ToString() );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetStringListProperty( const CString& name, const TStringVector& value )
{GUCEF_TRACE;

    return SetProperty( name, JoinStringList( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetIntListProperty( const CString& name, const GUCEF::vector< Int32 >& value )
{GUCEF_TRACE;

    return SetProperty( name, JoinIntList( value ) );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetPropertyValue( const CString& widgetProperty ,
                           const CString& propertyValue  )
{GUCEF_TRACE;

    return SetProperty( widgetProperty, propertyValue );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetPropertyValue( const CString& widgetProperty ,
                           CString& propertyValue        ) const
{GUCEF_TRACE;

    if ( !HasProperty( widgetProperty ) )
        return false;
    propertyValue = GetProperty( widgetProperty );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetProperties( const CORE::CValueList& list )
{GUCEF_TRACE;

    bool success = true;
    for ( UInt32 i=0; i<list.GetKeyCount(); ++i )
    {
        CString key = list.GetKey( i ).AsString();
        success = SetProperty( key, list.GetValue( key ).AsString() ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetProperties( CORE::CValueList& list ) const
{GUCEF_TRACE;

    TStringVector names = GetPropertyNames();
    for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
        list.Set( names[ i ], GetProperty( names[ i ] ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::HasProperty( const CString& widgetProperty ) const
{GUCEF_TRACE;

    return GUCEF_NULL != GetPropertyDescriptor( widgetProperty ) || IsPropertySet( widgetProperty );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::HasProperty( EWidgetProperty property ) const
{GUCEF_TRACE;

    return GUCEF_NULL != GetPropertyDescriptor( property ) || IsPropertySet( property );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::LoadProperties( CORE::CIOAccess& propertyStorage )
{GUCEF_TRACE;

    bool success = true;
    while ( !propertyStorage.Eof() )
    {
        CString line = propertyStorage.ReadLine();
        if ( line.IsNULLOrEmpty() )
            continue;
        Int32 separator = line.HasChar( '=' );
        if ( separator <= 0 )
            continue;
        CString name = line.SubstrFromRange( 0, (UInt32) separator );
        CString value = line.SubstrFromRange( (UInt32) separator + 1, line.Length() );
        success = SetProperty( name, UnescapeLine( value ) ) && success;
    }
    NotifyObservers( PropertiesLoadedEvent );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SaveProperties( CORE::CIOAccess& propertyStorage ) const
{GUCEF_TRACE;

    TPropertyMap explicitProperties = GetExplicitProperties();
    TPropertyMap::const_iterator i = explicitProperties.begin();
    while ( i != explicitProperties.end() )
    {
        CString line = (*i).first + '=' + EscapeLine( (*i).second ) + '\n';
        if ( line.ByteSize() - 1 != propertyStorage.Write( line.C_String(), 1, line.ByteSize() - 1 ) )
            return false;
        ++i;
    }
    NotifyObservers( PropertiesSavedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

const CRect&
CWidget::GetRect( void ) const
{GUCEF_TRACE;

    return m_rect;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::GetNaturalSize( Int32& width, Int32& height ) const
{GUCEF_TRACE;

    width = 0;
    height = 0;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsEffectivelyVisible( void ) const
{GUCEF_TRACE;

    for ( const CWidget* w=this; GUCEF_NULL != w; w=w->m_parent )
    {
        if ( !w->IsVisible() )
            return false;
        if ( GUCEF_NULL != w->m_parent && !w->m_parent->AreChildrenShown() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::AreChildrenShown( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsEffectivelyEnabled( void ) const
{GUCEF_TRACE;

    for ( const CWidget* w=this; GUCEF_NULL != w; w=w->m_parent )
        if ( !w->IsEnabled() )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CWidget::GetState( void ) const
{GUCEF_TRACE;

    UInt32 state = m_state;
    if ( !IsEffectivelyEnabled() )
        state |= STATE_DISABLED;
    if ( HasProperty( EWidgetProperty_Checked ) && GetBoolProperty( EWidgetProperty_Checked ) )
        state |= STATE_CHECKED;
    return state;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsHovered( void ) const
{GUCEF_TRACE;

    return 0 != ( m_state & STATE_HOVERED );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsPressed( void ) const
{GUCEF_TRACE;

    return 0 != ( m_state & STATE_PRESSED );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::HasFocus( void ) const
{GUCEF_TRACE;

    return 0 != ( m_state & STATE_FOCUSED );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SetFocus( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context )
        return false;
    return m_context->SetFocus( this );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsFocusable( void ) const
{GUCEF_TRACE;

    return GetBoolProperty( EWidgetProperty_Focusable ) && IsEffectivelyVisible() && IsEffectivelyEnabled();
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsActivatable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::BringToFront( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_parent )
    {
        if ( GUCEF_NULL != m_context )
            return m_context->ShowRoot( this, m_context->IsModalRoot( this ) );
        return false;
    }

    TWidgetVector& siblings = m_parent->m_children;
    for ( TWidgetVector::iterator i=siblings.begin(); i!=siblings.end(); ++i )
    {
        if ( (*i) == this )
        {
            siblings.erase( i );
            break;
        }
    }
    siblings.push_back( this );
    m_parent->m_childOrderDirty = true;
    if ( GUCEF_NULL != m_context )
        m_context->OnWidgetDirty( m_parent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::SendToBack( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_parent )
        return false;

    TWidgetVector& siblings = m_parent->m_children;
    for ( TWidgetVector::iterator i=siblings.begin(); i!=siblings.end(); ++i )
    {
        if ( (*i) == this )
        {
            siblings.erase( i );
            break;
        }
    }
    siblings.insert( siblings.begin(), this );
    m_parent->m_childOrderDirty = true;
    if ( GUCEF_NULL != m_context )
        m_context->OnWidgetDirty( m_parent );
    return true;
}

/*-------------------------------------------------------------------------*/

CGUIContext*
CWidget::GetContext( void ) const
{GUCEF_TRACE;

    return m_context;
}

/*-------------------------------------------------------------------------*/

CWidgetHandler*
CWidget::GetHandler( void ) const
{GUCEF_TRACE;

    return m_handler;
}

/*-------------------------------------------------------------------------*/

void*
CWidget::GetNativeHandle( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_handler )
        return GUCEF_NULL;
    return m_handler->GetNativeHandle( *this );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::RunCommand( const CString& command, const CORE::CValueList& arguments )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_handler )
        return false;
    if ( GUCEF_NULL != m_context )
        m_context->FlushChanges();
    return m_handler->RunCommand( *this, command, arguments );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::NotifyRouted( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    SetLocalCoordinates( eventData, *this );
    NotifyObservers( eventId, &eventData );
    return eventData.handled;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::DispatchEvent( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    eventData.source = this;
    eventData.handled = false;

    TWidgetVector ancestors;
    for ( CWidget* p=m_parent; GUCEF_NULL != p; p=p->m_parent )
        ancestors.push_back( p );

    eventData.phase = EVENTPHASE_TUNNEL;
    for ( Int32 i=(Int32)ancestors.size()-1; i>=0; --i )
        if ( ancestors[ i ]->NotifyRouted( eventId, eventData ) )
            return true;

    eventData.phase = EVENTPHASE_TARGET;
    if ( NotifyRouted( eventId, eventData ) )
        return true;

    eventData.phase = EVENTPHASE_BUBBLE;
    for ( UInt32 i=0; i<(UInt32)ancestors.size(); ++i )
        if ( ancestors[ i ]->NotifyRouted( eventId, eventData ) )
            return true;

    // back at the target, as the source of the default action
    SetLocalCoordinates( eventData, *this );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWidget::DispatchDirectEvent( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    eventData.source = this;
    eventData.handled = false;
    eventData.phase = EVENTPHASE_TARGET;
    NotifyRouted( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWidget::SetPropertyApplierOverride( const CString& name, CWidgetHandler::TPropertyApplier applier )
{GUCEF_TRACE;

    m_applierOverrides[ name ] = applier;
    MarkDirty( name );
}

/*-------------------------------------------------------------------------*/

void
CWidget::RemovePropertyApplierOverride( const CString& name )
{GUCEF_TRACE;

    m_applierOverrides.erase( name );
    MarkDirty( name );
}

/*-------------------------------------------------------------------------*/

void*
CWidget::GetImplementationPtr( void ) const
{GUCEF_TRACE;

    return m_impPtr;
}

/*-------------------------------------------------------------------------*/

void
CWidget::SetImplementationPtr( void* impPtr )
{GUCEF_TRACE;

    m_impPtr = impPtr;
}

/*-------------------------------------------------------------------------*/

const CString&
CWidget::GetType( void ) const
{GUCEF_TRACE;

    return m_widgetTypeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CWidget::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CWidget";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::IsRegisteredAs( const char* registeredTypeName ) const
{GUCEF_TRACE;

    return CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsA( m_widgetTypeName, registeredTypeName );
}

/*-------------------------------------------------------------------------*/

CScrollPanel*
CWidget::AsScrollPanel( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CScrollPanel*
CWidget::AsScrollPanel( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWindow*
CWidget::AsWindow( void )
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CWindow*
CWidget::AsWindow( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CScrollPanel*
CWidget::ToScrollPanel( CWidget* widget )
{GUCEF_TRACE;

    return GUCEF_NULL != widget ? widget->AsScrollPanel() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CScrollPanel*
CWidget::ToScrollPanel( const CWidget* widget )
{GUCEF_TRACE;

    return GUCEF_NULL != widget ? widget->AsScrollPanel() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CWindow*
CWidget::ToWindow( CWidget* widget )
{GUCEF_TRACE;

    return GUCEF_NULL != widget ? widget->AsWindow() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CWindow*
CWidget::ToWindow( const CWidget* widget )
{GUCEF_TRACE;

    return GUCEF_NULL != widget ? widget->AsWindow() : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CWidget::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::OnCustomPropertyChanged( const CString& name, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::OnDescendantFocusReceived( CWidget* descendant )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::AdjustSize( Int32& width, Int32& height, const CRect& area ) const
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWidget::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWidget::NeedsUpdate( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ChangeValueByUserImpl( EWidgetProperty property, const CString& customName, const CString& value )
{GUCEF_TRACE;

    const bool isCustom = EWidgetProperty_Unknown == property;
    CString oldValue = isCustom ? GetPropertyRef( customName ) : GetPropertyRef( property );
    if ( !( isCustom ? SetProperty( customName, value ) : SetProperty( property, value ) ) )
        return false;
    CString newValue = isCustom ? GetPropertyRef( customName ) : GetPropertyRef( property );
    if ( oldValue == newValue )
        return true;

    CValueChangedEventData eventData;
    eventData.property = property;
    eventData.customPropertyName = isCustom ? customName : CString::Empty;
    eventData.oldValue = oldValue;
    eventData.newValue = newValue;
    DispatchEvent( ValueChangedEvent, eventData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ChangeValueByUser( EWidgetProperty property, const CString& value )
{GUCEF_TRACE;

    if ( EWidgetProperty_Unknown == property )
        return false;
    return ChangeValueByUserImpl( property, CString::Empty, value );
}

/*-------------------------------------------------------------------------*/

bool
CWidget::ChangeValueByUser( const CString& name, const CString& value )
{GUCEF_TRACE;

    return ChangeValueByUserImpl( StringToWidgetProperty( name ), name, value );
}

/*-------------------------------------------------------------------------*/

void
CWidget::ActivateItem( Int32 index, const CString& item )
{GUCEF_TRACE;

    CItemEventData eventData;
    eventData.index = index;
    eventData.item = item;
    DispatchEvent( ItemActivatedEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CWidget::Touch( EWidgetProperty property )
{GUCEF_TRACE;

    MarkDirty( property );
}

/*-------------------------------------------------------------------------*/

void
CWidget::Touch( const CString& name )
{GUCEF_TRACE;

    MarkDirty( name );
}

/*-------------------------------------------------------------------------*/

void
CWidget::MarkDirty( EWidgetProperty property )
{GUCEF_TRACE;

    if ( (UInt32) property >= (UInt32) EWidgetProperty_Count || EWidgetProperty_Unknown == property )
        return;
    m_dirtyMask.Set( property );
    if ( GUCEF_NULL != m_context )
        m_context->OnWidgetDirty( this );
}

/*-------------------------------------------------------------------------*/

void
CWidget::MarkDirty( const CString& name )
{GUCEF_TRACE;

    EWidgetProperty property = StringToWidgetProperty( name );
    if ( EWidgetProperty_Unknown != property )
        m_dirtyMask.Set( property );
    else
        m_dirtyCustomProperties.insert( name );
    if ( GUCEF_NULL != m_context )
        m_context->OnWidgetDirty( this );
}

/*-------------------------------------------------------------------------*/

void
CWidget::MarkAllPropertiesDirty( void )
{GUCEF_TRACE;

    const CResolvedWidgetType* type = GetResolvedType();
    if ( GUCEF_NULL != type )
    {
        const CResolvedWidgetType::TDescriptorVector& descriptors = type->GetProperties();
        for ( UInt32 i=0; i<(UInt32)descriptors.size(); ++i )
        {
            if ( EWidgetProperty_Unknown != descriptors[ i ]->id )
                m_dirtyMask.Set( descriptors[ i ]->id );
            else
                m_dirtyCustomProperties.insert( descriptors[ i ]->name );
        }
    }
    for ( UInt32 i=0; i<(UInt32)m_explicit.size(); ++i )
        m_dirtyMask.Set( m_explicit[ i ].property );
    for ( TCustomPropertyMap::const_iterator c=m_customProperties.begin(); c!=m_customProperties.end(); ++c )
        m_dirtyCustomProperties.insert( (*c).first );
}

/*-------------------------------------------------------------------------*/

void
CWidget::SetStateFlag( UInt32 flag, bool on )
{GUCEF_TRACE;

    UInt32 newState = on ? ( m_state | flag ) : ( m_state & ~flag );
    if ( newState == m_state )
        return;
    m_state = newState;
    m_stateDirty = true;
    if ( GUCEF_NULL != m_context )
    {
        m_context->MarkThemeDependentDirty( *this );
        m_context->OnWidgetDirty( this );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
