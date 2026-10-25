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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H
#include "guidriverNull_CNullWidgetHandler.h"
#define GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H
#endif /* GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
#include <emscripten.h>
#include "guidriverNull_pluginAPI.h"
#include "guidriverRmlUIOpenGL_pluginAPI.h"
#include "guidriverEmscripten_pluginAPI.h"
#include "inputdriverEMSCRIPTEN_pluginAPI.h"
#include "fontpluginFREETYPE_pluginAPI.h"
#endif

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

/*-------------------------------------------------------------------------*/

bool
LoadTestPlugin( const char* moduleName, const char* pluginGroup )
{
    CORE::CString release = CORE::CString( "$MODULEDIR$/" ) + moduleName;
    CORE::CString debug = release + "_d";
    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString& first = debug;
    const CORE::CString& second = release;
    #else
    const CORE::CString& first = release;
    const CORE::CString& second = debug;
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    return pluginControl.AddPluginFromDir( first, pluginGroup, true ) || pluginControl.AddPluginFromDir( second, pluginGroup, true );
}

/*-------------------------------------------------------------------------*/

bool
IsInBrowser( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
    return 0 != EM_ASM_INT( { return ( typeof document !== "undefined" && document.querySelector( "#canvas" ) ) ? 1 : 0; } );
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateNullContext( void )
{
    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    if ( GUCEF_NULL == manager.GetGuiDriver( "Null" ) )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
        GUIDRIVERNULL::guidriverNull_GUCEFPlugin_Load( 0, GUCEF_NULL );
        #else
        LoadTestPlugin( "guidriverNull", "guidriver" );
        #endif
    }
    GUI::TGuiContextPtr context = manager.CreateGUIContext( "Null", GUI::TWindowContextPtr() );
    GUI::CGUIContext* guiContext = ( context.IsNULL() ? GUCEF_NULL : context->AsGUIContext() );
    if ( GUCEF_NULL != guiContext )
        guiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateRmlUiContext( void )
{
    return CreateRmlUiContextInWindow( 640, 480 );
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CreateRmlUiContextInWindow( UInt32 windowWidth, UInt32 windowHeight )
{
    GUI::CGUIManager& manager = GUI::CGuiGlobal::Instance()->GetGuiManager();
    GUI::CWindowManager& windows = GUI::CGuiGlobal::Instance()->GetWindowManager();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    // in a browser: a WebGL 2 canvas of the page; node has no page
    if ( !IsInBrowser() )
        return GUI::TGuiContextPtr();
    const char* backendName = "Emscripten";
    if ( windows.GetBackend( backendName ).IsNULL() )
        GUIDRIVEREMSCRIPTEN::guidriverEmscripten_GUCEFPlugin_Load( 0, GUCEF_NULL );
    if ( GUCEF_NULL == manager.GetGuiDriver( "RmlUi" ) )
        GUIDRIVERRMLUIGL::guidriverRmlUIOpenGL_GUCEFPlugin_Load( 0, GUCEF_NULL );

    #else

    // an offscreen window
    const char* backendName = "EGL";
    if ( windows.GetBackend( backendName ).IsNULL() )
        LoadTestPlugin( "guidriverEGL", "guidriver" );
    if ( GUCEF_NULL == manager.GetGuiDriver( "RmlUi" ) )
        LoadTestPlugin( "guidriverRmlUIOpenGL", "guidriver" );

    #endif

    GUI::TWindowManagerBackendPtr backend = windows.GetBackend( backendName );
    if ( backend.IsNULL() || GUCEF_NULL == manager.GetGuiDriver( "RmlUi" ) )
        return GUI::TGuiContextPtr();
    GUI::TWindowContextPtr window = backend->CreateWindowContext( "Test", windowWidth, windowHeight, false );
    if ( window.IsNULL() )
        return GUI::TGuiContextPtr();
    GUI::TGuiContextPtr context = manager.CreateGUIContext( "RmlUi", window );
    GUI::CGUIContext* guiContext = ( context.IsNULL() ? GUCEF_NULL : context->AsGUIContext() );
    if ( GUCEF_NULL != guiContext )
        guiContext->SetLogicalResolution( 640, 480 );
    return context;
}

/*-------------------------------------------------------------------------*/

IMAGE::TPixelMapPtr
MakeSolidImage( UInt32 width, UInt32 height, UInt8 r, UInt8 g, UInt8 b, UInt8 a )
{
    IMAGE::TPixelMapPtr pixels( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, width, height, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8* data = static_cast< UInt8* >( pixels->GetDataPtr() );
    for ( UInt32 i=0; i<width*height; ++i )
    {
        data[ i*4 ] = r;
        data[ i*4+1 ] = g;
        data[ i*4+2 ] = b;
        data[ i*4+3 ] = a;
    }
    return pixels;
}

/*-------------------------------------------------------------------------*/

GUI::TBitmapFontPtr
MakeTestFont( Int32 width, Int32 height )
{
    // a white atlas with one cell per glyph; every glyph is a full block
    UInt32 count = 126 - 32 + 1;
    GUI::TBitmapFontPtr font( GUCEF_NEW GUI::CBitmapFont() );
    font->SetAtlas( MakeSolidImage( (UInt32) width * count, (UInt32) height, 255, 255, 255, 255 ) );
    for ( UInt32 cp=32; cp<=126; ++cp )
    {
        GUI::CBitmapFont::TGlyph glyph;
        glyph.source = GUI::CRect( (Int32) ( cp - 32 ) * width, 0, width, height );
        glyph.advance = width;
        font->SetGlyph( cp, glyph );
    }
    font->SetLineHeight( height );
    font->SetBaseline( height );
    font->SetFallbackCodePoint( '?' );
    return font;
}

/*-------------------------------------------------------------------------*/

bool
LoadFreeTypeFontPlugin( void )
{
    GUI::CFontRasterizerRegistry& registry = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry();
    if ( GUCEF_NULL == registry.Get( "FreeType" ) )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )
        FONTPLUGINFREETYPE::fontpluginFREETYPE_GUCEFPlugin_Load( 0, GUCEF_NULL );
        #else
        LoadTestPlugin( "fontpluginFREETYPE", "fontplugin" );
        #endif
    }
    return GUCEF_NULL != registry.Get( "FreeType" );
}

/*-------------------------------------------------------------------------*/

bool
LoadLayoutCodecs( void )
{
    CORE::CDStoreCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry();
    if ( !registry.IsRegistered( "xml" ) )
        LoadTestPlugin( "dstorepluginPARSIFALXML", "dstoreplugin" );
    if ( !registry.IsRegistered( "json" ) )
        LoadTestPlugin( "dstorepluginJSONPARSER", "dstoreplugin" );
    return registry.IsRegistered( "xml" ) && registry.IsRegistered( "json" );
}

/*-------------------------------------------------------------------------*/

namespace {

/** Records the events of the widgets it observes; can mark one event of one widget handled */
class CEventRecorder : public CORE::CObserver
{
    public:

    struct SRecord
    {
        GUI::CWidget* notifier;
        CORE::CEvent eventId;
        GUI::TEventPhase phase;
        Int32 localX;
        Int32 localY;
        UInt32 clickCount;
    };
    typedef GUCEF::vector< SRecord > TRecordVector;

    TRecordVector records;
    GUI::CWidget* handleAt;
    CORE::CEvent handleEvent;

    CEventRecorder( void )
        : CORE::CObserver()
        , records()
        , handleAt( GUCEF_NULL )
        , handleEvent()
    {
    }

    virtual ~CEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    void Watch( GUI::CWidget* widget )
    {
        SubscribeTo( widget );
    }

    UInt32 Count( const CORE::CEvent& eventId, GUI::CWidget* notifier = GUCEF_NULL ) const
    {
        UInt32 count = 0;
        for ( UInt32 i=0; i<(UInt32)records.size(); ++i )
            if ( records[ i ].eventId == eventId && ( GUCEF_NULL == notifier || records[ i ].notifier == notifier ) )
                ++count;
        return count;
    }

    const SRecord* Find( const CORE::CEvent& eventId, GUI::CWidget* notifier ) const
    {
        for ( UInt32 i=0; i<(UInt32)records.size(); ++i )
            if ( records[ i ].eventId == eventId && records[ i ].notifier == notifier )
                return &records[ i ];
        return GUCEF_NULL;
    }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
        if ( GUCEF_NULL == routed )
            return;
        SRecord record;
        record.notifier = static_cast< GUI::CWidget* >( notifier );
        record.eventId = eventId;
        record.phase = routed->phase;
        record.localX = record.localY = -1;
        record.clickCount = 0;
        GUI::CPointerEventData* pointer = CORE::CICloneable::CloneableAs< GUI::CPointerEventData >( eventData );
        if ( GUCEF_NULL != pointer )
        {
            record.localX = pointer->localX;
            record.localY = pointer->localY;
            record.clickCount = pointer->clickCount;
        }
        records.push_back( record );
        if ( record.notifier == handleAt && eventId == handleEvent )
            routed->handled = true;
    }
};

/*-------------------------------------------------------------------------*/

/** Destroys a form when a widget of it is activated, the way a screen closes itself from a button */
class CFormCloser : public CORE::CObserver
{
    public:

    GUI::CForm* form;
    bool fired;

    CFormCloser( GUI::CForm* f ) : CORE::CObserver(), form( f ), fired( false ) {}

    virtual ~CFormCloser() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CWidget::ActivatedEvent == eventId && GUCEF_NULL != form )
        {
            fired = true;
            delete form;
            form = GUCEF_NULL;
        }
    }
};

