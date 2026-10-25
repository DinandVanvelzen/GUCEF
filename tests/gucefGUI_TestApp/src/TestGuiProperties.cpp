/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#include "gucefGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** What the benchmarks add up, so no compiler removes the work */
volatile UInt32 g_sink = 0;

/** A page of widgets as a form has: rows of labels, buttons, check boxes, edit boxes and lists */
struct SBenchPage
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUI::CWidget::TWidgetVector widgets;

    SBenchPage( void ) : contextPtr(), context( GUCEF_NULL ), widgets() {}
};

bool
BuildBenchPage( SBenchPage& page )
{
    page.contextPtr = CreateNullContext();
    page.context = static_cast< GUI::CGUIContext* >( page.contextPtr.GetPointerAlways() );
    if ( GUCEF_NULL == page.context )
        return false;
    GUI::CGUIContext* context = page.context;
    context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
    CORE::CDataNode theme;
    GUI::BuildDarkTheme( theme, "Test" );
    context->SetTheme( theme );

    GUI::CWidget* root = context->CreateWidget( "Panel", "Page", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    page.widgets.push_back( root );

    static const char* const types[ 19 ] = { "Label", "Label", "Label", "Label", "Label", "Label", "Button", "Button", "Button", "PushButton",
                                             "Checkbox", "Checkbox", "Checkbox", "Editbox", "Editbox", "Slider", "ProgressBar", "Listbox", "Combobox" };
    for ( UInt32 row=0; row<10; ++row )
    {
        GUI::CWidget* panel = context->CreateWidget( "Panel", CORE::CString( "Row" ) + CORE::UInt32ToString( row ), root );
        panel->SetProperty( "Position", CORE::CString( "0,0 0," ) + CORE::UInt32ToString( row * 46 ) );
        panel->SetProperty( "Size", "1,0 0,44" );
        panel->SetProperty( "BackgroundColour", "30,32,40,255" );
        page.widgets.push_back( panel );
        for ( UInt32 c=0; c<19; ++c )
        {
            GUI::CWidget* widget = context->CreateWidget( types[ c ], CORE::CString( "W" ) + CORE::UInt32ToString( c ), panel );
            widget->SetProperty( "Position", CORE::CString( "0," ) + CORE::UInt32ToString( c * 33 ) + " 0,4" );
            widget->SetProperty( "Size", "0,32 0,36" );
            if ( c < 13 )
                widget->SetProperty( "Text", "Item" );
            page.widgets.push_back( widget );
        }
    }
    context->FlushChanges();
    return true;
}

/** Nanoseconds per operation for 'ticks' taken by 'operations' */
double
NanosPerOp( UInt64 ticks, UInt64 operations )
{
    double seconds = (double) ticks / (double) MT::PrecisionTimerResolution();
    return operations > 0 ? seconds * 1000000000.0 / (double) operations : 0.0;
}

void
ReportBench( const char* name, UInt64 ticks, UInt64 operations, UInt64 referenceTicks = 0 )
{
    if ( referenceTicks > 0 && ticks > 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PropertyBenchmark: " << name << ": " << (UInt64) NanosPerOp( ticks, operations ) << " ns per operation, " << operations << " operations, "
                   << ( referenceTicks * 100 / ticks ) << " percent of the speed of the way by name" );
    }
    else
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PropertyBenchmark: " << name << ": " << (UInt64) NanosPerOp( ticks, operations ) << " ns per operation, " << operations << " operations" );
    }
}

/** The constants of the 64 bit FNV-1a hash */
UInt64
FnvOffset( void )
{
    return ( (UInt64) 0xcbf29ce4 << 32 ) | (UInt64) 0x84222325;
}

UInt64
FnvPrime( void )
{
    return ( (UInt64) 0x100 << 32 ) | (UInt64) 0x1b3;
}

/** A hash of the pixels, so a frame can be compared with the frame of an earlier version of the code */
UInt64
HashPixels( const IMAGE::TPixelMapPtr& pixels )
{
    UInt64 hash = FnvOffset();
    if ( pixels.IsNULL() )
        return hash;
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() );
    const size_t size = (size_t) pixels->GetWidthInPixels() * pixels->GetHeightInPixels() * 4;
    for ( size_t i=0; i<size; ++i )
    {
        hash ^= data[ i ];
        hash *= FnvPrime();
    }
    return hash;
}

UInt64
HashText( UInt64 hash, const CORE::CString& text )
{
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        hash ^= (UInt8) text[ i ];
        hash *= FnvPrime();
    }
    hash ^= 0xff;
    hash *= FnvPrime();
    return hash;
}

/** A hash of the effective value of every property of the widget and its children, by name, as the string interface gives them */
UInt64
HashProperties( const GUI::CWidget& widget, UInt64 hash )
{
    hash = HashText( hash, widget.GetNamePath() );
    GUI::CWidget::TStringVector names = widget.GetPropertyNames();
    for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
    {
        hash = HashText( hash, names[ i ] );
        hash = HashText( hash, widget.GetProperty( names[ i ] ) );
    }
    const GUI::CWidget::TWidgetVector& children = widget.GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        hash = HashProperties( *children[ i ], hash );
    return hash;
}

UInt64
HashContextProperties( GUI::CGUIContext* context )
{
    UInt64 hash = FnvOffset();
    const GUI::CGUIContext::TWidgetVector& roots = context->GetRootWidgets();
    for ( UInt32 i=0; i<(UInt32)roots.size(); ++i )
        hash = HashProperties( *roots[ i ], hash );
    return hash;
}

/** A value as 16 lowercase hexadecimal digits */
CORE::CString
HexText( UInt64 value )
{
    static const char digits[] = "0123456789abcdef";
    CORE::CString text;
    for ( Int32 shift=60; shift>=0; shift-=4 )
        text += digits[ ( value >> shift ) & 0xf ];
    return text;
}

/** Draws the context and logs what it looks like and the values of the properties of its widgets, one line per scene: the lines of two versions of the code are equal when nothing changed */
void
ReportScene( GUI::CGUIContext* context, const char* name )
{
    IMAGE::TPixelMapPtr frame;
    context->RenderFrame( 16 );
    bool captured = context->CaptureFrame( frame );
    CORE::CString text = "no frame";
    if ( captured )
        text = "pixels " + HexText( HashPixels( frame ) ) + " properties " + HexText( HashContextProperties( context ) );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PropertyScene: " << name << ": " << text );
}

/** What the theme gives, found the way a theme was always read: every style of the tree in order, the state, the class, the name and the type of the widget against the attributes of the style, the best match wins, the last of equals */
UInt32
LegacyStateFlag( const CORE::CString& name )
{
    CORE::CString lower = name.Trim( true ).Trim( false ).Lowercase();
    if ( lower == "hover" || lower == "hovered" ) return GUI::CWidget::STATE_HOVERED;
    if ( lower == "pressed" ) return GUI::CWidget::STATE_PRESSED;
    if ( lower == "focused" || lower == "focus" ) return GUI::CWidget::STATE_FOCUSED;
    if ( lower == "disabled" ) return GUI::CWidget::STATE_DISABLED;
    if ( lower == "checked" ) return GUI::CWidget::STATE_CHECKED;
    return 0;
}

bool
LegacyStyleMatches( const CORE::CDataNode& style, const GUI::CWidget& widget, Int32& score )
{
    score = 0;
    CORE::CString type = style.GetAttributeValue( "type" ).AsString();
    if ( !type.IsNULLOrEmpty() )
    {
        if ( !GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().IsA( widget.GetType(), type ) )
            return false;
        score += 1;
    }
    CORE::CString styleClass = style.GetAttributeValue( "class" ).AsString();
    if ( !styleClass.IsNULLOrEmpty() )
    {
        CORE::CString classes;
        widget.GetExplicitProperty( "StyleClass", classes );
        bool found = false;
        CORE::CString::StringVector parts = classes.ParseElements( ' ', false );
        for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
            found = found || parts[ i ] == styleClass;
        if ( !found )
            return false;
        score += 100;
    }
    CORE::CString name = style.GetAttributeValue( "name" ).AsString();
    if ( !name.IsNULLOrEmpty() )
    {
        if ( widget.GetName() != name )
            return false;
        score += 10000;
    }
    CORE::CString states = style.GetAttributeValue( "state" ).AsString();
    if ( !states.IsNULLOrEmpty() )
    {
        UInt32 widgetState = widget.GetState();
        CORE::CString::StringVector parts = states.ParseElements( ',', false );
        for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
        {
            UInt32 flag = LegacyStateFlag( parts[ i ] );
            if ( 0 == flag || 0 == ( widgetState & flag ) )
                return false;
            score += 10;
        }
    }
    return true;
}

