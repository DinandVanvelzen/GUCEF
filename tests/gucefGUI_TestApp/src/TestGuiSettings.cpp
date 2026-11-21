/*
 *  gucefGUI_TestApp: GUCEF test application for the GUI module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_CScrollPanel.h"
#include "gucefGUI_CListBox.h"
#include "gucefGUI_CInputSettingsForm.h"
#include "gucefGUI_CITooltipTextProvider.h"
#include "gucefGUI_TestCommon.h"
#include "gucefGUI_TestAssetFormSupport.h"

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

GUI::CVideoOptions::TDisplayMode
Mode( UInt32 width, UInt32 height, UInt32 frequency )
{
    GUI::CVideoOptions::TDisplayMode mode;
    mode.widthInPixels = width;
    mode.heightInPixels = height;
    mode.depthInBits = 32;
    mode.frequency = frequency;
    return mode;
}

/** Video settings with every field away from its default */
GUI::CVideoSettings
MakeChangedVideoSettings( void )
{
    GUI::CVideoSettings settings;
    settings.SetFullscreenState( true );
    settings.SetVSyncState( true );
    settings.SetResolution( 1920, 1080, 24 );
    settings.SetFrequency( 144 );
    settings.SetAntiAliasingFactor( 4 );
    settings.SetPresentMode( GUI::PRESENTMODE_INTEGER );
    settings.SetPresentFilter( "fsr1" );
    GUI::CVideoSettings::TParameterMap parameters;
    parameters[ "sharpness" ] = "0.25";
    settings.SetPresentFilterParameters( parameters );
    settings.SetImageUpscaler( "mmpx" );
    settings.SetMovieUpscaling( GUI::CVideoSettings::MovieUpscalingConverted );
    settings.SetRenderScale( 0 );
    settings.SetRenderer( "Vulkan" );
    return settings;
}

/** A logger that counts the messages of the user log: the console of a test */
class CUserMessageCounter : public CORE::CILogger
{
    public:

    UInt32 userMessages;
    UInt32 otherMessages;

    CUserMessageCounter( void ) : CORE::CILogger(), userMessages( 0 ), otherMessages( 0 ), m_minLogLevel( CORE::LOGLEVEL_EVERYTHING ) {}

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString&, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Count( logMsgType ); }
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream&, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Count( logMsgType ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString&, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Count( logMsgType ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream&, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Count( logMsgType ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE { m_minLogLevel = logLevel; }
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_minLogLevel; }

    private:

    void Count( const CORE::TLogMsgType logMsgType ) { if ( CORE::LOG_USER == logMsgType ) ++userMessages; else ++otherMessages; }

    Int32 m_minLogLevel;
};

/** Counts one event of one notifier */
class CEventCount : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;

    CEventCount( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 )
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CEventCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id == eventId )
            ++count;
    }
};

/** A page of a scroll panel that is taller than the window and a field */
class CScrollingTestPage : public GUI::CSettingsPageForm
{
    public:

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Scrolling"; }

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE { SetHasChanges( false ); }

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE { SetHasChanges( false ); return true; }

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "ScrollingPage", "0,0 0,0", "1,0 1,0" );
        CORE::CDataNode* body = GUI::CFormLayout::AddWidgetNode( *page, "ScrollPanel", "Body", "0,0 0,0", "1,0 1,0" );
        CORE::CDataNode* name = GUI::CFormLayout::AddWidgetNode( *body, "Editbox", "Name", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *name, "Text", "a" );
        GUI::CFormLayout::AddWidgetNode( *body, "Button", "Tall", "0,0 0,40", "1,0 0,2000" );
    }
};

/** A page that tries the scope of reading settings: what IsReadingSettings() says inside and after it */
class CReadingScopeProbePage : public GUI::CSettingsPageForm
{
    public:

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE { return "Probe"; }

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE { SetHasChanges( false ); }

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE { SetHasChanges( false ); return true; }

    bool Reading( void ) const { return IsReadingSettings(); }

    /** Nested scopes: reading inside each, and the state before is back when they end */
    bool NestingWorks( void )
    {
        if ( IsReadingSettings() )
            return false;
        {
            CReadingSettingsScope outer( *this );
            if ( !IsReadingSettings() )
                return false;
            {
                CReadingSettingsScope inner( *this );
                if ( !IsReadingSettings() )
                    return false;
            }
            if ( !IsReadingSettings() )
                return false;
        }
        return !IsReadingSettings();
    }

    /** Leaves the block early */
    bool ReturnsEarly( void )
    {
        CReadingSettingsScope scope( *this );
        return IsReadingSettings();
    }

    /** An exception that unwinds the block */
    void Throws( void )
    {
        CReadingSettingsScope scope( *this );
        throw 42;
    }

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUI::CFormLayout::AddWidgetNode( layout, "Panel", "ProbePage", "0,0 0,0", "1,0 1,0" );
    }
};

/** A click of the user in the middle of a widget */
void
ClickWidget( GUI::CGUIContext* context, GUI::CWidget* widget )
{
    context->FlushChanges();
    const GUI::CRect& rect = widget->GetRect();
    context->InjectMouseMove( rect.left + rect.width / 2, rect.top + rect.height / 2 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
}

/** The user opens the list of a combo box and clicks an entry */
bool
PickItem( GUI::CGUIContext* context, GUI::CWidget* combobox, Int32 index )
{
    if ( GUCEF_NULL == combobox )
        return false;
    ClickWidget( context, combobox );
    context->FlushChanges();
    GUI::CListbox* list = GUI::CWidget::As< GUI::CListbox >( context->GetPopup() );
    if ( GUCEF_NULL == list )
        return false;
    const GUI::CRect& rect = list->GetRect();
    Int32 rowHeight = list->GetRowHeight();
    context->InjectMouseMove( rect.left + 4, rect.top + index * rowHeight + rowHeight / 2 );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
    context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
    return true;
}

CORE::CString
ComboboxText( GUI::CSettingsPageForm* page, const CORE::CString& name )
{
    GUI::CCombobox* combobox = page->GetWidgetAs< GUI::CCombobox >( name );
    return GUCEF_NULL != combobox ? combobox->GetText() : CORE::CString::Empty;
}

bool
ComboboxHas( GUI::CSettingsPageForm* page, const CORE::CString& name, const CORE::CString& item )
{
    GUI::CCombobox* combobox = page->GetWidgetAs< GUI::CCombobox >( name );
    if ( GUCEF_NULL == combobox )
        return false;
    GUI::CCombobox::TStringVector items;
    combobox->GetListItems( items );
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( items[ i ] == item )
            return true;
    return false;
}

bool
IsEnabled( GUI::CForm* form, const CORE::CString& name )
{
    GUI::CWidget* widget = form->GetWidget( name );
    return GUCEF_NULL != widget && widget->GetBoolProperty( "Enabled" );
}

/** The input page in a settings form for the layout lint: the actions of an application with their keys */
class CInputSettingsScene : public GUITEST::CFormScene
{
    public:

    GUI::CKeyboardShortcutMap map;
    GUI::CInputSettingsForm page;
    GUI::CSettingsForm form;

    explicit CInputSettingsScene( GUI::CGUIContext* context ) : GUITEST::CFormScene(), map(), page(), form( context )
    {
        const char* ids[] = { "file", "folder", "fold", "inspector", "preview", "settings", "help", "console" };
        const char* titles[] = { "Open a file", "Open a folder", "Fold or unfold the resources", "Fold or unfold the inspector", "Preview layout on or off",
                                 "Open the settings", "Help of the place you are at", "Open or close the developer console" };
        const INPUT::KeyCode keys[] = { INPUT::KEYCODE_O, INPUT::KEYCODE_O, INPUT::KEYCODE_F9, INPUT::KEYCODE_F10, INPUT::KEYCODE_F11, INPUT::KEYCODE_COMMA, INPUT::KEYCODE_F1, INPUT::KEYCODE_GRAVE };
        const UInt32 modifiers[] = { INPUT::KEYMOD_CTRL, INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT, 0, 0, 0, INPUT::KEYMOD_CTRL, 0, 0 };
        page.SetShortcutMap( &map );
        for ( UInt32 i=0; i<8; ++i )
        {
            map.Bind( ids[ i ], keys[ i ], modifiers[ i ] );
            page.AddAction( ids[ i ], titles[ i ], keys[ i ], modifiers[ i ] );
        }
        form.AddPage( &page );
        form.Build();
        form.Open();
        context->FlushChanges();
    }
};

GUITEST::CFormScene*
BuildInputSettingsScene( GUI::CGUIContext* context )
{
    return new CInputSettingsScene( context );
}

/** A context of the null driver at 1280 x 960 in the dark theme, with the fixed width test font */
GUI::TGuiContextPtr
CreateSettingsContext( void )
{
    GUI::TGuiContextPtr contextPtr = CreateNullContext();
    GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
    if ( GUCEF_NULL != context )
    {
        context->SetLogicalResolution( 1280, 960 );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        CORE::CDataNode theme;
        GUI::BuildDarkTheme( theme, "Test" );
        context->SetTheme( theme );
    }
    return contextPtr;
}

bool
SameVideoSettings( const GUI::CVideoSettings& a, const GUI::CVideoSettings& b )
{
    return a.GetFullscreenState() == b.GetFullscreenState() && a.GetVSyncState() == b.GetVSyncState() &&
           a.GetResolutionWidthInPixels() == b.GetResolutionWidthInPixels() && a.GetResolutionHeightInPixels() == b.GetResolutionHeightInPixels() &&
           a.GetResolutionDepthInBits() == b.GetResolutionDepthInBits() && a.GetFrequency() == b.GetFrequency() &&
           a.GetAntiAliasingFactor() == b.GetAntiAliasingFactor() && a.GetPresentMode() == b.GetPresentMode() &&
           a.GetPresentFilter() == b.GetPresentFilter() && a.GetPresentFilterParameters() == b.GetPresentFilterParameters() &&
           a.GetImageUpscaler() == b.GetImageUpscaler() && a.GetMovieUpscaling() == b.GetMovieUpscaling() && a.GetRenderScale() == b.GetRenderScale() &&
           a.GetRenderer() == b.GetRenderer();
}

}