GUIDRIVERNULL::TNullNative*
NativeOf( GUI::CWidget* widget )
{
    return static_cast< GUIDRIVERNULL::TNullNative* >( widget->GetNativeHandle() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiCoreTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Core" );

    GUCEF_TESTFW_TESTCASE( "Core: UDim, UVector, rectangles and colours read and write their text forms" )
    try
    {
        GUI::CUDim dim;
        ASSERT_TRUE( GUI::CUDim::FromString( "0.5,-20", dim ) );
        ASSERT_TRUE( 0.5f == dim.GetRelativeValue() && -20 == dim.GetPixelValue() );
        ASSERT_TRUE( 300 == dim.Resolve( 640 ) );
        ASSERT_TRUE( GUI::CUDim::FromString( "12", dim ) && 12 == dim.GetPixelValue() && 0.0f == dim.GetRelativeValue() );
        ASSERT_FALSE( GUI::CUDim::FromString( "a,b", dim ) );

        GUI::CUVector vec;
        ASSERT_TRUE( GUI::CUVector::FromString( "0,10 1,-5", vec ) );
        GUI::CUVector parsed;
        ASSERT_TRUE( GUI::CUVector::FromString( vec.ToString(), parsed ) && parsed == vec );

        GUI::CRect rect;
        ASSERT_TRUE( GUI::CRect::FromString( "1,2,3,4", rect ) && GUI::CRect( 1, 2, 3, 4 ) == rect );
        ASSERT_TRUE( rect.Contains( 1, 2 ) && rect.Contains( 3, 5 ) && !rect.Contains( 4, 2 ) );
        ASSERT_FALSE( GUI::CRect::FromString( "1,2,3", rect ) );

        GUI::CColour colour;
        ASSERT_TRUE( GUI::CColour::FromString( "#ff8000", colour ) && GUI::CColour( 255, 128, 0, 255 ) == colour );
        ASSERT_TRUE( GUI::CColour::FromString( "1,2,3", colour ) && GUI::CColour( 1, 2, 3, 255 ) == colour );
        ASSERT_FALSE( GUI::CColour::FromString( "1,2,300", colour ) );

        // presenting a 640x480 picture in other windows
        ASSERT_TRUE( GUI::CRect( 0, 0, 1280, 960 ) == GUI::CalculatePresentRect( 640, 480, 1280, 960, GUI::PRESENTMODE_NEAREST ) );
        ASSERT_TRUE( GUI::CRect( 160, 0, 1280, 960 ) == GUI::CalculatePresentRect( 640, 480, 1600, 960, GUI::PRESENTMODE_LINEAR ) );
        ASSERT_TRUE( GUI::CRect( 320, 60, 1280, 960 ) == GUI::CalculatePresentRect( 640, 480, 1920, 1080, GUI::PRESENTMODE_INTEGER ) );
        ASSERT_TRUE( GUI::CRect( 0, 0, 800, 500 ) == GUI::CalculatePresentRect( 640, 480, 800, 500, GUI::PRESENTMODE_STRETCH ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: The registry knows every widget type gucefGUI has, with inherited and validated properties" )
    try
    {
        GUI::CWidgetTypeRegistry& registry = GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry();
        const char* types[] = { "Widget", "Panel", "Form", "HBox", "VBox", "Grid", "Margin", "Window", "Label", "Button", "PushButton",
                                "Checkbox", "Editbox", "Textbox", "Listbox", "Combobox", "Spinner", "Slider", "Scrollbar", "ProgressBar",
                                "ImageFrame", "VideoView", "AudioPlayer", "DrawSurface", "RenderContext", "TabControl", "TabContentPane",
                                "Treeview", "MenuBar", "PopupMenu", "GridView" };
        for ( UInt32 i=0; i<sizeof( types )/sizeof( types[ 0 ] ); ++i )
            ASSERT_TRUE( registry.IsRegistered( types[ i ] ) );

        ASSERT_TRUE( registry.IsA( "VideoView", "ImageFrame" ) && registry.IsA( "VideoView", "Widget" ) && !registry.IsA( "Label", "Button" ) );
        // a derived type sees the properties of its bases, and may redefine one
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "Checkbox", "NormalImage" ) );
        ASSERT_TRUE( GUCEF_NULL != registry.FindProperty( "Checkbox", "Visible" ) );
        ASSERT_TRUE( "Vertical" == registry.FindProperty( "Scrollbar", "Orientation" )->defaultValue );
        ASSERT_TRUE( "Horizontal" == registry.FindProperty( "Slider", "Orientation" )->defaultValue );
        ASSERT_TRUE( GUCEF_NULL == registry.FindProperty( "Label", "NormalImage" ) );

        const GUI::CPropertyDescriptor* visible = registry.FindProperty( "Button", "Visible" );
        CORE::CString normalized;
        ASSERT_TRUE( visible->Validate( "Yes", normalized ) && "true" == normalized );
        ASSERT_FALSE( visible->Validate( "maybe", normalized ) );
        ASSERT_TRUE( registry.FindProperty( "Label", "HTextAlign" )->Validate( "center", normalized ) && "Center" == normalized );
        ASSERT_FALSE( registry.FindProperty( "Label", "HTextAlign" )->Validate( "middle", normalized ) );
        ASSERT_TRUE( registry.FindProperty( "Listbox", "DisabledItems" )->Validate( " 1, 3 ", normalized ) && "1,3" == normalized );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: The null driver gets every property once at creation and changes batched once per frame" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ( contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext() );
        ASSERT_TRUE( GUCEF_NULL != context );
        ASSERT_TRUE( GUCEF_NULL != context->GetDriver() && "Null" == context->GetDriver()->GetDriverName() );
        ASSERT_TRUE( GUI::CGUIDriver::InterfaceVersion == context->GetDriver()->GetInterfaceVersion() );
        ASSERT_FALSE( context->GetDriver()->HasCapability( "Render" ) );

        GUI::CWidget* label = context->CreateWidget( "Label", "Title", GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL != label && "Label" == label->GetType() && "Title" == label->GetName() );
        GUIDRIVERNULL::TNullNative* native = NativeOf( label );
        ASSERT_TRUE( GUCEF_NULL != native );

        context->FlushChanges();
        UInt32 propertyCount = (UInt32) GUI::CGuiGlobal::Instance()->GetWidgetTypeRegistry().GetProperties( "Label" ).size();
        ASSERT_TRUE( propertyCount == native->propertyApplyCount );
        ASSERT_TRUE( "true" == native->properties[ "Visible" ] );

        // three changes, one frame: one application with the last value
        label->SetProperty( "Text", "a" );
        label->SetProperty( "Text", "b" );
        label->SetProperty( "Text", "c" );
        ASSERT_TRUE( "c" == label->GetProperty( "Text" ) );
        ASSERT_TRUE( propertyCount == native->propertyApplyCount );
        context->RenderFrame( 16 );
        ASSERT_TRUE( propertyCount + 1 == native->propertyApplyCount && "c" == native->properties[ "Text" ] );

        // a value that does not fit the type is refused and nothing changes
        ASSERT_FALSE( label->SetProperty( "Visible", "perhaps" ) );
        ASSERT_TRUE( label->IsVisible() );

        // an unknown property is kept and passed on (a newer version or a backend hint)
        ASSERT_TRUE( label->SetProperty( "FutureProperty", "x" ) );
        context->FlushChanges();
        ASSERT_TRUE( "x" == native->properties[ "FutureProperty" ] );

        // a per widget override replaces the handler's applier for that widget only
        struct SOverride
        {
            static bool Apply( GUI::CWidgetHandler& handler, GUI::CWidget& widget, const CORE::CString& value )
            {
                NativeOf( &widget )->properties[ "Text" ] = "override:" + value;
                return true;
            }
        };
        label->SetPropertyApplierOverride( "Text", &SOverride::Apply );
        context->FlushChanges();
        ASSERT_TRUE( "override:c" == native->properties[ "Text" ] );

        context->DestroyWidget( label );
        ASSERT_TRUE( context->GetOwnedWidgets().empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: Widgets are laid out with UDims, alignment, natural sizes and the containers" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterImage( "Pic40x20", MakeSolidImage( 40, 20, 255, 0, 0, 255 ) );

        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* centered = context->CreateWidget( "Panel", "Centered", root );
        centered->SetProperty( "Position", "0.5,0 0.5,0" );
        centered->SetProperty( "Size", "0,200 0,100" );
        centered->SetProperty( "HAlign", "Center" );
        centered->SetProperty( "VAlign", "Center" );
        GUI::CWidget* button = context->CreateWidget( "Button", "Ok", centered );
        button->SetProperty( "NormalImage", "Pic40x20" );
        button->SetProperty( "Position", "1,-40 1,-20" );

        GUI::CWidget* hbox = context->CreateWidget( "HBox", "Row", root );
        hbox->SetProperty( "Position", "0,10 0,400" );
        hbox->SetProperty( "Padding", "2" );
        hbox->SetProperty( "Spacing", "5" );
        GUI::CWidget* a = context->CreateWidget( "ImageFrame", "A", hbox );
        a->SetProperty( "Image", "Pic40x20" );
        GUI::CWidget* b = context->CreateWidget( "Panel", "B", hbox );
        b->SetProperty( "Size", "0,30 0,10" );
        b->SetProperty( "VAlign", "Bottom" );

        GUI::CWidget* grid = context->CreateWidget( "Grid", "Grid", root );
        grid->SetProperty( "Position", "0,300 0,0" );
        grid->SetProperty( "Size", "0,100 0,100" );
        grid->SetProperty( "Columns", "2" );
        GUI::CWidget* cells[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
        {
            cells[ i ] = context->CreateWidget( "Panel", "Cell", grid );
            cells[ i ]->SetProperty( "Size", "0,10 0,20" );
        }

        context->FlushChanges();
        ASSERT_TRUE( GUI::CRect( 0, 0, 640, 480 ) == root->GetRect() );
        ASSERT_TRUE( GUI::CRect( 220, 190, 200, 100 ) == centered->GetRect() );
        // no size: the natural size, the picture's
        ASSERT_TRUE( GUI::CRect( 380, 270, 40, 20 ) == button->GetRect() );
        ASSERT_TRUE( GUI::CRect( 12, 402, 40, 20 ) == a->GetRect() );
        ASSERT_TRUE( GUI::CRect( 57, 412, 30, 10 ) == b->GetRect() );
        // the box itself has the size of its children
        ASSERT_TRUE( GUI::CRect( 10, 400, 2 + 40 + 5 + 30 + 2, 24 ) == hbox->GetRect() );
        ASSERT_TRUE( GUI::CRect( 300, 0, 50, 20 ) == cells[ 0 ]->GetRect() );
        ASSERT_TRUE( GUI::CRect( 350, 0, 50, 20 ) == cells[ 1 ]->GetRect() );
        ASSERT_TRUE( GUI::CRect( 300, 20, 50, 20 ) == cells[ 2 ]->GetRect() );
        // the handler is told where the widget is
        ASSERT_TRUE( button->GetRect() == NativeOf( button )->rect );

        // a hidden child takes no place in a box
        a->SetVisibility( false );
        context->FlushChanges();
        // the box is only as high as B now, so B, at the bottom, moves up as well
        ASSERT_TRUE( GUI::CRect( 12, 402, 30, 10 ) == b->GetRect() );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: A click goes through tunnel, target and bubble with local coordinates, and a handled event stops" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* panel = context->CreateWidget( "Panel", "Panel", root );
        panel->SetProperty( "Position", "0,100 0,100" );
        panel->SetProperty( "Size", "0,200 0,200" );
        GUI::CWidget* button = context->CreateWidget( "Button", "Ok", panel );
        button->SetProperty( "Position", "0,10 0,20" );
        button->SetProperty( "Size", "0,50 0,30" );
        ASSERT_TRUE( context->ShowRoot( root, false ) );

        CEventRecorder recorder;
        recorder.Watch( root );
        recorder.Watch( panel );
        recorder.Watch( button );

        context->InjectMouseMove( 120, 130 );
        ASSERT_TRUE( button->IsHovered() && button == context->GetHovered() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( button->IsPressed() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_FALSE( button->IsPressed() );

        // the down event: root (tunnel), panel (tunnel), button (target), panel (bubble), root (bubble)
        UInt32 downs = 0;
        GUI::CWidget* expectedOrder[] = { root, panel, button, panel, root };
        GUI::TEventPhase expectedPhase[] = { GUI::EVENTPHASE_TUNNEL, GUI::EVENTPHASE_TUNNEL, GUI::EVENTPHASE_TARGET, GUI::EVENTPHASE_BUBBLE, GUI::EVENTPHASE_BUBBLE };
        for ( UInt32 i=0; i<(UInt32)recorder.records.size(); ++i )
        {
            if ( recorder.records[ i ].eventId != GUI::CWidget::MouseDownEvent )
                continue;
            ASSERT_TRUE( downs < 5 );
            ASSERT_TRUE( recorder.records[ i ].notifier == expectedOrder[ downs ] && recorder.records[ i ].phase == expectedPhase[ downs ] );
            ++downs;
        }
        ASSERT_TRUE( 5 == downs );
        // local coordinates for each widget that is notified
        const CEventRecorder::SRecord* atButton = recorder.Find( GUI::CWidget::MouseClickEvent, button );
        ASSERT_TRUE( GUCEF_NULL != atButton && 10 == atButton->localX && 10 == atButton->localY && 1 == atButton->clickCount );
        const CEventRecorder::SRecord* atPanel = recorder.Find( GUI::CWidget::MouseClickEvent, panel );
        ASSERT_TRUE( GUCEF_NULL != atPanel && 20 == atPanel->localX && 30 == atPanel->localY );
        // activation is routed as well: two tunnel, the target, two bubble
        ASSERT_TRUE( 5 == recorder.Count( GUI::CWidget::ActivatedEvent ) );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::MouseFocusReceivedEvent, button ) );

        // the panel handles the down on its way in: the button never sees it
        recorder.records.clear();
        recorder.handleAt = panel;
        recorder.handleEvent = GUI::CWidget::MouseDownEvent;
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 0 == recorder.Count( GUI::CWidget::MouseDownEvent, button ) && 1 == recorder.Count( GUI::CWidget::MouseDownEvent, root ) );
        recorder.handleAt = GUCEF_NULL;

        // that second click came soon after the first: a double click; a third one makes a triple click
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::MouseDoubleClickEvent, button ) );
        recorder.records.clear();
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        const CEventRecorder::SRecord* tripleClick = recorder.Find( GUI::CWidget::MouseTripleClickEvent, button );
        ASSERT_TRUE( GUCEF_NULL != tripleClick && 3 == tripleClick->clickCount );
        // after a pause the count starts again
        context->RenderFrame( 1000 );

        // released somewhere else: no click
        recorder.records.clear();
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( 300, 300 );
        ASSERT_FALSE( button->IsHovered() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::MouseUpEvent, button ) && 0 == recorder.Count( GUI::CWidget::MouseClickEvent, button ) );

        // a right click is a click without activation
        recorder.records.clear();
        context->InjectMouseMove( 120, 130 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_RIGHT, false );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::MouseClickEvent, button ) && 0 == recorder.Count( GUI::CWidget::ActivatedEvent ) );

        // a disabled button gets nothing
        recorder.records.clear();
        button->SetEnabled( false );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 0 == recorder.Count( GUI::CWidget::MouseDownEvent, button ) );
        ASSERT_TRUE( 0 != ( button->GetState() & GUI::CWidget::STATE_DISABLED ) );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: A modal root keeps the input, a form can destroy itself from its own button" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );

        GUI::CWidget* back = context->CreateWidget( "Panel", "Back", GUCEF_NULL );
        back->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* backButton = context->CreateWidget( "Button", "BackButton", back );
        backButton->SetProperty( "Size", "0,100 0,100" );
        context->ShowRoot( back, false );

        GUI::CForm* dialog = GUCEF_NEW GUI::CForm( context );
        CORE::CDataNode layout( "GUILayout" );
        CORE::CDataNode* panelNode = layout.AddChild( "Widget" );
        panelNode->SetAttribute( "type", "Panel" );
        panelNode->SetAttribute( "name", "Dialog" );
        panelNode->SetAttribute( "position", "0,200 0,200" );
        panelNode->SetAttribute( "size", "0,100 0,50" );
        CORE::CDataNode* yesNode = panelNode->AddChild( "Widget" );
        yesNode->SetAttribute( "type", "Button" );
        yesNode->SetAttribute( "name", "Yes" );
        yesNode->SetAttribute( "size", "0,40 0,20" );
        ASSERT_TRUE( dialog->LoadLayout( layout ) );
        ASSERT_FALSE( dialog->IsVisible() );
        ASSERT_TRUE( dialog->ShowModal() && dialog->IsVisible() );
        ASSERT_TRUE( dialog->GetRootWidget() == context->GetModalRoot() );

        CEventRecorder recorder;
        recorder.Watch( backButton );
        context->InjectMouseMove( 50, 50 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( recorder.records.empty() );
        ASSERT_TRUE( GUCEF_NULL == context->HitTest( 50, 50 ) );

        CFormCloser closer( dialog );
        closer.SubscribeTo( dialog->GetWidget( "Yes" ) );
        context->InjectMouseMove( 210, 210 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( closer.fired && GUCEF_NULL == closer.form );
        ASSERT_TRUE( GUCEF_NULL == context->GetModalRoot() );
        ASSERT_TRUE( 2 == context->GetOwnedWidgets().size() );

        // the back panel takes input again
        context->InjectMouseMove( 50, 50 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::ActivatedEvent, backButton ) );

        context->DestroyWidget( back );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: The keyboard focus, Tab order, arrow navigation and Enter on a focused button" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* topLeft = context->CreateWidget( "Button", "TopLeft", root );
        topLeft->SetProperty( "Position", "0,0 0,0" );
        topLeft->SetProperty( "Size", "0,50 0,20" );
        GUI::CWidget* topRight = context->CreateWidget( "Button", "TopRight", root );
        topRight->SetProperty( "Position", "0,200 0,0" );
        topRight->SetProperty( "Size", "0,50 0,20" );
        GUI::CWidget* bottom = context->CreateWidget( "Button", "Bottom", root );
        bottom->SetProperty( "Position", "0,10 0,200" );
        bottom->SetProperty( "Size", "0,50 0,20" );
        GUI::CWidget* label = context->CreateWidget( "Label", "Label", root );
        context->ShowRoot( root, false );

        // Tab follows TabIndex first
        bottom->SetIntProperty( "TabIndex", 0 );
        topRight->SetIntProperty( "TabIndex", 1 );
        ASSERT_FALSE( label->IsFocusable() );
        context->InjectKey( INPUT::KEYCODE_TAB, true, 0 );
        ASSERT_TRUE( bottom == context->GetFocus() && bottom->HasFocus() );
        context->InjectKey( INPUT::KEYCODE_TAB, true, 0 );
        ASSERT_TRUE( topRight == context->GetFocus() );
        context->InjectKey( INPUT::KEYCODE_TAB, true, 0 );
        ASSERT_TRUE( topLeft == context->GetFocus() );
        context->InjectKey( INPUT::KEYCODE_TAB, true, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( topRight == context->GetFocus() );

        // arrows go to the nearest widget in that direction, an explicit neighbour wins
        context->InjectKey( INPUT::KEYCODE_LEFT, true, 0 );
        ASSERT_TRUE( topLeft == context->GetFocus() );
        context->InjectKey( INPUT::KEYCODE_DOWN, true, 0 );
        ASSERT_TRUE( bottom == context->GetFocus() );
        bottom->SetProperty( "NavUp", "TopRight" );
        context->InjectKey( INPUT::KEYCODE_UP, true, 0 );
        ASSERT_TRUE( topRight == context->GetFocus() );

        // Enter activates the focused button
        CEventRecorder recorder;
        recorder.Watch( topRight );
        context->InjectKey( INPUT::KEYCODE_RETURN, true, 0 );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::ActivatedEvent, topRight ) );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::KeyDownEvent, topRight ) );
        ASSERT_TRUE( 1 == recorder.Count( GUI::CWidget::NavigationEvent, topRight ) );

        // a key handled by an observer is not turned into navigation
        recorder.handleAt = topRight;
        recorder.handleEvent = GUI::CWidget::KeyDownEvent;
        context->InjectKey( INPUT::KEYCODE_LEFT, true, 0 );
        ASSERT_TRUE( topRight == context->GetFocus() );

        // hiding the focused widget takes the focus away
        topRight->SetVisibility( false );
        ASSERT_TRUE( GUCEF_NULL == context->GetFocus() );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: A theme sets values by type, class, name and state, and a state change reaches the driver" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        CORE::CDataNode theme( "GUITheme" );
        CORE::CDataNode* style = theme.AddChild( "Style" );
        style->SetAttribute( "type", "Button" );
        CORE::CDataNode* property = style->AddChild( "Property" );
        property->SetAttribute( "name", "TextColour" );
        property->SetAttribute( "value", "10,10,10,255" );
        style = theme.AddChild( "Style" );
        style->SetAttribute( "type", "Button" );
        style->SetAttribute( "state", "hover" );
        property = style->AddChild( "Property" );
        property->SetAttribute( "name", "TextColour" );
        property->SetAttribute( "value", "255,255,0,255" );
        style = theme.AddChild( "Style" );
        style->SetAttribute( "class", "Special" );
        property = style->AddChild( "Property" );
        property->SetAttribute( "name", "TextColour" );
        property->SetAttribute( "value", "1,2,3,255" );
        ASSERT_TRUE( context->SetTheme( theme ) );

        GUI::CWidget* root = context->CreateWidget( "Panel", "Root", GUCEF_NULL );
        root->SetProperty( "Size", "1,0 1,0" );
        GUI::CWidget* plain = context->CreateWidget( "Button", "Plain", root );
        plain->SetProperty( "Size", "0,50 0,20" );
        GUI::CWidget* special = context->CreateWidget( "Button", "Special", root );
        special->SetProperty( "Position", "0,100 0,0" );
        special->SetProperty( "Size", "0,50 0,20" );
        special->SetProperty( "StyleClass", "Big Special" );
        GUI::CWidget* own = context->CreateWidget( "Button", "Own", root );
        own->SetProperty( "Position", "0,200 0,0" );
        own->SetProperty( "TextColour", "9,9,9,255" );
        context->ShowRoot( root, false );
        context->FlushChanges();

        ASSERT_TRUE( "10,10,10,255" == plain->GetProperty( "TextColour" ) );
        ASSERT_TRUE( "1,2,3,255" == special->GetProperty( "TextColour" ) );
        ASSERT_TRUE( "9,9,9,255" == own->GetProperty( "TextColour" ) );

        context->InjectMouseMove( 10, 10 );
        ASSERT_TRUE( "255,255,0,255" == plain->GetProperty( "TextColour" ) );
        context->FlushChanges();
        ASSERT_TRUE( "255,255,0,255" == NativeOf( plain )->properties[ "TextColour" ] );
        context->InjectMouseMove( 400, 400 );
        context->FlushChanges();
        ASSERT_TRUE( "10,10,10,255" == NativeOf( plain )->properties[ "TextColour" ] );

        context->DestroyWidget( root );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Core: Images register as RGBA with a revision, bitmap fonts measure and break text" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );

        IMAGE::TPixelMapPtr rgb( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 4, 2, IMAGE::PSF_RGB, GUCEF::DATATYPE_UINT8 ) );
        memset( rgb->GetDataPtr(), 200, rgb->GetTotalSizeInBytes() );
        ASSERT_TRUE( context->RegisterImage( "Rgb", rgb ) );
        IMAGE::TPixelMapPtr resolved;
        UInt32 revision = 0;
        ASSERT_TRUE( context->ResolveImage( "Rgb", resolved, revision ) && 1 == revision );
        ASSERT_TRUE( IMAGE::PSF_RGBA == resolved->GetPixelStorageFormat() && 4 == resolved->GetWidthInPixels() );
        ASSERT_TRUE( 255 == static_cast< UInt8* >( resolved->GetDataPtr() )[ 3 ] && 200 == static_cast< UInt8* >( resolved->GetDataPtr() )[ 0 ] );
        ASSERT_TRUE( context->UpdateImage( "Rgb" ) );
        ASSERT_TRUE( context->ResolveImage( "Rgb", resolved, revision ) && 2 == revision );
        ASSERT_FALSE( context->ResolveImage( "NoSuchImageAnywhere.png", resolved, revision ) );
        ASSERT_TRUE( context->MakeUniqueImageAlias( "x" ) != context->MakeUniqueImageAlias( "x" ) );

        GUI::TBitmapFontPtr font = MakeTestFont( 8, 10 );
        ASSERT_TRUE( context->RegisterFont( "Test", font ) );
        Int32 w = 0, h = 0;
        ASSERT_TRUE( context->MeasureText( "Test", "abc", 0, w, h ) && 24 == w && 10 == h );
        ASSERT_TRUE( 10 == context->GetLineHeight( "Test" ) );
        ASSERT_TRUE( 16 == context->GetLineHeight( "Unknown" ) );
        font->SetSpacing( 1 );
        ASSERT_TRUE( 26 == font->GetTextWidth( "abc" ) );
        font->SetSpacing( 0 );
        // breaking: on newlines, between words, and inside a word too wide for a line
        GUI::CBitmapFont::TStringVector lines = font->BreakLines( "one two three\nfour", 72 );
        ASSERT_TRUE( 3 == lines.size() && "one two" == lines[ 0 ] && "three" == lines[ 1 ] && "four" == lines[ 2 ] );
        lines = font->BreakLines( "abcdefghij", 32 );
        ASSERT_TRUE( 3 == lines.size() && "abcd" == lines[ 0 ] && "efgh" == lines[ 1 ] && "ij" == lines[ 2 ] );
        // a code point the font lacks is drawn as the fallback
        ASSERT_TRUE( GUCEF_NULL != font->ResolveGlyph( 0x263A ) && GUCEF_NULL == font->GetGlyph( 0x263A ) );

        // a label without a size is as large as its text
        GUI::CWidget* label = context->CreateWidget( "Label", "Label", GUCEF_NULL );
        label->SetProperty( "Font", "Test" );
        label->SetProperty( "Text", "hello" );
        context->FlushChanges();
        ASSERT_TRUE( 40 == label->GetRect().width && 10 == label->GetRect().height );
        context->DestroyWidget( label );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