bool
LegacyThemeProperty( const CORE::CDataNode& theme, const GUI::CWidget& widget, const CORE::CString& name, CORE::CString& value )
{
    bool found = false;
    Int32 bestScore = -1;
    for ( CORE::CDataNode::const_iterator s=theme.ConstBegin(); s!=theme.ConstEnd(); ++s )
    {
        const CORE::CDataNode* style = (*s);
        if ( style->GetName() != "Style" )
            continue;
        bool setsIt = false;
        for ( CORE::CDataNode::const_iterator p=style->ConstBegin(); p!=style->ConstEnd(); ++p )
            if ( (*p)->GetName() == "Property" && (*p)->GetAttributeValue( "name" ).AsString() == name )
                setsIt = true;
        Int32 score = 0;
        if ( !setsIt || !LegacyStyleMatches( *style, widget, score ) || score < bestScore )
            continue;
        for ( CORE::CDataNode::const_iterator p=style->ConstBegin(); p!=style->ConstEnd(); ++p )
        {
            if ( (*p)->GetName() == "Property" && (*p)->GetAttributeValue( "name" ).AsString() == name )
            {
                value = (*p)->GetAttributeValue( "value" ).AsString();
                bestScore = score;
                found = true;
            }
        }
    }
    return found;
}

/** The value of a property as it was always worked out: set on the widget, the theme, the default of the descriptor */
CORE::CString
LegacyPropertyValue( const CORE::CDataNode& theme, const GUI::CWidget& widget, const CORE::CString& name )
{
    CORE::CString value;
    if ( widget.GetExplicitProperty( name, value ) )
        return value;
    if ( name != "Visible" && name != "Enabled" && name != "Checked" && LegacyThemeProperty( theme, widget, name, value ) )
        return value;
    const GUI::CPropertyDescriptor* descriptor = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().FindProperty( widget.GetType(), name );
    return GUCEF_NULL == descriptor ? CORE::CString::Empty : descriptor->defaultValue;
}