/*-------------------------------------------------------------------------*/

void
PerformGuiSettingsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Settings" );

    GUCEF_TESTFW_TESTCASE( "Settings: Video settings are saved as a config section and read back" )
    try
    {
        GUI::CVideoSettings settings = MakeChangedVideoSettings();
        GUI::CVideoSettings copy( settings );
        ASSERT_TRUE( SameVideoSettings( settings, copy ) );

        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( settings.SaveConfig( config ) );
        const CORE::CDataNode* section = config.FindChild( GUI::CVideoSettings::ConfigSectionName );
        ASSERT_TRUE( GUCEF_NULL != section && "integer" == section->GetAttributeValue( "presentMode" ).AsString() );

        // from the node that has the section and from the section itself
        GUI::CVideoSettings loaded;
        ASSERT_FALSE( SameVideoSettings( settings, loaded ) );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( SameVideoSettings( settings, loaded ) );
        GUI::CVideoSettings fromSection;
        ASSERT_TRUE( GUCEF_NULL != section && fromSection.LoadConfig( *section ) );
        ASSERT_TRUE( SameVideoSettings( settings, fromSection ) );

        // saving again replaces the section, parameters that are gone are gone from the config too
        GUI::CVideoSettings::TParameterMap noParameters;
        loaded.SetPresentFilterParameters( noParameters );
        ASSERT_TRUE( loaded.SaveConfig( config ) );
        ASSERT_TRUE( 1 == config.GetNrOfChildNodes() );
        GUI::CVideoSettings reloaded = MakeChangedVideoSettings();
        ASSERT_TRUE( reloaded.LoadConfig( config ) );
        ASSERT_TRUE( reloaded.GetPresentFilterParameters().size() == 1 );

        // through JSON, as an application config file has it
        if ( LoadLayoutCodecs() )
        {
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) );
            CORE::CDataNode jsonConfig( "Config" );
            ASSERT_TRUE( settings.SaveConfig( jsonConfig ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &jsonConfig, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            GUI::CVideoSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( SameVideoSettings( settings, fromJson ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: What a video settings section does not have stays as it was" )
    try
    {
        CORE::CDataNode config( "Config" );
        ASSERT_FALSE( GUI::CVideoSettings().LoadConfig( config ) );

        CORE::CDataNode* section = config.AddChild( GUI::CVideoSettings::ConfigSectionName );
        section->SetAttribute( "vsync", true );
        section->SetAttribute( "presentMode", "No such mode" );
        GUI::CVideoSettings settings = MakeChangedVideoSettings();
        settings.SetVSyncState( false );
        ASSERT_TRUE( settings.LoadConfig( config ) );
        ASSERT_TRUE( settings.GetVSyncState() );
        GUI::CVideoSettings expected = MakeChangedVideoSettings();
        ASSERT_TRUE( SameVideoSettings( expected, settings ) );

        // the names of the present modes
        for ( Int32 m=GUI::PRESENTMODE_NEAREST; m<=GUI::PRESENTMODE_STRETCH; ++m )
        {
            GUI::TPresentMode mode = (GUI::TPresentMode) m;
            ASSERT_TRUE( mode == GUI::CVideoSettings::StringToPresentMode( GUI::CVideoSettings::PresentModeToString( mode ), GUI::PRESENTMODE_STRETCH == mode ? GUI::PRESENTMODE_NEAREST : GUI::PRESENTMODE_STRETCH ) );
        }
        ASSERT_TRUE( GUI::PRESENTMODE_LINEAR == GUI::CVideoSettings::StringToPresentMode( "Linear", GUI::PRESENTMODE_NEAREST ) );
        ASSERT_TRUE( GUI::PRESENTMODE_INTEGER == GUI::CVideoSettings::StringToPresentMode( "", GUI::PRESENTMODE_INTEGER ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The common resolutions offered follow the modes of the display" )
    try
    {
        // no modes known: every common size for a window, none for full screen
        GUI::CVideoOptions options;
        GUI::CVideoOptions::TDisplayModeVector resolutions;
        options.GetCommonResolutions( false, resolutions );
        ASSERT_TRUE( resolutions.size() == GUI::CVideoOptions::GetCommonGameResolutions().size() );
        resolutions.clear();
        options.GetCommonResolutions( true, resolutions );
        ASSERT_TRUE( resolutions.empty() );

        options.AddDisplayMode( Mode( 1920, 1080, 60 ) );
        options.AddDisplayMode( Mode( 1920, 1080, 144 ) );
        options.AddDisplayMode( Mode( 1920, 1080, 60 ) );
        options.AddDisplayMode( Mode( 1280, 720, 60 ) );
        options.AddDisplayMode( Mode( 1000, 700, 60 ) );
        ASSERT_TRUE( 4 == options.GetDisplayModes().size() );

        // a window: the common sizes that fit in the largest mode, smallest first
        resolutions.clear();
        options.GetCommonResolutions( false, resolutions );
        ASSERT_TRUE( !resolutions.empty() && 640 == resolutions.front().widthInPixels );
        ASSERT_TRUE( 1920 == resolutions.back().widthInPixels && 1080 == resolutions.back().heightInPixels );
        for ( UInt32 i=0; i<(UInt32)resolutions.size(); ++i )
            ASSERT_TRUE( resolutions[ i ].widthInPixels <= 1920 && resolutions[ i ].heightInPixels <= 1080 );

        // full screen: the common sizes the display has, at its highest frequency for them
        resolutions.clear();
        options.GetCommonResolutions( true, resolutions );
        ASSERT_TRUE( 2 == resolutions.size() );
        if ( 2 == resolutions.size() )
        {
            ASSERT_TRUE( 1280 == resolutions[ 0 ].widthInPixels && 720 == resolutions[ 0 ].heightInPixels && 60 == resolutions[ 0 ].frequency );
            ASSERT_TRUE( 1920 == resolutions[ 1 ].widthInPixels && 1080 == resolutions[ 1 ].heightInPixels && 144 == resolutions[ 1 ].frequency );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The resolutions offered include the wide displays and the largest mode of the display" )
    try
    {
        // a super wide display of 3840 x 1080: its own size is offered, for a window and for full screen
        GUI::CVideoOptions wide;
        wide.AddDisplayMode( Mode( 3840, 1080, 120 ) );
        wide.AddDisplayMode( Mode( 3840, 1080, 60 ) );
        wide.AddDisplayMode( Mode( 1920, 1080, 120 ) );
        wide.AddDisplayMode( Mode( 1280, 720, 60 ) );

        GUI::CVideoOptions::TDisplayModeVector resolutions;
        wide.GetCommonResolutions( true, resolutions );
        ASSERT_TRUE( 3 == resolutions.size() );
        if ( 3 == resolutions.size() )
        {
            ASSERT_TRUE( 1280 == resolutions[ 0 ].widthInPixels && 1920 == resolutions[ 1 ].widthInPixels );
            ASSERT_TRUE( 3840 == resolutions[ 2 ].widthInPixels && 1080 == resolutions[ 2 ].heightInPixels && 120 == resolutions[ 2 ].frequency );
        }

        resolutions.clear();
        wide.GetCommonResolutions( false, resolutions );
        ASSERT_TRUE( !resolutions.empty() && 3840 == resolutions.back().widthInPixels && 1080 == resolutions.back().heightInPixels );
        for ( UInt32 i=0; i<(UInt32)resolutions.size(); ++i )
            ASSERT_TRUE( resolutions[ i ].heightInPixels <= 1080 );

        // a size that is not in the list of usual sizes is there as well, at its highest frequency, in place by its size
        GUI::CVideoOptions odd;
        odd.AddDisplayMode( Mode( 1920, 1080, 60 ) );
        odd.AddDisplayMode( Mode( 3000, 1000, 60 ) );
        odd.AddDisplayMode( Mode( 3000, 1000, 75 ) );
        resolutions.clear();
        odd.GetCommonResolutions( true, resolutions );
        ASSERT_TRUE( 2 == resolutions.size() );
        if ( 2 == resolutions.size() )
            ASSERT_TRUE( 3000 == resolutions[ 1 ].widthInPixels && 1000 == resolutions[ 1 ].heightInPixels && 75 == resolutions[ 1 ].frequency );
        resolutions.clear();
        odd.GetCommonResolutions( false, resolutions );
        UInt32 found = 0;
        for ( UInt32 i=0; i<(UInt32)resolutions.size(); ++i )
        {
            if ( 3000 == resolutions[ i ].widthInPixels && 1000 == resolutions[ i ].heightInPixels )
                ++found;
            if ( i > 0 )
                ASSERT_TRUE( 3000 != resolutions[ i - 1 ].widthInPixels || 3000 == resolutions[ i ].widthInPixels );
        }
        ASSERT_TRUE( 1 == found );

        // the wide sizes are among the usual ones
        const GUI::CVideoOptions::TDisplayModeVector& common = GUI::CVideoOptions::GetCommonGameResolutions();
        bool has3840x1080 = false, has5120x1440 = false, has3440x1440 = false;
        for ( UInt32 i=0; i<(UInt32)common.size(); ++i )
        {
            has3840x1080 = has3840x1080 || ( 3840 == common[ i ].widthInPixels && 1080 == common[ i ].heightInPixels );
            has5120x1440 = has5120x1440 || ( 5120 == common[ i ].widthInPixels && 1440 == common[ i ].heightInPixels );
            has3440x1440 = has3440x1440 || ( 3440 == common[ i ].widthInPixels && 1440 == common[ i ].heightInPixels );
        }
        ASSERT_TRUE( has3840x1080 && has5120x1440 && has3440x1440 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The settings form has a tab per page, Apply applies the pages with changes, Cancel puts back" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            // the console log window of the General page: no window in a test
            CORE::CString consoleLoggerEnv = CORE::GetEnv( "GUCEF_CONSOLE_LOGGER" );
            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", "none" );

            GUI::CSettingsForm form( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( form.AddPage( "Video" ) );
            GUI::CGeneralSettingsForm* general = static_cast< GUI::CGeneralSettingsForm* >( form.AddPage( "General" ) );
            ASSERT_TRUE( GUCEF_NULL != video && GUCEF_NULL != general );
            ASSERT_TRUE( GUCEF_NULL == form.AddPage( "No such page" ) );
            ASSERT_TRUE( 2 == form.GetPages().size() && video == form.GetPage( "Video" ) );

            GUI::CVideoOptions options;
            options.AddDisplayMode( Mode( 1920, 1080, 60 ) );
            options.AddDisplayMode( Mode( 1920, 1080, 144 ) );
            options.AddDisplayMode( Mode( 1280, 720, 60 ) );
            video->SetVideoOptions( options );
            GUI::CVideoSettings settings;
            settings.SetResolution( 1280, 960, 32 );
            video->SetVideoSettings( settings );
            GUI::CVideoSettingsForm::TStringVector filters;
            filters.push_back( "fsr1" );
            filters.push_back( "mmpx" );
            video->SetAvailablePresentFilters( filters );

            CEventCount formApplied( &form, GUI::CSettingsForm::SettingsAppliedEvent );
            CEventCount formClosed( &form, GUI::CSettingsForm::ClosedEvent );
            CEventCount videoApplied( video, GUI::CSettingsPageForm::SettingsAppliedEvent );
            CEventCount generalApplied( general, GUI::CSettingsPageForm::SettingsAppliedEvent );

            ASSERT_TRUE( form.Build() );
            ASSERT_FALSE( form.IsVisible() );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( form.IsVisible() );
            ASSERT_TRUE( "Video\nGeneral" == form.GetWidget( "Pages" )->GetProperty( "Tabs" ) );
            ASSERT_FALSE( IsEnabled( &form, "Apply" ) );

            // the video page shows the settings and offers only what there is
            ASSERT_TRUE( "Window" == ComboboxText( video, "DisplayMode" ) );
            ASSERT_TRUE( "1280 x 960" == ComboboxText( video, "Resolution" ) );
            ASSERT_TRUE( ComboboxHas( video, "Resolution", "1920 x 1080" ) && !ComboboxHas( video, "Resolution", "2560 x 1440" ) );
            ASSERT_TRUE( "None" == ComboboxText( video, "PresentFilter" ) && IsEnabled( video, "PresentFilter" ) );
            ASSERT_FALSE( IsEnabled( video, "Sharpness" ) );
            ASSERT_FALSE( IsEnabled( video, "ImageUpscaler" ) );
            ASSERT_FALSE( IsEnabled( video, "MovieUpscaling" ) );
            ASSERT_FALSE( video->HasChanges() );

            // the user picks a filter: the page has changes, Apply can be pressed, the sharpness can be set
            ASSERT_TRUE( PickItem( context, video->GetWidget( "PresentFilter" ), 1 ) );
            ASSERT_TRUE( "fsr1" == ComboboxText( video, "PresentFilter" ) );
            ASSERT_TRUE( video->HasChanges() && IsEnabled( &form, "Apply" ) && IsEnabled( video, "Sharpness" ) );

            // full screen: the resolutions are those of the display, the one in use stays offered
            ASSERT_TRUE( PickItem( context, video->GetWidget( "DisplayMode" ), 1 ) );
            ASSERT_TRUE( "Full screen" == ComboboxText( video, "DisplayMode" ) );
            ASSERT_TRUE( "1280 x 960" == ComboboxText( video, "Resolution" ) );
            ASSERT_TRUE( ComboboxHas( video, "Resolution", "1280 x 720" ) && !ComboboxHas( video, "Resolution", "1600 x 900" ) );
            ASSERT_TRUE( PickItem( context, video->GetWidget( "Resolution" ), 2 ) );
            ASSERT_TRUE( "1920 x 1080" == ComboboxText( video, "Resolution" ) );

            // Apply: only the page with changes is applied
            ClickWidget( context, form.GetWidget( "Apply" ) );
            ASSERT_TRUE( 1 == formApplied.count && 1 == videoApplied.count && 0 == generalApplied.count );
            const GUI::CVideoSettings& applied = video->GetVideoSettings();
            ASSERT_TRUE( applied.GetFullscreenState() && 1920 == applied.GetResolutionWidthInPixels() && 1080 == applied.GetResolutionHeightInPixels() );
            ASSERT_TRUE( 144 == applied.GetFrequency() && "fsr1" == applied.GetPresentFilter() );
            ASSERT_TRUE( 1 == applied.GetPresentFilterParameters().count( "sharpness" ) );
            ASSERT_FALSE( video->HasChanges() || IsEnabled( &form, "Apply" ) );
            ASSERT_TRUE( form.IsVisible() );

            // a change on the general page, then Cancel: the page shows the settings again, nothing applied
            GUI::CTabControl* tabs = form.GetWidgetAs< GUI::CTabControl >( "Pages" );
            ASSERT_TRUE( GUCEF_NULL != tabs && tabs->SetActiveTab( 1 ) );
            ClickWidget( context, general->GetWidget( "ConsoleLogWindow" ) );
            ASSERT_TRUE( general->GetWidget( "ConsoleLogWindow" )->GetBoolProperty( "Checked" ) && general->HasChanges() );
            ClickWidget( context, form.GetWidget( "Cancel" ) );
            ASSERT_FALSE( form.IsVisible() );
            ASSERT_TRUE( 1 == formClosed.count && 0 == generalApplied.count );
            ASSERT_FALSE( general->GetWidget( "ConsoleLogWindow" )->GetBoolProperty( "Checked" ) || general->HasChanges() );
            ASSERT_FALSE( CORE::CCoreGlobal::Instance()->GetLogManager().IsNativeConsoleLoggerEnabled() );

            // the console log in a window of its own: indented below the console log option, checked, greyed out while that is off
            GUI::CWidget* ownWindow = general->GetWidget( "ConsoleLogOwnWindow" );
            ASSERT_TRUE( GUCEF_NULL != ownWindow && ownWindow->GetBoolProperty( "Checked" ) && !ownWindow->IsEffectivelyEnabled() );
            ASSERT_TRUE( ownWindow->GetRect().left > general->GetWidget( "ConsoleLogWindow" )->GetRect().left );
            ASSERT_TRUE( GUCEF_NULL != GUI::CWidget::As< GUI::CGroupBox >( general->GetWidget( "LogGroup" ) ) );

            // again, now with OK: applied and closed
            ASSERT_TRUE( form.Open() );
            ClickWidget( context, general->GetWidget( "ConsoleLogWindow" ) );
            ClickWidget( context, form.GetWidget( "Ok" ) );
            ASSERT_FALSE( form.IsVisible() );
            ASSERT_TRUE( 2 == formClosed.count && 1 == generalApplied.count && 1 == videoApplied.count );
            ASSERT_TRUE( general->GetGeneralSettings().GetConsoleLogWindow() && general->GetGeneralSettings().GetConsoleLogOwnWindow() );
            ASSERT_TRUE( general->GetWidget( "ConsoleLogOwnWindow" )->IsEffectivelyEnabled() );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetLogManager().IsNativeConsoleLoggerEnabled() );
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetLogManager().IsNativeConsoleLoggerInOwnWindow() );
            CORE::CCoreGlobal::Instance()->GetLogManager().SetNativeConsoleLoggerEnabled( false );

            // Escape is Cancel
            ASSERT_TRUE( form.Open() );
            context->InjectKey( INPUT::KEYCODE_ESCAPE, true, 0 );
            context->InjectKey( INPUT::KEYCODE_ESCAPE, false, 0 );
            ASSERT_FALSE( form.IsVisible() );
            ASSERT_TRUE( 3 == formClosed.count );

            // the layout made in code is a layout like any other
            CORE::CDataNode layout;
            ASSERT_TRUE( form.SaveLayout( layout ) );
            GUI::CFormLayout::TStringVector problems;
            ASSERT_TRUE( GUI::CFormLayout::Validate( *GUI::CFormLayout::FindLayoutRoot( layout ), problems ) );

            CORE::SetEnv( "GUCEF_CONSOLE_LOGGER", consoleLoggerEnv );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The video page shows how far work after Apply is, with a button that asks to stop it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( form.AddPage( "Video" ) );
            ASSERT_TRUE( GUCEF_NULL != video && form.Open() );
            CEventCount stopped( video, GUI::CVideoSettingsForm::ProgressStoppedEvent );
            ASSERT_FALSE( video->IsProgressShown() || video->GetWidget( "StopProgress" )->IsVisible() );

            video->ShowProgress( "Upscaling the pictures", 0.25f );
            ASSERT_TRUE( video->IsProgressShown() && video->GetWidget( "StopProgress" )->IsVisible() );
            ASSERT_TRUE( 250.0f == video->GetWidget( "Progress" )->GetFloatProperty( "Value" ) );
            ClickWidget( context, video->GetWidget( "StopProgress" ) );
            ASSERT_TRUE( 1 == stopped.count );

            video->HideProgress();
            ASSERT_FALSE( video->IsProgressShown() || video->GetWidget( "StopProgress" )->IsVisible() );

            // the page listens to the root of its layout (value changes) and to the stop button: a link per widget, the four standard events and one more each
            ASSERT_TRUE( 10 == video->GetObserverSubscriptionCount() );

            // a layout that is loaded again has new widgets: the old subscriptions are gone and the new ones are there, no more than before
            CORE::CDataNode saved;
            ASSERT_TRUE( video->SaveLayout( saved ) && video->LoadLayout( saved ) );
            ASSERT_TRUE( 10 == video->GetObserverSubscriptionCount() );
            video->ShowProgress( "Upscaling the pictures", 0.5f );
            ClickWidget( context, video->GetWidget( "StopProgress" ) );
            ASSERT_TRUE( 2 == stopped.count );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The Movies row offers the ways of upscaling movies the application has" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            // copies are asked for but only the filter is there: the movies show as they are, the filter can be picked
            GUI::CSettingsForm form( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( form.AddPage( "Video" ) );
            GUI::CVideoSettings settings;
            settings.SetMovieUpscaling( GUI::CVideoSettings::MovieUpscalingConverted );
            video->SetVideoSettings( settings );
            video->SetAvailableMovieUpscaling( true, false );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( IsEnabled( video, "MovieUpscaling" ) );
            ASSERT_TRUE( GUI::CVideoSettingsForm::MoviesAsTheyAreChoice == ComboboxText( video, "MovieUpscaling" ) );
            ASSERT_TRUE( ComboboxHas( video, "MovieUpscaling", GUI::CVideoSettingsForm::MoviesFilterChoice ) && !ComboboxHas( video, "MovieUpscaling", GUI::CVideoSettingsForm::MoviesCopiesChoice ) );
            ASSERT_TRUE( PickItem( context, video->GetWidget( "MovieUpscaling" ), 1 ) );
            ASSERT_TRUE( GUI::CVideoSettings::MovieUpscalingFilter == video->GetSettingsFromWidgets().GetMovieUpscaling() );
            form.Cancel();

            // both: the copies the settings ask for are picked, "as they are" gives no upscaling
            GUI::CSettingsForm both( context );
            GUI::CVideoSettingsForm* bothVideo = static_cast< GUI::CVideoSettingsForm* >( both.AddPage( "Video" ) );
            bothVideo->SetVideoSettings( settings );
            bothVideo->SetAvailableMovieUpscaling( true, true );
            ASSERT_TRUE( both.Open() );
            ASSERT_TRUE( GUI::CVideoSettingsForm::MoviesCopiesChoice == ComboboxText( bothVideo, "MovieUpscaling" ) );
            ASSERT_TRUE( PickItem( context, bothVideo->GetWidget( "MovieUpscaling" ), 0 ) );
            ASSERT_TRUE( bothVideo->GetSettingsFromWidgets().GetMovieUpscaling().IsNULLOrEmpty() );
            both.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The Renderer row offers Automatic and the renderers of the application, and says a renderer other than the one in use takes effect after a restart" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            // the renderer is in the settings section, empty by default
            ASSERT_TRUE( GUI::CVideoSettings().GetRenderer().IsNULLOrEmpty() );
            CORE::CDataNode config( "Config" );
            GUI::CVideoSettings vulkan;
            vulkan.SetRenderer( "Vulkan" );
            ASSERT_TRUE( vulkan.SaveConfig( config ) );
            ASSERT_TRUE( "Vulkan" == config.FindChild( GUI::CVideoSettings::ConfigSectionName )->GetAttributeValue( "renderer" ).AsString() );

            // no renderers: the row is disabled and Apply keeps the renderer of the settings
            GUI::CSettingsForm none( context );
            GUI::CVideoSettingsForm* noneVideo = static_cast< GUI::CVideoSettingsForm* >( none.AddPage( "Video" ) );
            noneVideo->SetVideoSettings( vulkan );
            ASSERT_TRUE( none.Open() );
            ASSERT_FALSE( IsEnabled( noneVideo, "Renderer" ) );
            ASSERT_TRUE( GUI::CVideoSettingsForm::AutomaticChoice == ComboboxText( noneVideo, "Renderer" ) );
            ASSERT_FALSE( noneVideo->GetWidget( "RendererNote" )->IsVisible() );
            ASSERT_TRUE( "Vulkan" == noneVideo->GetSettingsFromWidgets().GetRenderer() );
            none.Cancel();

            // the renderers the backend has, in its order, OpenGL in use: Vulkan in the settings is picked and is not the one in use
            GUI::CSettingsForm form( context );
            GUI::CVideoSettingsForm* video = static_cast< GUI::CVideoSettingsForm* >( form.AddPage( "Video" ) );
            video->SetVideoSettings( vulkan );
            GUI::CVideoSettingsForm::TStringVector renderers;
            renderers.push_back( "OpenGL" );
            renderers.push_back( "Vulkan" );
            video->SetAvailableRenderers( renderers, "OpenGL" );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( IsEnabled( video, "Renderer" ) );
            ASSERT_TRUE( ComboboxHas( video, "Renderer", GUI::CVideoSettingsForm::AutomaticChoice ) && ComboboxHas( video, "Renderer", "OpenGL" ) );
            ASSERT_TRUE( "Vulkan" == ComboboxText( video, "Renderer" ) );
            ASSERT_TRUE( video->GetWidget( "RendererNote" )->IsVisible() );
            ASSERT_TRUE( GUI::CVideoSettingsForm::RendererNoteText == video->GetWidget( "RendererNote" )->GetProperty( "Text" ) );
            ASSERT_FALSE( video->HasChanges() );

            // Automatic is the first renderer of the list, the one in use: no note, and no renderer in the settings
            ASSERT_TRUE( PickItem( context, video->GetWidget( "Renderer" ), 0 ) );
            ASSERT_TRUE( GUI::CVideoSettingsForm::AutomaticChoice == ComboboxText( video, "Renderer" ) );
            ASSERT_FALSE( video->GetWidget( "RendererNote" )->IsVisible() );
            ASSERT_TRUE( video->HasChanges() );
            ASSERT_TRUE( video->GetSettingsFromWidgets().GetRenderer().IsNULLOrEmpty() );

            // OpenGL by name: the one in use, no note; Vulkan again: the note, and Apply keeps it in the settings
            ASSERT_TRUE( PickItem( context, video->GetWidget( "Renderer" ), 1 ) );
            ASSERT_FALSE( video->GetWidget( "RendererNote" )->IsVisible() );
            ASSERT_TRUE( "OpenGL" == video->GetSettingsFromWidgets().GetRenderer() );
            ASSERT_TRUE( PickItem( context, video->GetWidget( "Renderer" ), 2 ) );
            ASSERT_TRUE( video->GetWidget( "RendererNote" )->IsVisible() );
            ASSERT_TRUE( video->ApplySettings() );
            ASSERT_TRUE( "Vulkan" == video->GetVideoSettings().GetRenderer() );
            form.Cancel();

            // a renderer of the settings that is not available here shows as Automatic, without a note for the one the backend prefers
            GUI::CSettingsForm other( context );
            GUI::CVideoSettingsForm* otherVideo = static_cast< GUI::CVideoSettingsForm* >( other.AddPage( "Video" ) );
            GUI::CVideoSettings direct3d;
            direct3d.SetRenderer( "Direct3D 11" );
            otherVideo->SetVideoSettings( direct3d );
            otherVideo->SetAvailableRenderers( renderers, "OpenGL" );
            ASSERT_TRUE( other.Open() );
            ASSERT_TRUE( GUI::CVideoSettingsForm::AutomaticChoice == ComboboxText( otherVideo, "Renderer" ) );
            ASSERT_FALSE( otherVideo->GetWidget( "RendererNote" )->IsVisible() );
            other.Cancel();

            // the layout with the new row is valid
            CORE::CDataNode layout;
            ASSERT_TRUE( form.SaveLayout( layout ) );
            GUI::CFormLayout::TStringVector problems;
            ASSERT_TRUE( GUI::CFormLayout::Validate( *GUI::CFormLayout::FindLayoutRoot( layout ), problems ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: A page of the form is brought to the front by its title, also when the form opens" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CSettingsForm form( context );
            ASSERT_FALSE( form.SelectPage( "General" ) );
            ASSERT_TRUE( form.GetSelectedPageTitle().IsNULLOrEmpty() );
            ASSERT_TRUE( GUCEF_NULL != form.AddPage( "Video" ) && GUCEF_NULL != form.AddPage( "General" ) );
            ASSERT_FALSE( form.SelectPage( "General" ) );
            ASSERT_TRUE( form.Build() );

            ASSERT_TRUE( form.Open() && "Video" == form.GetSelectedPageTitle() );
            ASSERT_TRUE( form.SelectPage( "General" ) && "General" == form.GetSelectedPageTitle() );
            ASSERT_FALSE( form.SelectPage( "Nothing like it" ) );
            ASSERT_TRUE( "General" == form.GetSelectedPageTitle() );
            form.Cancel();

            // opened at a page: that page is in front, an unknown title leaves the one that was
            ASSERT_TRUE( form.Open( "Video" ) && "Video" == form.GetSelectedPageTitle() );
            form.Cancel();
            ASSERT_TRUE( form.Open( "General" ) && "General" == form.GetSelectedPageTitle() );
            form.Cancel();
            ASSERT_TRUE( form.Open( "Nothing like it" ) && "General" == form.GetSelectedPageTitle() );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: Scrolling a page, with the wheel or the scroll bar, is not a change of the settings" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            CScrollingTestPage page;
            GUI::CSettingsForm form( context );
            form.AddPage( &page );
            ASSERT_TRUE( form.Build() && form.Open() );
            context->FlushChanges();
            GUI::CScrollPanel* body = static_cast< GUI::CScrollPanel* >( page.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != body && body->IsScrollable() );

            // over the field, over the tall button, over the scroll bar
            const GUI::CRect field = page.GetWidget( "Name" )->GetRect();
            context->InjectMouseMove( field.left + 10, field.top + 5 );
            context->InjectMouseWheel( -1 );
            context->FlushChanges();
            ASSERT_TRUE( 48 == body->GetScrollPosition() );
            const GUI::CRect track = body->GetTrackRect();
            context->InjectMouseMove( body->GetRect().left + track.left + 5, body->GetRect().top + track.Bottom() - 5 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( body->GetScrollPosition() > 48 );

            ASSERT_FALSE( page.HasChanges() || form.HasChanges() );
            ASSERT_FALSE( IsEnabled( &form, "Apply" ) );

            // a value of the user is one
            ASSERT_TRUE( page.GetWidget( "Name" )->ChangeValueByUser( "Text", "b" ) );
            ASSERT_TRUE( page.HasChanges() && IsEnabled( &form, "Apply" ) );
            form.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Shortcuts: a chord is parsed from its text, and the bindings are saved and read back, also when two actions swap their keys" )
    try
    {
        INPUT::KeyCode key = INPUT::KEYCODE_UNASSIGNED;
        UInt32 modifiers = 0;
        ASSERT_TRUE( GUI::CKeyboardShortcutMap::ParseChord( "Ctrl+Shift+F10", key, modifiers ) && INPUT::KEYCODE_F10 == key && ( INPUT::KEYMOD_CTRL | INPUT::KEYMOD_SHIFT ) == modifiers );
        ASSERT_TRUE( GUI::CKeyboardShortcutMap::ParseChord( "Ctrl+,", key, modifiers ) && INPUT::KEYCODE_COMMA == key && INPUT::KEYMOD_CTRL == modifiers );
        ASSERT_TRUE( GUI::CKeyboardShortcutMap::ParseChord( "Grave", key, modifiers ) && INPUT::KEYCODE_GRAVE == key && 0 == modifiers );
        ASSERT_TRUE( GUI::CKeyboardShortcutMap::ParseChord( "Alt+Left", key, modifiers ) && INPUT::KEYCODE_LEFT == key && INPUT::KEYMOD_ALT == modifiers );
        ASSERT_FALSE( GUI::CKeyboardShortcutMap::ParseChord( "Hyper+X", key, modifiers ) || GUI::CKeyboardShortcutMap::ParseChord( "", key, modifiers ) || GUI::CKeyboardShortcutMap::ParseChord( "Ctrl+Nonsense", key, modifiers ) );
        ASSERT_TRUE( "Grave" == GUI::CKeyboardShortcutMap::DescribeKey( INPUT::KEYCODE_GRAVE ) );

        GUI::CKeyboardShortcutMap map;
        ASSERT_TRUE( map.Bind( "a", INPUT::KEYCODE_F9, 0 ) && map.Bind( "b", INPUT::KEYCODE_F10, 0 ) && map.Bind( "c", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
        // a and b swap their keys: written, then read into a map that has the old keys
        GUI::CKeyboardShortcutMap swapped;
        ASSERT_TRUE( swapped.Bind( "a", INPUT::KEYCODE_F10, 0 ) && swapped.Bind( "b", INPUT::KEYCODE_F9, 0 ) && swapped.Bind( "c", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
        CORE::CDataNode node( "Shortcuts" );
        ASSERT_TRUE( swapped.SaveBindings( node ) );
        ASSERT_TRUE( 3 == map.LoadBindings( node ) );
        ASSERT_TRUE( "a" == map.FindAction( INPUT::KEYCODE_F10, 0 ) && "b" == map.FindAction( INPUT::KEYCODE_F9, 0 ) && "c" == map.FindAction( INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
        // an entry that is no chord is left out and leaves the action as it was
        node.SetAttribute( "c", "nonsense" );
        ASSERT_TRUE( 2 == map.LoadBindings( node ) && "c" == map.FindAction( INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
        CORE::CString::StringVector ids;
        map.GetActionIds( ids );
        ASSERT_TRUE( 3 == ids.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The input page lists the actions with their keys, refuses a key that another action has, Escape cancels a change, Reset puts the shipped key back and Apply binds them" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CKeyboardShortcutMap map;
            ASSERT_TRUE( map.Bind( "fold", INPUT::KEYCODE_F9, 0 ) && map.Bind( "preview", INPUT::KEYCODE_F11, 0 ) && map.Bind( "open", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
            GUI::CSettingsForm form( context );
            GUI::CInputSettingsForm page;
            page.SetShortcutMap( &map );
            ASSERT_TRUE( page.AddAction( "fold", "Fold the side", INPUT::KEYCODE_F9, 0 ) && page.AddAction( "preview", "Preview", INPUT::KEYCODE_F11, 0 ) && page.AddAction( "open", "Open", INPUT::KEYCODE_O, INPUT::KEYMOD_CTRL ) );
            ASSERT_FALSE( page.AddAction( "fold", "Again", INPUT::KEYCODE_F1, 0 ) || page.AddAction( "", "None", INPUT::KEYCODE_F1, 0 ) );
            ASSERT_TRUE( form.AddPage( &page ) && form.Build() && form.Open() );
            CEventCount applied( &page, GUI::CSettingsPageForm::SettingsAppliedEvent );
            ASSERT_TRUE( "Input" == page.GetTitle() && 3 == page.GetActionCount() );
            ASSERT_TRUE( "Fold the side    F9" == page.GetActionLine( 0 ) && "Open    Ctrl+O" == page.GetActionLine( 2 ) );
            GUI::CListbox* list = page.GetWidgetAs< GUI::CListbox >( "Actions" );
            ASSERT_TRUE( GUCEF_NULL != list && 3 == list->GetItems().size() && "Fold the side    F9" == list->GetItems()[ 0 ] );
            ASSERT_FALSE( page.HasChanges() || page.IsRebinding() );

            // change: select the action, press Change, press the key
            ASSERT_TRUE( page.SelectAction( "fold" ) );
            ClickWidget( context, page.GetWidget( "Change" ) );
            ASSERT_TRUE( page.IsRebinding() );
            context->InjectKey( INPUT::KEYCODE_F8, true, 0 );
            context->InjectKey( INPUT::KEYCODE_F8, false, 0 );
            ASSERT_FALSE( page.IsRebinding() );
            ASSERT_TRUE( "Fold the side    F8" == page.GetActionLine( 0 ) && page.HasChanges() && IsEnabled( &form, "Apply" ) );
            ASSERT_TRUE( "F9" == map.GetShortcutText( "fold" ) );

            // a key another action has: refused with the name of that action, nothing changes
            ClickWidget( context, page.GetWidget( "Change" ) );
            context->InjectKey( INPUT::KEYCODE_F11, true, 0 );
            ASSERT_FALSE( page.IsRebinding() );
            ASSERT_TRUE( "Fold the side    F8" == page.GetActionLine( 0 ) && page.GetStatusText().HasSubstr( "F11 is the key of Preview already" ) >= 0 );
            // with a modifier it is another chord
            ClickWidget( context, page.GetWidget( "Change" ) );
            context->InjectKey( INPUT::KEYCODE_LSHIFT, true, INPUT::KEYMOD_SHIFT );
            ASSERT_TRUE( page.IsRebinding() );
            context->InjectKey( INPUT::KEYCODE_F11, true, INPUT::KEYMOD_SHIFT );
            ASSERT_TRUE( "Fold the side    Shift+F11" == page.GetActionLine( 0 ) );

            // Escape cancels a change
            ClickWidget( context, page.GetWidget( "Change" ) );
            context->InjectKey( INPUT::KEYCODE_ESCAPE, true, 0 );
            ASSERT_FALSE( page.IsRebinding() );
            ASSERT_TRUE( "Fold the side    Shift+F11" == page.GetActionLine( 0 ) && form.IsVisible() );

            // reset puts the shipped key back; a key that is taken by another action keeps the reset from happening
            ClickWidget( context, page.GetWidget( "Reset" ) );
            ASSERT_TRUE( "Fold the side    F9" == page.GetActionLine( 0 ) );
            ASSERT_TRUE( page.Rebind( "fold", INPUT::KEYCODE_F5, 0 ) && page.Rebind( "preview", INPUT::KEYCODE_F9, 0 ) );
            ASSERT_FALSE( page.ResetAction( "fold" ) );
            ASSERT_TRUE( "Fold the side    F5" == page.GetActionLine( 0 ) );
            ClickWidget( context, page.GetWidget( "ResetAll" ) );
            ASSERT_TRUE( "Fold the side    F9" == page.GetActionLine( 0 ) && "Preview    F11" == page.GetActionLine( 1 ) );

            // apply binds the keys in the map, once
            ASSERT_TRUE( page.Rebind( "fold", INPUT::KEYCODE_F5, 0 ) && page.Rebind( "preview", INPUT::KEYCODE_F9, 0 ) );
            ClickWidget( context, form.GetWidget( "Apply" ) );
            ASSERT_TRUE( 1 == applied.count && !page.HasChanges() );
            ASSERT_TRUE( "F5" == map.GetShortcutText( "fold" ) && "F9" == map.GetShortcutText( "preview" ) && "fold" == map.FindAction( INPUT::KEYCODE_F5, 0 ) );
            // cancel puts the keys of the map back in the page
            ASSERT_TRUE( page.Rebind( "open", INPUT::KEYCODE_P, INPUT::KEYMOD_CTRL ) );
            form.Cancel();
            ASSERT_TRUE( "Open    Ctrl+O" == page.GetActionLine( 2 ) && "F5" == map.GetShortcutText( "fold" ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Tooltips: a text provider gives the tooltip of a widget that has none of its own, and the tooltip of the widget goes first" )
    try
    {
        class CTableProvider : public GUI::CITooltipTextProvider
        {
            public:
            virtual CORE::CString GetTooltipFor( const GUI::CWidget& widget, Int32 localX, Int32 localY ) const GUCEF_VIRTUAL_OVERRIDE
            {
                return "Close" == widget.GetName() ? CORE::CString( "from the table" ) : CORE::CString::Empty;
            }
        };

        GUI::TGuiContextPtr contextPtr = CreateSettingsContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
            GUI::CWidget* close = context->CreateWidget( "Button", "Close", screen );
            GUI::CWidget* own = context->CreateWidget( "Button", "Own", screen );
            GUI::CWidget* none = context->CreateWidget( "Button", "None", screen );
            own->SetProperty( GUI::EWidgetProperty_Tooltip, "its own" );
            CTableProvider provider;
            GUI::CTooltipService& tooltips = context->GetTooltipService();
            ASSERT_TRUE( GUCEF_NULL == tooltips.GetTextProvider() && tooltips.GetTooltipText( *close, 1, 1 ).IsNULLOrEmpty() );
            tooltips.SetTextProvider( &provider );
            ASSERT_TRUE( &provider == tooltips.GetTextProvider() );
            ASSERT_TRUE( "from the table" == tooltips.GetTooltipText( *close, 1, 1 ) && "its own" == tooltips.GetTooltipText( *own, 1, 1 ) && tooltips.GetTooltipText( *none, 1, 1 ).IsNULLOrEmpty() );
            close->SetProperty( GUI::EWidgetProperty_Tooltip, "the widget says" );
            ASSERT_TRUE( "the widget says" == tooltips.GetTooltipText( *close, 1, 1 ) );
            tooltips.SetTextProvider( GUCEF_NULL );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The scope of reading settings ends at the end of the block, on a return and when an exception unwinds it, and nests" )
    try
    {
        CReadingScopeProbePage page;
        ASSERT_FALSE( page.Reading() );
        ASSERT_TRUE( page.NestingWorks() );
        ASSERT_TRUE( page.ReturnsEarly() );
        ASSERT_FALSE( page.Reading() );
        bool caught = false;
        try
        {
            page.Throws();
        }
        catch ( int )
        {
            caught = true;
        }
        ASSERT_TRUE( caught );
        ASSERT_FALSE( page.Reading() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The window system is x11 until the settings say otherwise, is read in any case, and a wrong name leaves it as it was" )
    try
    {
        GUI::CGeneralSettings settings;
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_X11 == settings.GetWindowSystem() );
        settings.SetWindowSystem( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND );
        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( settings.SaveConfig( config ) );
        ASSERT_TRUE( "wayland" == config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->GetAttributeValue( "windowSystem" ).AsString() );
        GUI::CGeneralSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == loaded.GetWindowSystem() );

        GUI::CGeneralSettings copy( loaded );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == copy.GetWindowSystem() );
        GUI::CGeneralSettings assigned;
        assigned = loaded;
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == assigned.GetWindowSystem() );

        config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->SetAttribute( "windowSystem", "AUTO" );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_AUTO == loaded.GetWindowSystem() );
        config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->SetAttribute( "windowSystem", "x11" );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_X11 == loaded.GetWindowSystem() );
        config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->SetAttribute( "windowSystem", "wayland" );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->SetAttribute( "windowSystem", "mir" );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == loaded.GetWindowSystem() );

        // a config from before the setting existed does not touch it
        CORE::CDataNode old( "Config" );
        old.Structure( GUI::CGeneralSettings::ConfigSectionName, '/' )->SetAttribute( "developerConsole", true );
        GUI::CGeneralSettings fromOld;
        ASSERT_TRUE( fromOld.LoadConfig( old ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_X11 == fromOld.GetWindowSystem() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: Window system names are parsed in any case and auto picks Wayland only where it is available" )
    try
    {
        GUI::CGeneralSettings::EWindowSystem system = GUI::CGeneralSettings::WINDOWSYSTEM_X11;
        ASSERT_TRUE( GUI::CGeneralSettings::ParseWindowSystem( "Wayland", system ) && GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == system );
        ASSERT_TRUE( GUI::CGeneralSettings::ParseWindowSystem( " auto ", system ) && GUI::CGeneralSettings::WINDOWSYSTEM_AUTO == system );
        ASSERT_TRUE( GUI::CGeneralSettings::ParseWindowSystem( "X11", system ) && GUI::CGeneralSettings::WINDOWSYSTEM_X11 == system );
        ASSERT_TRUE( GUI::CGeneralSettings::ParseWindowSystem( "xwayland", system ) && GUI::CGeneralSettings::WINDOWSYSTEM_X11 == system );
        system = GUI::CGeneralSettings::WINDOWSYSTEM_AUTO;
        ASSERT_FALSE( GUI::CGeneralSettings::ParseWindowSystem( "", system ) );
        ASSERT_FALSE( GUI::CGeneralSettings::ParseWindowSystem( "mir", system ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_AUTO == system );
        ASSERT_TRUE( 0 == strcmp( "wayland", GUI::CGeneralSettings::WindowSystemToString( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND ) ) );
        ASSERT_TRUE( 0 == strcmp( "auto", GUI::CGeneralSettings::WindowSystemToString( GUI::CGeneralSettings::WINDOWSYSTEM_AUTO ) ) );
        ASSERT_TRUE( 0 == strcmp( "x11", GUI::CGeneralSettings::WindowSystemToString( GUI::CGeneralSettings::WINDOWSYSTEM_X11 ) ) );

        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == GUI::CGeneralSettings::ResolveWindowSystem( GUI::CGeneralSettings::WINDOWSYSTEM_AUTO, true ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_X11 == GUI::CGeneralSettings::ResolveWindowSystem( GUI::CGeneralSettings::WINDOWSYSTEM_AUTO, false ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_X11 == GUI::CGeneralSettings::ResolveWindowSystem( GUI::CGeneralSettings::WINDOWSYSTEM_X11, true ) );
        ASSERT_TRUE( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND == GUI::CGeneralSettings::ResolveWindowSystem( GUI::CGeneralSettings::WINDOWSYSTEM_WAYLAND, false ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: A window backend is available unless it says it is not, and a source of display modes can be registered and selected" )
    try
    {
        class CPlainBackend : public GUI::CWindowManagerBackend
        {
            public:
            virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString&, const GUI::UInt32, const GUI::UInt32, const bool ) { return GUI::TWindowContextPtr(); }
            virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString&, const GUI::CVideoSettings&, const CORE::CValueList* = GUCEF_NULL ) { return GUI::TWindowContextPtr(); }
            virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString&, const GUI::UInt32, const GUI::UInt32, const bool, const CORE::CValueList& ) { return GUI::TWindowContextPtr(); }
            virtual void DestroyWindowContext( GUI::TWindowContextPtr& ) {}
            virtual bool ApplyVideoSettings( GUI::TWindowContextPtr&, const GUI::CVideoSettings& ) { return false; }
        };
        CPlainBackend backend;
        ASSERT_TRUE( backend.IsAvailable() );
        ASSERT_TRUE( backend.GetAvailabilityDescription().IsNULLOrEmpty() );

        // nothing selected: the platform is asked, as before
        ASSERT_TRUE( GUI::CVideoOptions::GetSelectedDisplayModeSource().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The general settings apply the user log at once: off, the messages of what the user does are made for no logger, on they reach all of them" )
    try
    {
        CORE::CLogManager& logManager = CORE::CCoreGlobal::Instance()->GetLogManager();
        CORE::CLogStreamScope::FlushLogs();
        CUserMessageCounter console;
        logManager.AddLogger( &console );
        const CORE::CString logFile = CORE::RelativePath( "$TEMPDIR$/gucefGUI_TestApp_UserLog/UserActions.log" );
        CORE::DeleteFile( logFile );

        GUI::CGeneralSettings settings;
        settings.SetUserActionLogFile( logFile );
        ASSERT_FALSE( settings.GetUserActionLog() );
        GUI::CGeneralSettingsForm::ApplyGeneralSettings( settings, GUCEF_NULL );
        ASSERT_FALSE( logManager.IsUserLogEnabled() || logManager.IsUserActionLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "What the user did while it was off" );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "What the program said while it was off" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 0 == console.userMessages && console.otherMessages > 0 );

        settings.SetUserActionLog( true );
        GUI::CGeneralSettingsForm::ApplyGeneralSettings( settings, GUCEF_NULL );
        ASSERT_TRUE( logManager.IsUserLogEnabled() && logManager.IsUserActionLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "What the user did while it was on" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 1 == console.userMessages );
        CORE::CString content;
        ASSERT_TRUE( CORE::LoadTextFileAsString( logFile, content ) && content.HasSubstr( "while it was on" ) >= 0 && content.HasSubstr( "while it was off" ) < 0 );

        settings.SetUserActionLog( false );
        GUI::CGeneralSettingsForm::ApplyGeneralSettings( settings, GUCEF_NULL );
        ASSERT_FALSE( logManager.IsUserLogEnabled() || logManager.IsUserActionLogEnabled() );
        GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "What the user did when it was off again" );
        CORE::CLogStreamScope::FlushLogs();
        ASSERT_TRUE( 1 == console.userMessages );

        // the programs that do not apply settings keep what they had: all of it
        logManager.SetUserLogEnabled( true );
        logManager.RemoveLogger( &console );
        CORE::DeleteFile( logFile );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: General settings are saved as a config section and read back" )
    try
    {
        GUI::CGeneralSettings settings;
        ASSERT_FALSE( settings.GetConsoleLogWindow() || settings.GetDeveloperConsole() || settings.GetUserActionLog() );
        ASSERT_TRUE( INPUT::KEYCODE_GRAVE == settings.GetDeveloperConsoleKey() );
        settings.SetConsoleLogWindow( true );
        settings.SetDeveloperConsole( true );
        settings.SetDeveloperConsoleKey( INPUT::KEYCODE_F12 );
        settings.SetUserActionLog( true );
        settings.SetUserActionLogFile( "$TEMPDIR$/MyApp/UserActions.log" );

        CORE::CDataNode config( "Config" );
        ASSERT_TRUE( settings.SaveConfig( config ) );
        ASSERT_TRUE( "F12" == config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->GetAttributeValue( "developerConsoleKey" ).AsString() );
        GUI::CGeneralSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( loaded.GetConsoleLogWindow() && loaded.GetDeveloperConsole() && INPUT::KEYCODE_F12 == loaded.GetDeveloperConsoleKey() );
        ASSERT_TRUE( loaded.GetUserActionLog() && "$TEMPDIR$/MyApp/UserActions.log" == loaded.GetUserActionLogFile() );

        // the console log in a window of its own unless said otherwise
        ASSERT_TRUE( settings.GetConsoleLogOwnWindow() && loaded.GetConsoleLogOwnWindow() );
        settings.SetConsoleLogOwnWindow( false );
        ASSERT_TRUE( settings.SaveConfig( config ) && loaded.LoadConfig( config ) && !loaded.GetConsoleLogOwnWindow() );

        // a key name that is no key leaves the key as it was
        config.FindChild( GUI::CGeneralSettings::ConfigSectionName )->SetAttribute( "developerConsoleKey", "NoSuchKey" );
        ASSERT_TRUE( loaded.LoadConfig( config ) );
        ASSERT_TRUE( INPUT::KEYCODE_F12 == loaded.GetDeveloperConsoleKey() );
        ASSERT_TRUE( "~" == GUI::CGeneralSettingsForm::GetKeyDisplayName( INPUT::KEYCODE_GRAVE ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: The display of this system tells its modes and the mode it is in" )
    GUI::CVideoOptions::TDisplayMode current;
    if ( !GUI::CVideoOptions::RetrieveCurrentDisplayModeFromOS( current ) )
        GUCEF_TESTFW_SKIP( "there is no display to ask" );
    try
    {
        ASSERT_TRUE( current.widthInPixels > 0 && current.heightInPixels > 0 && current.depthInBits > 0 && current.frequency > 0 );
        GUI::CVideoSettings settings;
        ASSERT_TRUE( GUI::CVideoSettings::RetrieveSettingsFromOS( settings ) );
        ASSERT_TRUE( current.widthInPixels == settings.GetResolutionWidthInPixels() && current.heightInPixels == settings.GetResolutionHeightInPixels() );

        GUI::CVideoOptions options;
        ASSERT_TRUE( GUI::CVideoOptions::RetrieveOptionsFromOS( options ) );
        ASSERT_TRUE( !options.GetDisplayModes().empty() );
        GUI::CVideoOptions::TDisplayModeVector sameSize;
        options.GetDisplayModesForResolution( current.widthInPixels, current.heightInPixels, sameSize );
        ASSERT_TRUE( !sameSize.empty() );
        for ( UInt32 i=0; i<(UInt32)options.GetDisplayModes().size(); ++i )
            ASSERT_TRUE( options.GetDisplayModes()[ i ].widthInPixels > 0 && options.GetDisplayModes()[ i ].heightInPixels > 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Settings: A page of the application keeps its widgets when the form goes before it, also with RmlUi, whose elements of the page are moved out of the window first" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateRmlUiContext();
        GUI::CGUIContext* context = ( contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext() );
        if ( GUCEF_NULL == context )
            GUCEF_TESTFW_SKIP( "there is no RmlUi context (no offscreen window)" );
        CScrollingTestPage* page = new CScrollingTestPage();
        GUI::CSettingsForm* form = new GUI::CSettingsForm( context );
        ASSERT_TRUE( form->AddPage( page ) && form->Build() && form->Open() );
        context->RenderFrame( 16 );
        GUI::CWidget* name = page->GetWidget( "Name" );
        ASSERT_TRUE( GUCEF_NULL != name && GUCEF_NULL != name->GetImplementationPtr() );

        // the form goes first, as the application keeps its page: the page is a root of its own now, its widgets are there and drawn
        delete form;
        ASSERT_TRUE( GUCEF_NULL != page->GetRootWidget() && GUCEF_NULL == page->GetRootWidget()->GetParentWidget() );
        ASSERT_TRUE( name == page->GetWidget( "Name" ) && GUCEF_NULL != name->GetImplementationPtr() );
        ASSERT_TRUE( name->SetProperty( "Text", "still here" ) );
        context->RenderFrame( 16 );
        delete page;
        context->RenderFrame( 16 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormLintBaseline( "InputSettings", &BuildInputSettingsScene );
}

/*-------------------------------------------------------------------------*/