/** The stock theme and styles that use every part of the way a style is selected */
void
BuildTestTheme( CORE::CDataNode& theme )
{
    GUI::BuildDarkTheme( theme, "Test" );
    CORE::CDataNode* style = theme.AddChild( "Style" );
    style->SetAttribute( "type", "Widget" );
    style->SetAttribute( "class", "Primary" );
    style->SetAttribute( "state", "hover,focused" );
    CORE::CDataNode* property = style->AddChild( "Property" );
    property->SetAttribute( "name", "TextColour" );
    property->SetAttribute( "value", "1,2,3,255" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Tooltip" );
    property->SetAttribute( "value", "both states" );

    style = theme.AddChild( "Style" );
    style->SetAttribute( "name", "Target" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Opacity" );
    property->SetAttribute( "value", "0.5" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Opacity" );
    property->SetAttribute( "value", "0.75" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "ExoticHint" );
    property->SetAttribute( "value", "from the theme" );

    style = theme.AddChild( "Style" );
    style->SetAttribute( "type", "Button" );
    style->SetAttribute( "state", "no such state" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Tooltip" );
    property->SetAttribute( "value", "never" );

    style = theme.AddChild( "Style" );
    style->SetAttribute( "state", "disabled,checked" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "BackgroundColour" );
    property->SetAttribute( "value", "9,9,9,255" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Visible" );
    property->SetAttribute( "value", "false" );

    style = theme.AddChild( "Style" );
    style->SetAttribute( "class", "Primary" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "ExoticHint" );
    property->SetAttribute( "value", "primary" );
    property = style->AddChild( "Property" );
    property->SetAttribute( "name", "Clip" );
    property->SetAttribute( "value", "true" );
}

/** True when every standard property of the enumeration is described by some stock widget type and every property they describe has an id */
bool
EveryStandardPropertyIsDescribed( void )
{
    GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
    GUI::CWidgetTypeRegistry::TStringSet types = registry.GetTypeNames();
    GUCEF::set< UInt32 > described;
    for ( GUI::CWidgetTypeRegistry::TStringSet::const_iterator t=types.begin(); t!=types.end(); ++t )
    {
        GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = registry.GetProperties( *t );
        for ( UInt32 d=0; d<(UInt32)descriptors.size(); ++d )
            described.insert( (UInt32) GUI::StringToWidgetProperty( descriptors[ d ]->name ) );
    }
    for ( UInt32 id=1; id<(UInt32)GUI::EWidgetProperty_Count; ++id )
        if ( described.find( id ) == described.end() )
            return false;
    return described.find( (UInt32) GUI::EWidgetProperty_Unknown ) == described.end();
}

/** Keeps what the last event of a widget carried */
class CPropertyEventObserver : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CPropertyEventObserver > TEventCallback;

    UInt32 count;
    GUI::EWidgetProperty property;
    CORE::CString name;
    CORE::CString oldValue;
    CORE::CString newValue;

    /** 'id' is CWidget::PropertyChangedEvent or CWidget::ValueChangedEvent */
    CPropertyEventObserver( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), count( 0 ), property( GUI::EWidgetProperty_Unknown ), name(), oldValue(), newValue()
    {
        if ( GUI::CWidget::PropertyChangedEvent == id )
        {
            TEventCallback callback( this, &CPropertyEventObserver::OnPropertyChanged );
            SubscribeTo( notifier, id, callback );
        }
        else
        {
            TEventCallback callback( this, &CPropertyEventObserver::OnValueChanged );
            SubscribeTo( notifier, id, callback );
        }
    }

    virtual ~CPropertyEventObserver() { UnsubscribeAllFromObserver(); }

    void OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData )
    {
        const GUI::CPropertyChangedEventData* changed = static_cast< const GUI::CPropertyChangedEventData* >( eventData );
        ++count;
        property = changed->property;
        name = changed->GetPropertyName();
    }

    void OnValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData )
    {
        const GUI::CValueChangedEventData* changed = static_cast< const GUI::CValueChangedEventData* >( eventData );
        if ( changed->source != notifier )
            return;
        ++count;
        property = changed->property;
        name = changed->GetPropertyName();
        oldValue = changed->oldValue;
        newValue = changed->newValue;
    }
};

/** The order a handler gets the changes of the properties in */
GUCEF::vector< CORE::CString > g_applied;

bool
RecordApplied( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CORE::CString& value )
{
    g_applied.push_back( value );
    return true;
}

UInt32
BenchRepetitions( void )
{
    CORE::CString value = CORE::GetEnv( "GUCEF_TEST_BENCH_REPS" );
    if ( !value.IsNULLOrEmpty() )
        return CORE::StringToUInt32( value, 20 );
    return 20;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiPropertyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Properties" );

    GUCEF_TESTFW_TESTCASE( "Properties: Benchmark of property get, set, theme lookup, paint and frame capture on a page of 200 widgets, by name and by id (numbers in the log)" )
    try
    {
        SBenchPage page;
        ASSERT_TRUE( BuildBenchPage( page ) );
        ASSERT_TRUE( page.widgets.size() >= 200 );
        GUI::CGUIContext* context = page.context;
        const UInt32 reps = BenchRepetitions();
        const UInt32 widgetCount = (UInt32) page.widgets.size();

        // the properties the painters, the layout and the input read all the time
        static const char* const names[ 10 ] = { "Visible", "Enabled", "Position", "Size", "Clip", "InputTransparent", "Opacity", "BackgroundColour", "TextColour", "Font" };
        static const GUI::EWidgetProperty ids[ 10 ] = { GUI::EWidgetProperty_Visible, GUI::EWidgetProperty_Enabled, GUI::EWidgetProperty_Position, GUI::EWidgetProperty_Size, GUI::EWidgetProperty_Clip,
                                                        GUI::EWidgetProperty_InputTransparent, GUI::EWidgetProperty_Opacity, GUI::EWidgetProperty_BackgroundColour, GUI::EWidgetProperty_TextColour, GUI::EWidgetProperty_Font };

        UInt64 start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                for ( UInt32 n=0; n<10; ++n )
                    g_sink += (UInt32) page.widgets[ w ]->GetProperty( names[ n ] ).Length();
        UInt64 byName = MT::PrecisionTickCount() - start;
        ReportBench( "GetProperty by name", byName, (UInt64) reps * widgetCount * 10 );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                for ( UInt32 n=0; n<10; ++n )
                    g_sink += (UInt32) page.widgets[ w ]->GetProperty( ids[ n ] ).Length();
        ReportBench( "GetProperty by id", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 10, byName );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                for ( UInt32 n=0; n<10; ++n )
                    g_sink += (UInt32) page.widgets[ w ]->GetPropertyRef( ids[ n ] ).Length();
        ReportBench( "GetPropertyRef by id", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 10, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                GUI::CWidget* widget = page.widgets[ w ];
                g_sink += widget->GetBoolProperty( "Visible" ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( "Enabled" ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( "Clip" ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( "InputTransparent" ) ? 1 : 0;
                g_sink += (UInt32) widget->GetFloatProperty( "Opacity" );
                g_sink += widget->GetColourProperty( "TextColour" ).r;
                g_sink += widget->GetColourProperty( "BackgroundColour" ).a;
                g_sink += (UInt32) widget->GetIntProperty( "BorderWidth" );
            }
        byName = MT::PrecisionTickCount() - start;
        ReportBench( "typed getters by name", byName, (UInt64) reps * widgetCount * 8 );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                GUI::CWidget* widget = page.widgets[ w ];
                g_sink += widget->GetBoolProperty( GUI::EWidgetProperty_Visible ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( GUI::EWidgetProperty_Enabled ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( GUI::EWidgetProperty_Clip ) ? 1 : 0;
                g_sink += widget->GetBoolProperty( GUI::EWidgetProperty_InputTransparent ) ? 1 : 0;
                g_sink += (UInt32) widget->GetFloatProperty( GUI::EWidgetProperty_Opacity );
                g_sink += widget->GetColourProperty( GUI::EWidgetProperty_TextColour ).r;
                g_sink += widget->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ).a;
                g_sink += (UInt32) widget->GetIntProperty( GUI::EWidgetProperty_BorderWidth );
            }
        ReportBench( "typed getters by id", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 8, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                g_sink += page.widgets[ w ]->IsEffectivelyVisible() ? 1 : 0;
        ReportBench( "IsEffectivelyVisible", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                CORE::CString value;
                g_sink += context->GetThemeProperty( *page.widgets[ w ], "TextColour", value ) ? 1 : 0;
                g_sink += context->GetThemeProperty( *page.widgets[ w ], "BackgroundColour", value ) ? 1 : 0;
                g_sink += context->GetThemeProperty( *page.widgets[ w ], "BorderColour", value ) ? 1 : 0;
                g_sink += context->GetThemeProperty( *page.widgets[ w ], "Opacity", value ) ? 1 : 0;
            }
        byName = MT::PrecisionTickCount() - start;
        ReportBench( "theme lookup by name", byName, (UInt64) reps * widgetCount * 4 );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                g_sink += GUCEF_NULL != context->FindThemeValue( *page.widgets[ w ], GUI::EWidgetProperty_TextColour ) ? 1 : 0;
                g_sink += GUCEF_NULL != context->FindThemeValue( *page.widgets[ w ], GUI::EWidgetProperty_BackgroundColour ) ? 1 : 0;
                g_sink += GUCEF_NULL != context->FindThemeValue( *page.widgets[ w ], GUI::EWidgetProperty_BorderColour ) ? 1 : 0;
                g_sink += GUCEF_NULL != context->FindThemeValue( *page.widgets[ w ], GUI::EWidgetProperty_Opacity ) ? 1 : 0;
            }
        ReportBench( "theme lookup by id", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 4, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                GUI::CWidget* widget = page.widgets[ w ];
                widget->SetProperty( "Opacity", ( r & 1 ) ? "0.5" : "1" );
                widget->SetBoolProperty( "Clip", 0 != ( r & 1 ) );
                widget->SetProperty( "Tooltip", ( r & 1 ) ? "a" : "b" );
            }
        byName = MT::PrecisionTickCount() - start;
        ReportBench( "SetProperty by name (value changes)", byName, (UInt64) reps * widgetCount * 3 );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                GUI::CWidget* widget = page.widgets[ w ];
                widget->SetProperty( GUI::EWidgetProperty_Opacity, ( r & 1 ) ? "1" : "0.5" );
                widget->SetBoolProperty( GUI::EWidgetProperty_Clip, 0 == ( r & 1 ) );
                widget->SetProperty( GUI::EWidgetProperty_Tooltip, ( r & 1 ) ? "b" : "a" );
            }
        ReportBench( "SetProperty by id (value changes)", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 3, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                page.widgets[ w ]->SetProperty( "Tooltip", "b" );
        byName = MT::PrecisionTickCount() - start;
        ReportBench( "SetProperty by name (same value)", byName, (UInt64) reps * widgetCount );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
                page.widgets[ w ]->SetProperty( GUI::EWidgetProperty_Tooltip, "b" );
        ReportBench( "SetProperty by id (same value)", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
        {
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                page.widgets[ w ]->SetProperty( "Tooltip", ( r & 1 ) ? "a" : "c" );
                page.widgets[ w ]->SetProperty( "Opacity", ( r & 1 ) ? "0.5" : "1" );
            }
            context->FlushChanges();
        }
        byName = MT::PrecisionTickCount() - start;
        ReportBench( "SetProperty by name and FlushChanges to the driver per changed property", byName, (UInt64) reps * widgetCount * 2 );
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
        {
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                page.widgets[ w ]->SetProperty( GUI::EWidgetProperty_Tooltip, ( r & 1 ) ? "c" : "a" );
                page.widgets[ w ]->SetProperty( GUI::EWidgetProperty_Opacity, ( r & 1 ) ? "1" : "0.5" );
            }
            context->FlushChanges();
        }
        ReportBench( "SetProperty by id and FlushChanges to the driver per changed property", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount * 2, byName );

        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<reps; ++r )
            for ( UInt32 w=0; w<widgetCount; ++w )
            {
                GUI::CDrawSurface::TCommandVector commands;
                GUI::CWidgetPainter::Paint( *page.widgets[ w ], *context, commands );
                g_sink += (UInt32) commands.size();
            }
        ReportBench( "CWidgetPainter::Paint per widget", MT::PrecisionTickCount() - start, (UInt64) reps * widgetCount );

        const UInt32 frames = reps > 4 ? reps / 4 : 1;
        IMAGE::TPixelMapPtr frame;
        start = MT::PrecisionTickCount();
        for ( UInt32 r=0; r<frames; ++r )
        {
            ASSERT_TRUE( context->CaptureFrame( frame ) );
            g_sink += frame.IsNULL() ? 0 : 1;
        }
        ReportBench( "CaptureFrame of the page (software renderer)", MT::PrecisionTickCount() - start, frames );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: Scenes drawn with the properties of the widgets: the pixels and the values do not change with the way properties are kept (hashes in the log)" )
    try
    {
        // a page of 200 widgets in many states
        {
            SBenchPage page;
            ASSERT_TRUE( BuildBenchPage( page ) );
            for ( UInt32 w=0; w<(UInt32)page.widgets.size(); ++w )
            {
                GUI::CWidget* widget = page.widgets[ w ];
                if ( 0 == w % 7 )
                    widget->SetProperty( "Enabled", "false" );
                if ( "Checkbox" == widget->GetType() && 0 == w % 2 )
                    widget->SetProperty( "Checked", "true" );
                if ( "Listbox" == widget->GetType() )
                {
                    widget->SetProperty( "Items", "one\ntwo\nthree" );
                    widget->SetProperty( "SelectedIndex", "1" );
                }
                if ( "Slider" == widget->GetType() )
                    widget->SetProperty( "Value", "40" );
                if ( "ProgressBar" == widget->GetType() )
                    widget->SetProperty( "Value", "70" );
                if ( "Editbox" == widget->GetType() )
                    widget->SetProperty( "Text", "typed" );
                if ( 0 == w % 11 )
                    widget->SetProperty( "StyleClass", "Primary" );
                if ( 0 == w % 13 )
                    widget->SetProperty( "Opacity", "0.5" );
            }
            ReportScene( page.context, "page of 200 widgets" );
        }

        // a window with the widgets of a form
        {
            GUI::TGuiContextPtr contextPtr = CreateNullContext();
            GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != context );
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            CORE::CDataNode theme;
            GUI::BuildDarkTheme( theme, "Test" );
            context->SetTheme( theme );

            GUI::CWidget* window = context->CreateWidget( "Window", "Gallery", GUCEF_NULL );
            window->SetProperty( "Position", "0,20 0,20" );
            window->SetProperty( "Size", "0,420 0,380" );
            window->SetProperty( "Title", "Gallery" );
            context->ShowRoot( window, false );
            GUI::CWidget* label = context->CreateWidget( "Label", "Words", window );
            label->SetProperty( "Position", "0,10 0,10" );
            label->SetProperty( "Size", "0,200 0,40" );
            label->SetProperty( "Text", "A label with words that wrap around the edge" );
            label->SetProperty( "WordWrap", "true" );
            GUI::CWidget* text = context->CreateWidget( "Textbox", "Lines", window );
            text->SetProperty( "Position", "0,10 0,60" );
            text->SetProperty( "Size", "0,200 0,60" );
            text->SetProperty( "Text", "first\nsecond\nthird\nfourth" );
            GUI::CWidget* list = context->CreateWidget( "Listbox", "Choices", window );
            list->SetProperty( "Position", "0,220 0,10" );
            list->SetProperty( "Size", "0,180 0,80" );
            list->SetProperty( "Items", "alpha\nbeta\ngamma\ndelta\nepsilon" );
            list->SetProperty( "SelectedIndex", "2" );
            list->SetProperty( "ScrollPosition", "1" );
            GUI::CWidget* combo = context->CreateWidget( "Combobox", "Drop", window );
            combo->SetProperty( "Position", "0,220 0,100" );
            combo->SetProperty( "Size", "0,180 0,24" );
            combo->SetProperty( "Items", "red\ngreen\nblue" );
            combo->SetProperty( "SelectedIndex", "1" );
            GUI::CWidget* slider = context->CreateWidget( "Slider", "Level", window );
            slider->SetProperty( "Position", "0,10 0,130" );
            slider->SetProperty( "Size", "0,200 0,20" );
            slider->SetProperty( "Value", "35" );
            GUI::CWidget* progress = context->CreateWidget( "ProgressBar", "Done", window );
            progress->SetProperty( "Position", "0,10 0,160" );
            progress->SetProperty( "Size", "0,200 0,20" );
            progress->SetProperty( "Value", "60" );
            GUI::CWidget* check = context->CreateWidget( "Checkbox", "Option", window );
            check->SetProperty( "Position", "0,10 0,190" );
            check->SetProperty( "Size", "0,200 0,24" );
            check->SetProperty( "Text", "An option" );
            check->SetProperty( "Checked", "true" );
            GUI::CWidget* edit = context->CreateWidget( "Editbox", "Name", window );
            edit->SetProperty( "Position", "0,10 0,220" );
            edit->SetProperty( "Size", "0,200 0,24" );
            edit->SetProperty( "Text", "Some text" );
            GUI::CWidget* tabs = context->CreateWidget( "TabControl", "Tabs", window );
            tabs->SetProperty( "Position", "0,10 0,250" );
            tabs->SetProperty( "Size", "0,390 0,100" );
            tabs->SetProperty( "Tabs", "First\nSecond" );
            context->CreateWidget( "TabContentPane", "First", tabs );
            GUI::CWidget* second = context->CreateWidget( "TabContentPane", "Second", tabs );
            GUI::CWidget* inner = context->CreateWidget( "Button", "Inner", second );
            inner->SetProperty( "Size", "0,100 0,30" );
            inner->SetProperty( "Text", "Inside" );
            ReportScene( context, "window with the common widgets" );

            // the same with the pointer over a button and the focus in the edit box
            context->InjectMouseMove( 40, 280 );
            edit->SetFocus();
            ReportScene( context, "window with hover and focus" );

            inner->SetProperty( "Enabled", "false" );
            window->SetProperty( "Collapsed", "true" );
            ReportScene( context, "window collapsed" );
        }

        // the message box and the settings form
        {
            GUI::TGuiContextPtr contextPtr = CreateNullContext();
            GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != context );
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            CORE::CDataNode theme;
            GUI::BuildDarkTheme( theme, "Test" );
            context->SetTheme( theme );

            GUI::CMessageBoxForm box( context );
            ASSERT_TRUE( box.Build() );
            ASSERT_TRUE( box.Open( "Question", "Replace the file?", GUI::CMessageBoxForm::MESSAGEBOXBUTTONS_YES_NO ) );
            ReportScene( context, "message box" );
        }
        {
            GUI::TGuiContextPtr contextPtr = CreateNullContext();
            GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
            ASSERT_TRUE( GUCEF_NULL != context );
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            CORE::CDataNode theme;
            GUI::BuildDarkTheme( theme, "Test" );
            context->SetTheme( theme );

            CORE::CString consoleLoggerEnv = CORE::GetEnv( "GUCEF_CONSOLE_LOGGER" );
            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", "none" );
            GUI::CSettingsForm form( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( form.AddPage( "Video" ) );
            form.AddPage( "General" );
            GUI::CVideoOptions options;
            GUI::CVideoOptions::TDisplayMode mode;
            mode.widthInPixels = 1920;
            mode.heightInPixels = 1080;
            mode.depthInBits = 32;
            mode.frequency = 60;
            options.AddDisplayMode( mode );
            video->SetVideoOptions( options );
            ASSERT_TRUE( form.Build() );
            ASSERT_TRUE( form.Open() );
            ReportScene( context, "settings form" );
            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", consoleLoggerEnv );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: Every standard property has an id and a name that convert both ways, other names are no standard property" )
    try
    {
        ASSERT_TRUE( GUI::WidgetPropertyToString( GUI::EWidgetProperty_Unknown ).IsNULLOrEmpty() );
        ASSERT_TRUE( GUI::WidgetPropertyToString( GUI::EWidgetProperty_Count ).IsNULLOrEmpty() );
        ASSERT_TRUE( GUI::WidgetPropertyToString( (GUI::EWidgetProperty) 100000 ).IsNULLOrEmpty() );
        ASSERT_FALSE( GUI::IsStandardWidgetProperty( GUI::EWidgetProperty_Unknown ) || GUI::IsStandardWidgetProperty( GUI::EWidgetProperty_Count ) );

        CORE::CString previous;
        UInt32 standardCount = 0;
        for ( UInt32 id=1; id<(UInt32)GUI::EWidgetProperty_Count; ++id )
        {
            GUI::EWidgetProperty property = (GUI::EWidgetProperty) id;
            const CORE::CString& name = GUI::WidgetPropertyToString( property );
            ASSERT_FALSE( name.IsNULLOrEmpty() );
            ASSERT_TRUE( GUI::IsStandardWidgetProperty( property ) );
            ASSERT_TRUE( property == GUI::StringToWidgetProperty( name ) );
            ASSERT_TRUE( property == GUI::StringToWidgetProperty( name.C_String() ) );
            ASSERT_TRUE( property == GUI::StringToWidgetProperty( CORE::CString( name.C_String() ) ) );
            ASSERT_TRUE( 0 < strcmp( name.C_String(), previous.C_String() ) );         // the ids are in the order of the names, so no name is in twice
            ASSERT_TRUE( &GUI::WidgetPropertyToString( property ) == &name );          // the same string every time, no copies
            previous = name;
            ++standardCount;
        }
        ASSERT_TRUE( standardCount + 1 == (UInt32) GUI::EWidgetProperty_Count && standardCount > 100 );

        // the properties the painters, the layout and the input read all the time
        ASSERT_TRUE( "Text" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Text ) && "TextColour" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_TextColour ) );
        ASSERT_TRUE( "BackgroundColour" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_BackgroundColour ) && "BorderColour" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_BorderColour ) );
        ASSERT_TRUE( "Position" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Position ) && "Size" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Size ) );
        ASSERT_TRUE( "Visible" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Visible ) && "Enabled" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Enabled ) );
        ASSERT_TRUE( "Clip" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Clip ) && "InputTransparent" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_InputTransparent ) );
        ASSERT_TRUE( "Title" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Title ) && "TitleVisible" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_TitleVisible ) );
        ASSERT_TRUE( "Value" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Value ) && "Min" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Min ) );
        ASSERT_TRUE( "Max" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Max ) && "Step" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Step ) );
        ASSERT_TRUE( "SelectedIndex" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_SelectedIndex ) && "Items" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_Items ) );
        ASSERT_TRUE( "ScrollPosition" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_ScrollPosition ) && "ScrollStep" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_ScrollStep ) );
        ASSERT_TRUE( "ScrollBarWidth" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_ScrollBarWidth ) && "MaxColumnWidth" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_MaxColumnWidth ) );

        // names that are no standard property: nothing close to one, other case, spaces, empty
        static const char* const others[] = { "", "Nope", "text", "TEXT", "Tex", "TextX", " Text", "Text ", "Zzzz", "A", "~", "Text\n", "ExoticHint", "NullOnly", "WINDOW", "Opacit", "Opacityy", "0" };
        for ( UInt32 i=0; i<sizeof( others ) / sizeof( others[ 0 ] ); ++i )
        {
            ASSERT_TRUE( GUI::EWidgetProperty_Unknown == GUI::StringToWidgetProperty( others[ i ] ) );
            ASSERT_TRUE( GUI::EWidgetProperty_Unknown == GUI::StringToWidgetProperty( CORE::CString( others[ i ] ) ) );
        }
        ASSERT_TRUE( GUI::EWidgetProperty_Unknown == GUI::StringToWidgetProperty( CORE::CString::Empty ) );
        ASSERT_TRUE( GUI::EWidgetProperty_Unknown == GUI::StringToWidgetProperty( (const char*) GUCEF_NULL ) );

        ASSERT_TRUE( GUI::IsWidgetStateProperty( GUI::EWidgetProperty_Visible ) && GUI::IsWidgetStateProperty( GUI::EWidgetProperty_Enabled ) && GUI::IsWidgetStateProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_FALSE( GUI::IsWidgetStateProperty( GUI::EWidgetProperty_Text ) );

        // the ids of the stock widget types: every standard property is described by one and every descriptor has its id
        ASSERT_TRUE( EveryStandardPropertyIsDescribed() );
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        GUI::CWidgetTypeRegistry::TDescriptorVector labelProperties = registry.GetProperties( "Label" );
        for ( UInt32 i=0; i<(UInt32)labelProperties.size(); ++i )
            ASSERT_TRUE( GUI::StringToWidgetProperty( labelProperties[ i ]->name ) == labelProperties[ i ]->id && GUI::EWidgetProperty_Unknown != labelProperties[ i ]->id );
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "Checkbox", GUI::EWidgetProperty_NormalImage ) && registry.FindProperty( "Checkbox", GUI::EWidgetProperty_NormalImage ) == registry.FindProperty( "Checkbox", "NormalImage" ) );
        ASSERT_TRUE( GUCEF_NULL == registry.FindProperty( "Label", GUI::EWidgetProperty_NormalImage ) && GUCEF_NULL == registry.FindProperty( "No such type", GUI::EWidgetProperty_Text ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: A widget reaches a standard property by id or by name, anything else by name, with the values, defaults, validation and events it always had" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );

        GUI::CWidget* label = context->CreateWidget( "Label", "L", GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL != label );

        // the same value by id and by name
        ASSERT_FALSE( label->IsPropertySet( GUI::EWidgetProperty_Text ) || label->IsPropertySet( "Text" ) );
        ASSERT_TRUE( label->SetProperty( GUI::EWidgetProperty_Text, "a" ) );
        ASSERT_TRUE( "a" == label->GetProperty( "Text" ) && "a" == label->GetProperty( GUI::EWidgetProperty_Text ) && "a" == label->GetPropertyRef( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( label->SetProperty( "Text", "b" ) );
        ASSERT_TRUE( "b" == label->GetProperty( GUI::EWidgetProperty_Text ) && label->IsPropertySet( GUI::EWidgetProperty_Text ) && label->IsPropertySet( "Text" ) );
        CORE::CString value;
        ASSERT_TRUE( label->GetExplicitProperty( GUI::EWidgetProperty_Text, value ) && "b" == value );
        ASSERT_TRUE( label->GetExplicitProperty( "Text", value ) && "b" == value );
        ASSERT_TRUE( label->ResetProperty( GUI::EWidgetProperty_Text ) );
        ASSERT_FALSE( label->IsPropertySet( "Text" ) );
        ASSERT_TRUE( label->GetProperty( GUI::EWidgetProperty_Text ).IsNULLOrEmpty() && label->GetProperty( "Text" ).IsNULLOrEmpty() );
        ASSERT_TRUE( label->SetProperty( "Text", "c" ) && label->ResetProperty( "Text" ) && !label->IsPropertySet( GUI::EWidgetProperty_Text ) );

        // the defaults of the descriptors, by id and by name
        ASSERT_TRUE( "true" == label->GetProperty( GUI::EWidgetProperty_Visible ) && label->GetBoolProperty( GUI::EWidgetProperty_Visible ) && label->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( "0,0 0,0" == label->GetProperty( GUI::EWidgetProperty_Position ) && "1" == label->GetProperty( "Opacity" ) );
        ASSERT_TRUE( 1.0f == label->GetFloatProperty( GUI::EWidgetProperty_Opacity ) && 1.0f == label->GetFloatProperty( "Opacity" ) );
        ASSERT_TRUE( -1 == label->GetIntProperty( GUI::EWidgetProperty_TabIndex ) && -1 == label->GetIntProperty( "TabIndex" ) );
        ASSERT_TRUE( label->GetProperty( GUI::EWidgetProperty_NormalImage ).IsNULLOrEmpty() );     // a property the type does not describe

        // validation as always: a value that does not fit is refused, one that does is normalized
        ASSERT_FALSE( label->SetProperty( GUI::EWidgetProperty_Visible, "maybe" ) );
        ASSERT_FALSE( label->SetProperty( "Visible", "maybe" ) );
        ASSERT_TRUE( label->IsVisible() );
        ASSERT_TRUE( label->SetProperty( GUI::EWidgetProperty_Visible, " NO " ) && "false" == label->GetProperty( "Visible" ) && !label->IsVisible() );
        ASSERT_TRUE( label->SetProperty( "Visible", "on" ) && "true" == label->GetProperty( GUI::EWidgetProperty_Visible ) );
        ASSERT_FALSE( label->SetProperty( GUI::EWidgetProperty_Size, "wide" ) );
        ASSERT_TRUE( label->SetProperty( GUI::EWidgetProperty_Size, "0.5,10 0,20" ) && "0.5,10 0,20" == label->GetProperty( "Size" ) );
        ASSERT_FALSE( label->SetProperty( GUI::EWidgetProperty_Unknown, "x" ) );
        ASSERT_FALSE( label->SetProperty( GUI::EWidgetProperty_Count, "x" ) );
        ASSERT_FALSE( label->SetProperty( CORE::CString::Empty, "x" ) );
        ASSERT_TRUE( label->SetProperty( GUI::EWidgetProperty_Text, "" ) && label->IsPropertySet( GUI::EWidgetProperty_Text ) );   // an empty value is a value
        ASSERT_TRUE( label->ResetProperty( GUI::EWidgetProperty_Unknown ) && label->ResetProperty( GUI::EWidgetProperty_Count ) );

        // typed access by id and by name
        GUI::CWidget* slider = context->CreateWidget( "Slider", "S", GUCEF_NULL );
        ASSERT_TRUE( slider->SetFloatProperty( GUI::EWidgetProperty_Value, 12.5f ) && 12.5f == slider->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( slider->SetIntProperty( "Max", 80 ) && 80 == slider->GetIntProperty( GUI::EWidgetProperty_Max ) );
        ASSERT_FALSE( slider->GetBoolProperty( GUI::EWidgetProperty_Clip ) );
        ASSERT_TRUE( slider->SetBoolProperty( "Clip", true ) && slider->GetBoolProperty( GUI::EWidgetProperty_Clip ) && slider->SetBoolProperty( GUI::EWidgetProperty_Clip, false ) && !slider->GetBoolProperty( "Clip" ) );
        GUI::CColour colour;
        ASSERT_FALSE( slider->TryGetColourProperty( GUI::EWidgetProperty_BackgroundColour, colour ) );
        ASSERT_TRUE( slider->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, GUI::CColour( 1, 2, 3, 4 ) ) );
        ASSERT_TRUE( slider->TryGetColourProperty( GUI::EWidgetProperty_BackgroundColour, colour ) && GUI::CColour( 1, 2, 3, 4 ) == colour && GUI::CColour( 1, 2, 3, 4 ) == slider->GetColourProperty( "BackgroundColour" ) );
        ASSERT_TRUE( slider->SetColourProperty( "BackgroundColour", GUI::CColour( 5, 6, 7, 8 ) ) && GUI::CColour( 5, 6, 7, 8 ) == slider->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ) );
        ASSERT_TRUE( slider->SetProperty( GUI::EWidgetProperty_BackgroundColour, "#102030" ) && GUI::CColour( 16, 32, 48, 255 ) == slider->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ) );
        ASSERT_TRUE( slider->SetProperty( GUI::EWidgetProperty_Position, "0.25,4 0,8" ) && 4 == slider->GetUVectorProperty( GUI::EWidgetProperty_Position ).GetX().GetPixelValue() && 8 == slider->GetUVectorProperty( "Position" ).GetY().GetPixelValue() );

        GUI::CWidget* list = context->CreateWidget( "Listbox", "List", GUCEF_NULL );
        GUI::CWidget::TStringVector items;
        items.push_back( "one" );
        items.push_back( "two" );
        ASSERT_TRUE( list->SetStringListProperty( GUI::EWidgetProperty_Items, items ) && 2 == list->GetStringListProperty( "Items" ).size() && "two" == list->GetStringListProperty( GUI::EWidgetProperty_Items )[ 1 ] );
        GUCEF::vector< Int32 > indices;
        indices.push_back( 1 );
        indices.push_back( 0 );
        ASSERT_TRUE( list->SetIntListProperty( "DisabledItems", indices ) && 2 == list->GetIntListProperty( GUI::EWidgetProperty_DisabledItems ).size() && "1,0" == list->GetProperty( "DisabledItems" ) );

        // a property that is not a standard one: by name, kept, passed on, listed, saved
        ASSERT_TRUE( label->SetProperty( "ExoticHint", "x" ) && "x" == label->GetProperty( "ExoticHint" ) && "x" == label->GetPropertyRef( "ExoticHint" ) );
        ASSERT_TRUE( label->IsPropertySet( "ExoticHint" ) && label->HasProperty( "ExoticHint" ) && label->GetExplicitProperty( "ExoticHint", value ) && "x" == value );
        ASSERT_TRUE( label->SetIntProperty( "ExoticCount", 7 ) && 7 == label->GetIntProperty( "ExoticCount" ) && label->SetBoolProperty( "ExoticFlag", true ) && label->GetBoolProperty( "ExoticFlag" ) );
        ASSERT_TRUE( label->SetFloatProperty( "ExoticFloat", 1.5f ) && 1.5f == label->GetFloatProperty( "ExoticFloat" ) && label->SetColourProperty( "ExoticColour", GUI::CColour( 9, 8, 7, 6 ) ) && GUI::CColour( 9, 8, 7, 6 ) == label->GetColourProperty( "ExoticColour" ) );
        ASSERT_TRUE( label->SetProperty( "text", "lower" ) && "lower" == label->GetProperty( "text" ) && label->GetProperty( GUI::EWidgetProperty_Text ).IsNULLOrEmpty() );   // the names are case sensitive: this is another property
        CORE::CValueList all;
        CORE::CVariant probe;
        ASSERT_TRUE( label->GetProperties( all ) && all.TryGetValue( "ExoticHint", probe ) && all.TryGetValue( "Text", probe ) && all.TryGetValue( "text", probe ) );
        GUI::CWidget::TPropertyMap explicitProperties = label->GetExplicitProperties();
        ASSERT_TRUE( "x" == explicitProperties[ "ExoticHint" ] && "lower" == explicitProperties[ "text" ] && "on" != explicitProperties[ "Visible" ] && "true" == explicitProperties[ "Visible" ] && "0.5,10 0,20" == explicitProperties[ "Size" ] );
        GUI::CWidget::TStringVector names = label->GetPropertyNames();
        UInt32 hintAt = 0, textAt = 0, lowerAt = 0, countAt = 0;
        for ( UInt32 i=0; i<(UInt32)names.size(); ++i )
        {
            if ( "ExoticHint" == names[ i ] ) hintAt = i + 1;
            if ( "Text" == names[ i ] ) textAt = i + 1;
            if ( "text" == names[ i ] ) lowerAt = i + 1;
            if ( "ExoticCount" == names[ i ] ) countAt = i + 1;
        }
        ASSERT_TRUE( textAt > 0 && hintAt > textAt && countAt > 0 && lowerAt > 0 );    // the properties of the type first, then the other ones
        ASSERT_TRUE( label->ResetProperty( "ExoticHint" ) && !label->IsPropertySet( "ExoticHint" ) && label->GetProperty( "ExoticHint" ).IsNULLOrEmpty() );
        ASSERT_TRUE( label->GetPropertyDescriptor( "Text" ) == label->GetPropertyDescriptor( GUI::EWidgetProperty_Text ) && GUCEF_NULL != label->GetPropertyDescriptor( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( GUCEF_NULL == label->GetPropertyDescriptor( "ExoticHint" ) );

        // the events carry the id, and for the others the name
        {
            GUI::CWidget* edit = context->CreateWidget( "Editbox", "E", GUCEF_NULL );
            CPropertyEventObserver changed( edit, GUI::CWidget::PropertyChangedEvent );
            CPropertyEventObserver valueChanged( edit, GUI::CWidget::ValueChangedEvent );
            ASSERT_TRUE( edit->SetProperty( GUI::EWidgetProperty_Text, "typed" ) );
            ASSERT_TRUE( 1 == changed.count && GUI::EWidgetProperty_Text == changed.property && "Text" == changed.name );
            ASSERT_TRUE( edit->SetProperty( "ExoticHint", "hint" ) );
            ASSERT_TRUE( 2 == changed.count && GUI::EWidgetProperty_Unknown == changed.property && "ExoticHint" == changed.name );
            ASSERT_TRUE( edit->SetProperty( "Text", "typed" ) && 2 == changed.count );          // no change, no event
            ASSERT_TRUE( edit->ResetProperty( GUI::EWidgetProperty_Text ) && 3 == changed.count && GUI::EWidgetProperty_Text == changed.property );

            ASSERT_TRUE( edit->ChangeValueByUser( GUI::EWidgetProperty_Text, "by the user" ) );
            ASSERT_TRUE( 1 == valueChanged.count && GUI::EWidgetProperty_Text == valueChanged.property && "Text" == valueChanged.name && "by the user" == valueChanged.newValue && valueChanged.oldValue.IsNULLOrEmpty() );
            ASSERT_TRUE( edit->ChangeValueByUser( "Text", "again" ) );
            ASSERT_TRUE( 2 == valueChanged.count && GUI::EWidgetProperty_Text == valueChanged.property && "by the user" == valueChanged.oldValue && "again" == valueChanged.newValue );
            ASSERT_TRUE( edit->ChangeValueByUser( "ExoticHint", "by hand" ) );
            ASSERT_TRUE( 3 == valueChanged.count && GUI::EWidgetProperty_Unknown == valueChanged.property && "ExoticHint" == valueChanged.name );
            ASSERT_TRUE( edit->ChangeValueByUser( "Text", "again" ) && 3 == valueChanged.count );      // nothing changed
            ASSERT_FALSE( edit->ChangeValueByUser( GUI::EWidgetProperty_Visible, "maybe" ) );
            GUI::CValueChangedEventData data;
            data.SetPropertyName( "ScrollPosition" );
            ASSERT_TRUE( GUI::EWidgetProperty_ScrollPosition == data.property && "ScrollPosition" == data.GetPropertyName() );
            data.SetPropertyName( "Whatever" );
            ASSERT_TRUE( GUI::EWidgetProperty_Unknown == data.property && "Whatever" == data.GetPropertyName() );
        }

        // a value remembers what it was read as, and forgets it with the text
        {
            GUI::CWidgetPropertyValue reading( "true" );
            ASSERT_TRUE( reading.AsBool() && "true" == reading.GetText() );
            reading.SetText( "0" );
            ASSERT_TRUE( !reading.AsBool() && 0 == reading.AsInt() && 0.0f == reading.AsFloat() );
            reading.SetText( "12.5" );
            ASSERT_TRUE( 12 == reading.AsInt() && 12.5f == reading.AsFloat() );
            reading.SetText( "#102030" );
            GUI::CColour read( 7, 7, 7, 7 );
            ASSERT_TRUE( reading.TryAsColour( read ) && GUI::CColour( 16, 32, 48, 255 ) == read && GUI::CColour( 16, 32, 48, 255 ) == reading.AsColour() );
            reading.SetText( "no colour" );
            read = GUI::CColour( 7, 7, 7, 7 );
            ASSERT_FALSE( reading.TryAsColour( read ) );
            ASSERT_TRUE( GUI::CColour( 7, 7, 7, 7 ) == read && GUI::CColour( 0, 0, 0, 0 ) == reading.AsColour() );
            reading.SetText( "3,4,5,6" );
            reading.ReadAll();
            ASSERT_TRUE( GUI::CColour( 3, 4, 5, 6 ) == reading.AsColour() && 3 == reading.AsInt() );
            ASSERT_TRUE( CORE::StringToBool( "yes", false ) == GUI::CWidgetPropertyValue( "yes" ).AsBool() && CORE::StringToInt32( " 42 ", 0 ) == GUI::CWidgetPropertyValue( " 42 " ).AsInt() );
        }

        // the state: Enabled and Checked reach the state of the widget
        GUI::CWidget* box = context->CreateWidget( "Checkbox", "C", GUCEF_NULL );
        ASSERT_TRUE( 0 == ( box->GetState() & GUI::CWidget::STATE_CHECKED ) );
        ASSERT_TRUE( box->SetProperty( GUI::EWidgetProperty_Checked, "true" ) && 0 != ( box->GetState() & GUI::CWidget::STATE_CHECKED ) );
        ASSERT_TRUE( box->SetBoolProperty( GUI::EWidgetProperty_Enabled, false ) && 0 != ( box->GetState() & GUI::CWidget::STATE_DISABLED ) && !box->IsEnabled() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: A widget type an application registers with a property of its own keeps its descriptors, defaults, validation and inheritance, by name" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();

        GUI::CWidgetTypeInfo base;
        base.typeName = "PropertyTestBase";
        base.baseTypeName = "Label";
        base.category = "Test";
        base.creator = &GUI::CLabel::Create;
        base.AddProperty( GUI::CPropertyDescriptor( "Cargo", GUI::CPropertyDescriptor::VALUETYPE_INT, "5", GUI::CPropertyDescriptor::FLAG_DEFAULT, "Test", "A number of its own" ) );
        base.AddProperty( GUI::CPropertyDescriptor( "Text", GUI::CPropertyDescriptor::VALUETYPE_STRING, "base text", GUI::CPropertyDescriptor::FLAG_DEFAULT, "Test", "Text with another default" ) );
        ASSERT_TRUE( registry.RegisterType( base ) );
        GUI::CWidgetTypeInfo derived;
        derived.typeName = "PropertyTestDerived";
        derived.baseTypeName = "PropertyTestBase";
        derived.category = "Test";
        derived.creator = &GUI::CLabel::Create;
        derived.AddProperty( GUI::CPropertyDescriptor( "Cargo", GUI::CPropertyDescriptor::VALUETYPE_INT, "9", GUI::CPropertyDescriptor::FLAG_DEFAULT, "Test", "Another default" ) );
        derived.AddProperty( GUI::CPropertyDescriptor( "Flavour", GUI::CPropertyDescriptor::VALUETYPE_BOOL, "true", GUI::CPropertyDescriptor::FLAG_DEFAULT, "Test", "A flag of its own" ) );
        ASSERT_TRUE( registry.RegisterType( derived ) );

        GUI::CWidget* widget = context->CreateWidget( "PropertyTestDerived", "Mine", GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL != widget );
        ASSERT_TRUE( registry.IsA( "PropertyTestDerived", "Label" ) && registry.IsA( "PropertyTestDerived", "Widget" ) && !registry.IsA( "PropertyTestDerived", "Button" ) );
        ASSERT_TRUE( 9 == widget->GetIntProperty( "Cargo" ) && widget->GetBoolProperty( "Flavour" ) );
        ASSERT_TRUE( "base text" == widget->GetProperty( GUI::EWidgetProperty_Text ) && "base text" == widget->GetProperty( "Text" ) );
        ASSERT_TRUE( registry.FindProperty( "PropertyTestDerived", "Cargo" ) != registry.FindProperty( "PropertyTestBase", "Cargo" ) && "9" == registry.FindProperty( "PropertyTestDerived", "Cargo" )->defaultValue );
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "PropertyTestDerived", "Visible" ) && GUI::EWidgetProperty_Unknown == registry.FindProperty( "PropertyTestDerived", "Cargo" )->id );

        // the properties of the type, base first, a redefined one in place with the definition of the most derived type
        GUI::CWidgetTypeRegistry::TDescriptorVector properties = registry.GetProperties( "PropertyTestDerived" );
        GUI::CWidgetTypeRegistry::TDescriptorVector labelProperties = registry.GetProperties( "Label" );
        ASSERT_TRUE( properties.size() == labelProperties.size() + 2 );
        UInt32 cargoAt = 0, textAt = 0, flavourAt = 0;
        for ( UInt32 i=0; i<(UInt32)properties.size(); ++i )
        {
            if ( "Cargo" == properties[ i ]->name ) { cargoAt = i + 1; ASSERT_TRUE( "9" == properties[ i ]->defaultValue ); }
            if ( "Text" == properties[ i ]->name ) { textAt = i + 1; ASSERT_TRUE( "base text" == properties[ i ]->defaultValue ); }
            if ( "Flavour" == properties[ i ]->name ) flavourAt = i + 1;
        }
        ASSERT_TRUE( cargoAt > 0 && textAt > 0 && flavourAt > cargoAt && textAt <= labelProperties.size() );

        // validation of the descriptors of the type, by name
        ASSERT_FALSE( widget->SetProperty( "Cargo", "many" ) );
        ASSERT_TRUE( widget->SetProperty( "Cargo", " 12 " ) && "12" == widget->GetProperty( "Cargo" ) && 12 == widget->GetIntProperty( "Cargo" ) );
        ASSERT_TRUE( widget->SetBoolProperty( "Flavour", false ) && !widget->GetBoolProperty( "Flavour" ) );
        ASSERT_TRUE( widget->ResetProperty( "Cargo" ) && 9 == widget->GetIntProperty( "Cargo" ) );
        GUI::CWidget::TStringVector widgetNames = widget->GetPropertyNames();
        ASSERT_TRUE( widgetNames.size() == properties.size() );

        // the type registered again: the widgets that were made see the new descriptors
        GUI::CWidgetTypeInfo changed = derived;
        changed.properties.clear();
        changed.AddProperty( GUI::CPropertyDescriptor( "Cargo", GUI::CPropertyDescriptor::VALUETYPE_INT, "77", GUI::CPropertyDescriptor::FLAG_DEFAULT, "Test", "Yet another default" ) );
        ASSERT_TRUE( registry.RegisterType( changed ) );
        ASSERT_TRUE( 77 == widget->GetIntProperty( "Cargo" ) && "false" == widget->GetProperty( "Flavour" ) && widget->IsPropertySet( "Flavour" ) );

        // and a type that is gone: nothing described, what was set stays
        ASSERT_TRUE( widget->SetProperty( "Cargo", "3" ) );
        ASSERT_TRUE( registry.UnregisterType( "PropertyTestDerived" ) );
        ASSERT_TRUE( "3" == widget->GetProperty( "Cargo" ) && widget->GetProperty( GUI::EWidgetProperty_Text ).IsNULLOrEmpty() && GUCEF_NULL == widget->GetPropertyDescriptor( "Cargo" ) );
        ASSERT_TRUE( widget->SetProperty( "Cargo", "not a number" ) && "not a number" == widget->GetProperty( "Cargo" ) );
        context->DestroyWidget( widget );
        ASSERT_TRUE( registry.UnregisterType( "PropertyTestBase" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: The theme gives the same value by id and by name as the theme tree says, over every widget type, every state, class and name" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
        CORE::CDataNode theme;
        BuildTestTheme( theme );
        ASSERT_TRUE( context->SetTheme( theme ) );

        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        GUI::CWidgetTypeRegistry::TStringSet types = registry.GetTypeNames();
        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        context->ShowRoot( root, false );

        // the property names to ask for: every one of every type, and a few that no type has
        GUCEF::set< CORE::CString > propertyNames;
        for ( GUI::CWidgetTypeRegistry::TStringSet::const_iterator t=types.begin(); t!=types.end(); ++t )
        {
            GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = registry.GetProperties( *t );
            for ( UInt32 d=0; d<(UInt32)descriptors.size(); ++d )
                propertyNames.insert( descriptors[ d ]->name );
        }
        propertyNames.insert( "ExoticHint" );
        propertyNames.insert( "NoSuchProperty" );
        propertyNames.insert( "text" );
        ASSERT_TRUE( propertyNames.size() > 100 );

        // widgets of every type in every combination of what the styles select on
        UInt32 compared = 0, themed = 0;
        Int32 row = 0;
        for ( GUI::CWidgetTypeRegistry::TStringSet::const_iterator t=types.begin(); t!=types.end(); ++t )
        {
            if ( "PropertyTestBase" == *t || "PropertyTestDerived" == *t )
                continue;
            for ( UInt32 variant=0; variant<8; ++variant )
            {
                GUI::CWidget* widget = context->CreateWidget( *t, ( 0 != ( variant & 1 ) ) ? CORE::CString( "Target" ) : CORE::CString( "Other" ), root );
                if ( GUCEF_NULL == widget )
                    break;
                widget->SetProperty( "Position", CORE::CString( "0,0 0," ) + CORE::Int32ToString( row * 4 ) );
                widget->SetProperty( "Size", "0,100 0,3" );
                ++row;
                if ( 0 != ( variant & 2 ) )
                    widget->SetProperty( "StyleClass", "Dim  Primary" );
                if ( 0 != ( variant & 4 ) )
                {
                    widget->SetProperty( "Enabled", "false" );
                    widget->SetProperty( "Checked", "true" );
                }
                widget->SetProperty( "Focusable", "true" );
                if ( variant >= 6 )
                    widget->SetFocus();
                if ( 3 == variant || 7 == variant )
                    context->InjectMouseMove( 50, ( row - 1 ) * 4 + 1 );

                for ( GUCEF::set< CORE::CString >::const_iterator n=propertyNames.begin(); n!=propertyNames.end(); ++n )
                {
                    CORE::CString expected;
                    bool expectedFound = LegacyThemeProperty( context->GetTheme(), *widget, *n, expected );
                    CORE::CString byName;
                    bool foundByName = context->GetThemeProperty( *widget, *n, byName );
                    ASSERT_TRUE( expectedFound == foundByName && ( !expectedFound || expected == byName ) );
                    GUI::EWidgetProperty property = GUI::StringToWidgetProperty( *n );
                    if ( GUI::EWidgetProperty_Unknown != property )
                    {
                        CORE::CString byId;
                        bool foundById = context->GetThemeProperty( *widget, property, byId );
                        ASSERT_TRUE( expectedFound == foundById && ( !expectedFound || expected == byId ) );
                        const GUI::CWidgetPropertyValue* value = context->FindThemeValue( *widget, property );
                        ASSERT_TRUE( expectedFound == ( GUCEF_NULL != value ) && ( !expectedFound || expected == value->GetText() ) );
                    }

                    // and the value of the property as the widget gives it, by name, by id and by reference
                    CORE::CString expectedValue = LegacyPropertyValue( context->GetTheme(), *widget, *n );
                    ASSERT_TRUE( expectedValue == widget->GetProperty( *n ) && expectedValue == widget->GetPropertyRef( *n ) );
                    if ( GUI::EWidgetProperty_Unknown != property )
                        ASSERT_TRUE( expectedValue == widget->GetProperty( property ) && expectedValue == widget->GetPropertyRef( property ) );
                    ++compared;
                    themed += expectedFound ? 1 : 0;
                }
            }
        }
        ASSERT_TRUE( compared > 10000 && themed > 500 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PropertyTheme: " << compared << " values compared with the theme tree, " << themed << " of them from a style" );

        // a new theme: the values follow
        CORE::CDataNode other;
        other.SetName( "GUITheme" );
        CORE::CDataNode* style = other.AddChild( "Style" );
        style->SetAttribute( "type", "Label" );
        CORE::CDataNode* property = style->AddChild( "Property" );
        property->SetAttribute( "name", "TextColour" );
        property->SetAttribute( "value", "7,7,7,255" );
        GUI::CWidget* label = context->CreateWidget( "Label", "Plain", root );
        ASSERT_TRUE( context->SetTheme( other ) );
        ASSERT_TRUE( GUI::CColour( 7, 7, 7, 255 ) == label->GetColourProperty( GUI::EWidgetProperty_TextColour ) && "7,7,7,255" == label->GetProperty( "TextColour" ) );
        ASSERT_TRUE( label->GetProperty( GUI::EWidgetProperty_BackgroundColour ).IsNULLOrEmpty() );
        ASSERT_TRUE( label->SetProperty( GUI::EWidgetProperty_TextColour, "1,1,1,255" ) && GUI::CColour( 1, 1, 1, 255 ) == label->GetColourProperty( "TextColour" ) );
        ASSERT_TRUE( label->ResetProperty( GUI::EWidgetProperty_TextColour ) && GUI::CColour( 7, 7, 7, 255 ) == label->GetColourProperty( "TextColour" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: The default of every property of every stock widget type is what the descriptor says, by id and by name, and an empty theme gives nothing" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        GUI::CWidgetTypeRegistry::TStringSet types = registry.GetTypeNames();
        UInt32 checked = 0;
        for ( GUI::CWidgetTypeRegistry::TStringSet::const_iterator t=types.begin(); t!=types.end(); ++t )
        {
            GUI::CWidget* widget = context->CreateWidget( *t, "W", GUCEF_NULL );
            if ( GUCEF_NULL == widget )
                continue;
            GUI::CWidgetTypeRegistry::TDescriptorVector descriptors = registry.GetProperties( *t );
            ASSERT_TRUE( widget->GetPropertyNames().size() == descriptors.size() );
            for ( UInt32 d=0; d<(UInt32)descriptors.size(); ++d )
            {
                const GUI::CPropertyDescriptor* descriptor = descriptors[ d ];
                ASSERT_TRUE( descriptor->defaultValue == widget->GetProperty( descriptor->name ) );
                ASSERT_TRUE( descriptor == widget->GetPropertyDescriptor( descriptor->name ) && descriptor == widget->GetPropertyDescriptor( descriptor->id ) );
                ASSERT_TRUE( widget->HasProperty( descriptor->name ) && widget->HasProperty( descriptor->id ) && !widget->IsPropertySet( descriptor->id ) );

                // the default is a value the descriptor accepts, and setting it by id or name leaves it the same
                CORE::CString normalized;
                if ( !descriptor->defaultValue.IsNULLOrEmpty() )
                {
                    ASSERT_TRUE( descriptor->Validate( descriptor->defaultValue, normalized ) );
                    ASSERT_TRUE( widget->SetProperty( descriptor->id, descriptor->defaultValue ) && widget->IsPropertySet( descriptor->name ) );
                    ASSERT_TRUE( normalized == widget->GetProperty( descriptor->name ) );
                    ASSERT_TRUE( widget->ResetProperty( descriptor->name ) && !widget->IsPropertySet( descriptor->id ) );
                }
                ++checked;
            }
            context->DestroyWidget( widget );
        }
        ASSERT_TRUE( checked > 300 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Properties: The layout nodes of a form are the same for a property named by id and by name, and the handler gets the changes in the order of the names" )
    try
    {
        CORE::CDataNode byId;
        GUI::CFormLayout::InitLayout( byId );
        CORE::CDataNode* idWidget = GUI::CFormLayout::AddWidgetNode( byId, "Label", "L", "0,5 0,5", "0,100 0,20" );
        GUI::CFormLayout::SetWidgetNodeProperty( *idWidget, GUI::EWidgetProperty_Text, "Hello" );
        GUI::CFormLayout::SetWidgetNodeProperty( *idWidget, GUI::EWidgetProperty_Text, "Hello again" );
        GUI::CFormLayout::SetWidgetNodeProperty( *idWidget, GUI::EWidgetProperty_TextColour, "1,2,3,255" );
        GUI::CFormLayout::SetWidgetNodeProperty( *idWidget, "ExoticHint", "x" );
        GUI::CFormLayout::SetWidgetNodeProperty( *idWidget, GUI::EWidgetProperty_Unknown, "ignored" );

        CORE::CDataNode byName;
        GUI::CFormLayout::InitLayout( byName );
        CORE::CDataNode* nameWidget = GUI::CFormLayout::AddWidgetNode( byName, "Label", "L", "0,5 0,5", "0,100 0,20" );
        GUI::CFormLayout::SetWidgetNodeProperty( *nameWidget, "Text", "Hello" );
        GUI::CFormLayout::SetWidgetNodeProperty( *nameWidget, "Text", "Hello again" );
        GUI::CFormLayout::SetWidgetNodeProperty( *nameWidget, "TextColour", "1,2,3,255" );
        GUI::CFormLayout::SetWidgetNodeProperty( *nameWidget, "ExoticHint", "x" );
        ASSERT_TRUE( 3 == idWidget->GetNrOfChildNodes() && 3 == nameWidget->GetNrOfChildNodes() );
        CORE::CDataNode::const_iterator a = idWidget->ConstBegin();
        CORE::CDataNode::const_iterator b = nameWidget->ConstBegin();
        for ( ; a != idWidget->ConstEnd() && b != nameWidget->ConstEnd(); ++a, ++b )
        {
            ASSERT_TRUE( (*a)->GetName() == (*b)->GetName() );
            ASSERT_TRUE( (*a)->GetAttributeValue( "name" ).AsString() == (*b)->GetAttributeValue( "name" ).AsString() );
            ASSERT_TRUE( (*a)->GetAttributeValue( "value" ).AsString() == (*b)->GetAttributeValue( "value" ).AsString() );
        }
        ASSERT_TRUE( "Text" == (*idWidget->ConstBegin())->GetAttributeValue( "name" ).AsString() && "Hello again" == (*idWidget->ConstBegin())->GetAttributeValue( "value" ).AsString() );

        // the nodes build a widget with the values
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CFormLayout::TWidgetSourceMap sources;
        GUI::CWidget* built = GUI::CFormLayout::Build( *context, *idWidget, GUCEF_NULL, sources );
        ASSERT_TRUE( GUCEF_NULL != built );
        ASSERT_TRUE( "Hello again" == built->GetProperty( GUI::EWidgetProperty_Text ) && GUI::CColour( 1, 2, 3, 255 ) == built->GetColourProperty( "TextColour" ) && "x" == built->GetProperty( "ExoticHint" ) );

        // the handler gets what changed in one go, in the order of the names, the properties without an id in between
        GUI::CWidget* label = context->CreateWidget( "Label", "Order", GUCEF_NULL );
        context->FlushChanges();
        label->SetPropertyApplierOverride( "Text", &RecordApplied );
        label->SetPropertyApplierOverride( "Size", &RecordApplied );
        label->SetPropertyApplierOverride( "Opacity", &RecordApplied );
        label->SetPropertyApplierOverride( "BCustom", &RecordApplied );
        label->SetPropertyApplierOverride( "ZCustom", &RecordApplied );
        label->SetPropertyApplierOverride( "Tooltip", &RecordApplied );
        context->FlushChanges();
        g_applied.clear();
        label->SetProperty( "ZCustom", "7" );
        label->SetProperty( GUI::EWidgetProperty_Tooltip, "6" );
        label->SetProperty( "Text", "5" );
        label->SetProperty( "BCustom", "2" );
        label->SetProperty( GUI::EWidgetProperty_Size, "0,1 0,1" );
        label->SetProperty( GUI::EWidgetProperty_Opacity, "0.5" );
        context->FlushChanges();
        ASSERT_TRUE( 6 == g_applied.size() );
        if ( 6 == g_applied.size() )
        {
            ASSERT_TRUE( "2" == g_applied[ 0 ] );               // BCustom
            ASSERT_TRUE( "0.5" == g_applied[ 1 ] );             // Opacity
            ASSERT_TRUE( "0,1 0,1" == g_applied[ 2 ] );         // Size
            ASSERT_TRUE( "5" == g_applied[ 3 ] );               // Text
            ASSERT_TRUE( "6" == g_applied[ 4 ] );               // Tooltip
            ASSERT_TRUE( "7" == g_applied[ 5 ] );               // ZCustom
        }
        g_applied.clear();
        context->FlushChanges();
        ASSERT_TRUE( g_applied.empty() );
        context->DestroyWidget( label );
        context->DestroyWidget( built );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
