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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGER_H
#include "gucefGUI_CWindowManager.h"
#define GUCEF_GUI_CWINDOWMANAGER_H
#endif /* GUCEF_GUI_CWINDOWMANAGER_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#include "gucefGUI_CVideoSettingsForm.h"

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

const CString CVideoSettingsForm::NoneChoice = "None";
const CString CVideoSettingsForm::MoviesAsTheyAreChoice = "As they are";
const CString CVideoSettingsForm::MoviesFilterChoice = "Upscaling filter while playing";
const CString CVideoSettingsForm::MoviesCopiesChoice = "Upscaled copies, made once";
const CORE::CEvent CVideoSettingsForm::ProgressStoppedEvent = "GUCEF::GUI::CVideoSettingsForm::ProgressStoppedEvent";

namespace {

const char* const SharpnessParameter = "sharpness";
const Float32 DefaultSharpness = 0.5f;

/** The rows of the page: a label on the left, the control on the right */
const Int32 RowTop = 24;
const Int32 RowStep = 48;
const Int32 RowHeight = 36;

CString
LabelPosition( Int32 row )
{
    return "0,24 0," + CORE::Int32ToString( RowTop + row * RowStep + 6 );
}

CString
ControlPosition( Int32 row )
{
    return "0,260 0," + CORE::Int32ToString( RowTop + row * RowStep );
}

const char* const LabelSize = "0,224 0,28";
const char* const ControlSize = "0,380 0,36";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CVideoSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    ProgressStoppedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CVideoSettingsForm::CVideoSettingsForm( void )
    : CSettingsPageForm()
    , m_settings()
    , m_options()
    , m_resolutions()
    , m_presentFilters()
    , m_imageUpscalers()
    , m_movieFilterAvailable( false )
    , m_movieCopiesAvailable( false )
    , m_backendName()
    , m_window()
{GUCEF_TRACE;

    RegisterEvents();
    CVideoOptions::RetrieveOptionsFromOS( m_options );
}

/*-------------------------------------------------------------------------*/

CVideoSettingsForm::~CVideoSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetVideoSettings( const CVideoSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CVideoSettings&
CVideoSettingsForm::GetVideoSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetVideoOptions( const CVideoOptions& options )
{GUCEF_TRACE;

    m_options = options;
}

/*-------------------------------------------------------------------------*/

const CVideoOptions&
CVideoSettingsForm::GetVideoOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetAvailablePresentFilters( const TStringVector& filterNames )
{GUCEF_TRACE;

    m_presentFilters = filterNames;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetAvailableImageUpscalers( const TStringVector& upscalerNames )
{GUCEF_TRACE;

    m_imageUpscalers = upscalerNames;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetAvailableMovieUpscaling( bool filter, bool copies )
{GUCEF_TRACE;

    m_movieFilterAvailable = filter;
    m_movieCopiesAvailable = copies;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::SetWindow( const CString& windowManagerBackendName, TWindowContextPtr window )
{GUCEF_TRACE;

    m_backendName = windowManagerBackendName;
    m_window = window;
}

/*-------------------------------------------------------------------------*/

CString
CVideoSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return "Video";
}

/*-------------------------------------------------------------------------*/

CString
CVideoSettingsForm::ResolutionToString( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    return CORE::UInt32ToString( width ) + " x " + CORE::UInt32ToString( height );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = CFormLayout::AddWidgetNode( layout, "Panel", "VideoSettings", "0,0 0,0", "1,0 1,0" );

    struct SRow { const char* label; const char* type; const char* name; };
    static const SRow rows[] = { { "Display", "Combobox", "DisplayMode" },
                                 { "Resolution", "Combobox", "Resolution" },
                                 { "", "Checkbox", "VSync" },
                                 { "Scaling", "Combobox", "PresentMode" },
                                 { "Screen filter", "Combobox", "PresentFilter" },
                                 { "Sharpness", "Slider", "Sharpness" },
                                 { "Art upscaler", "Combobox", "ImageUpscaler" },
                                 { "Movies", "Combobox", "MovieUpscaling" } };
    for ( Int32 r=0; r<(Int32)( sizeof( rows ) / sizeof( rows[ 0 ] ) ); ++r )
    {
        if ( '\0' != rows[ r ].label[ 0 ] )
        {
            CORE::CDataNode* label = CFormLayout::AddWidgetNode( *page, "Label", CString( rows[ r ].name ) + "Label", LabelPosition( r ), LabelSize );
            CFormLayout::SetWidgetNodeProperty( *label, EWidgetProperty_Text, rows[ r ].label );
        }
        CORE::CDataNode* control = CFormLayout::AddWidgetNode( *page, rows[ r ].type, rows[ r ].name, ControlPosition( r ), ControlSize );
        if ( CString( "Combobox" ) == rows[ r ].type )
            CFormLayout::SetWidgetNodeProperty( *control, EWidgetProperty_DropDownRows, "10" );
    }

    CORE::CDataNode::iterator i = page->Begin();
    while ( i != page->End() )
    {
        CORE::CDataNode* node = (*i);
        ++i;
        CString name = node->GetAttributeValue( "name" ).AsString();
        if ( name == "DisplayMode" )
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Items, "Window\nFull screen" );
        else
        if ( name == "VSync" )
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Text, "Wait for the vertical blank (vsync)" );
        else
        if ( name == "PresentMode" )
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Items, "Sharp pixels\nSmooth\nWhole multiples\nStretched" );
        else
        if ( name == "Sharpness" )
        {
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Min, "0" );
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Max, "100" );
            CFormLayout::SetWidgetNodeProperty( *node, EWidgetProperty_Step, "5" );
        }
    }

    CORE::CDataNode* progressText = CFormLayout::AddWidgetNode( *page, "Label", "ProgressText", "0,24 1,-84", "1,-48 0,28" );
    CFormLayout::SetWidgetNodeProperty( *progressText, EWidgetProperty_Visible, "false" );
    CORE::CDataNode* progress = CFormLayout::AddWidgetNode( *page, "ProgressBar", "Progress", "0,24 1,-52", "1,-184 0,24" );
    CFormLayout::SetWidgetNodeProperty( *progress, EWidgetProperty_Max, "1000" );
    CFormLayout::SetWidgetNodeProperty( *progress, EWidgetProperty_Visible, "false" );
    CORE::CDataNode* stop = CFormLayout::AddWidgetNode( *page, "Button", "StopProgress", "1,-144 1,-56", "0,120 0,32" );
    CFormLayout::SetWidgetNodeProperty( *stop, EWidgetProperty_Text, "Stop" );
    CFormLayout::SetWidgetNodeProperty( *stop, EWidgetProperty_Visible, "false" );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    CSettingsPageForm::OnPostLayoutLoad();
    CWidget* stop = GetWidget( "StopProgress" );
    if ( GUCEF_NULL != stop )
    {
        TEventCallback activatedCallback( this, &CVideoSettingsForm::OnStopProgressActivated );
        SubscribeTo( stop, CWidget::ActivatedEvent, activatedCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::OnStopProgressActivated( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( ProgressStoppedEvent );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::FillResolutions( bool fullscreen, UInt32 width, UInt32 height )
{GUCEF_TRACE;

    m_resolutions.clear();
    m_options.GetCommonResolutions( fullscreen, m_resolutions );

    // the resolution in use is always offered, also when it is not a usual one
    Int32 selected = -1;
    for ( UInt32 i=0; i<(UInt32)m_resolutions.size(); ++i )
        if ( m_resolutions[ i ].widthInPixels == width && m_resolutions[ i ].heightInPixels == height )
            selected = (Int32) i;
    if ( selected < 0 && width > 0 && height > 0 )
    {
        CVideoOptions::TDisplayMode mode;
        m_settings.GetDisplayMode( mode );
        mode.widthInPixels = width;
        mode.heightInPixels = height;
        UInt32 at = 0;
        while ( at < (UInt32)m_resolutions.size() &&
                (UInt64) m_resolutions[ at ].widthInPixels * m_resolutions[ at ].heightInPixels < (UInt64) width * height )
            ++at;
        m_resolutions.insert( m_resolutions.begin() + at, mode );
        selected = (Int32) at;
    }

    CCombobox* list = GetWidgetAs< CCombobox >( "Resolution" );
    if ( GUCEF_NULL == list )
        return;
    CCombobox::TStringVector items;
    for ( UInt32 i=0; i<(UInt32)m_resolutions.size(); ++i )
        items.push_back( ResolutionToString( m_resolutions[ i ].widthInPixels, m_resolutions[ i ].heightInPixels ) );
    list->SetListItems( items );
    list->SetSelectedIndex( selected );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::FillChoices( const CString& widgetName, const TStringVector& choices, const CString& selected )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( widgetName );
    if ( GUCEF_NULL == list )
        return;
    CCombobox::TStringVector items;
    items.push_back( NoneChoice );
    Int32 selectedIndex = 0;
    for ( UInt32 i=0; i<(UInt32)choices.size(); ++i )
    {
        items.push_back( choices[ i ] );
        if ( choices[ i ] == selected )
            selectedIndex = (Int32) i + 1;
    }
    list->SetListItems( items );
    list->SetSelectedIndex( selectedIndex );
}

/*-------------------------------------------------------------------------*/

CString
CVideoSettingsForm::GetChoice( const CString& widgetName )
{GUCEF_TRACE;

    CCombobox* list = GetWidgetAs< CCombobox >( widgetName );
    if ( GUCEF_NULL == list )
        return CString::Empty;
    CCombobox::TStringVector items;
    list->GetListItems( items );
    Int32 index = list->GetSelectedIndex();
    if ( index < 0 || index >= (Int32) items.size() )
        return CString::Empty;
    return items[ index ];
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::UpdateEnabledRows( void )
{GUCEF_TRACE;

    bool hasFilters = !m_presentFilters.empty();
    bool filterChosen = hasFilters && GetChoice( "PresentFilter" ) != NoneChoice;
    const char* filterRow[] = { "PresentFilter", "PresentFilterLabel" };
    for ( UInt32 i=0; i<2; ++i )
        if ( GUCEF_NULL != GetWidget( filterRow[ i ] ) )
            GetWidget( filterRow[ i ] )->SetEnabled( hasFilters );
    const char* sharpnessRow[] = { "Sharpness", "SharpnessLabel" };
    for ( UInt32 i=0; i<2; ++i )
        if ( GUCEF_NULL != GetWidget( sharpnessRow[ i ] ) )
            GetWidget( sharpnessRow[ i ] )->SetEnabled( filterChosen );
    const char* upscalerRow[] = { "ImageUpscaler", "ImageUpscalerLabel" };
    for ( UInt32 i=0; i<2; ++i )
        if ( GUCEF_NULL != GetWidget( upscalerRow[ i ] ) )
            GetWidget( upscalerRow[ i ] )->SetEnabled( !m_imageUpscalers.empty() );
    const char* movieRow[] = { "MovieUpscaling", "MovieUpscalingLabel" };
    for ( UInt32 i=0; i<2; ++i )
        if ( GUCEF_NULL != GetWidget( movieRow[ i ] ) )
            GetWidget( movieRow[ i ] )->SetEnabled( m_movieFilterAvailable || m_movieCopiesAvailable );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;

    SetReadingSettings( true );

    CCombobox* displayMode = GetWidgetAs< CCombobox >( "DisplayMode" );
    if ( GUCEF_NULL != displayMode )
        displayMode->SetSelectedIndex( m_settings.GetFullscreenState() ? 1 : 0 );
    FillResolutions( m_settings.GetFullscreenState(), m_settings.GetResolutionWidthInPixels(), m_settings.GetResolutionHeightInPixels() );
    SetWidgetProperty( "VSync", EWidgetProperty_Checked, CORE::BoolToString( m_settings.GetVSyncState() ) );
    CCombobox* presentMode = GetWidgetAs< CCombobox >( "PresentMode" );
    if ( GUCEF_NULL != presentMode )
        presentMode->SetSelectedIndex( (Int32) m_settings.GetPresentMode() );
    FillChoices( "PresentFilter", m_presentFilters, m_settings.GetPresentFilter() );

    Float32 sharpness = DefaultSharpness;
    CVideoSettings::TParameterMap::const_iterator p = m_settings.GetPresentFilterParameters().find( SharpnessParameter );
    if ( p != m_settings.GetPresentFilterParameters().end() )
        sharpness = CORE::StringToFloat( (*p).second, DefaultSharpness );
    CWidget* sharpnessSlider = GetWidget( "Sharpness" );
    if ( GUCEF_NULL != sharpnessSlider )
        sharpnessSlider->SetFloatProperty( EWidgetProperty_Value, sharpness * 100.0f );

    FillChoices( "ImageUpscaler", m_imageUpscalers, m_settings.GetImageUpscaler() );

    // the ways of upscaling movies the application has; a way it does not have shows as "as they are"
    CCombobox* movies = GetWidgetAs< CCombobox >( "MovieUpscaling" );
    if ( GUCEF_NULL != movies )
    {
        CCombobox::TStringVector items;
        items.push_back( MoviesAsTheyAreChoice );
        Int32 selected = 0;
        if ( m_movieFilterAvailable )
        {
            if ( CVideoSettings::MovieUpscalingFilter == m_settings.GetMovieUpscaling() )
                selected = (Int32) items.size();
            items.push_back( MoviesFilterChoice );
        }
        if ( m_movieCopiesAvailable )
        {
            if ( CVideoSettings::MovieUpscalingConverted == m_settings.GetMovieUpscaling() )
                selected = (Int32) items.size();
            items.push_back( MoviesCopiesChoice );
        }
        movies->SetListItems( items );
        movies->SetSelectedIndex( selected );
    }
    UpdateEnabledRows();

    SetReadingSettings( false );
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::OnValueChanged( CWidget& widget )
{GUCEF_TRACE;

    if ( widget.GetName() == "DisplayMode" && !IsReadingSettings() )
    {
        // the list of resolutions depends on full screen or not, the one picked stays when it can
        UInt32 width = m_settings.GetResolutionWidthInPixels(), height = m_settings.GetResolutionHeightInPixels();
        CCombobox* resolution = GetWidgetAs< CCombobox >( "Resolution" );
        if ( GUCEF_NULL != resolution )
        {
            Int32 index = resolution->GetSelectedIndex();
            if ( index >= 0 && index < (Int32) m_resolutions.size() )
            {
                width = m_resolutions[ index ].widthInPixels;
                height = m_resolutions[ index ].heightInPixels;
            }
        }
        bool fullscreen = GetChoice( "DisplayMode" ) == "Full screen";
        bool reading = IsReadingSettings();
        SetReadingSettings( true );
        FillResolutions( fullscreen, width, height );
        SetReadingSettings( reading );
    }
    else
    if ( widget.GetName() == "PresentFilter" )
    {
        UpdateEnabledRows();
    }
}

/*-------------------------------------------------------------------------*/

CVideoSettings
CVideoSettingsForm::GetSettingsFromWidgets( void )
{GUCEF_TRACE;

    CVideoSettings settings( m_settings );
    if ( GUCEF_NULL == GetRootWidget() )
        return settings;

    settings.SetFullscreenState( GetChoice( "DisplayMode" ) == "Full screen" );
    CCombobox* resolution = GetWidgetAs< CCombobox >( "Resolution" );
    if ( GUCEF_NULL != resolution )
    {
        Int32 index = resolution->GetSelectedIndex();
        if ( index >= 0 && index < (Int32) m_resolutions.size() )
        {
            const CVideoOptions::TDisplayMode& mode = m_resolutions[ index ];
            settings.SetResolutionWidthInPixels( mode.widthInPixels );
            settings.SetResolutionHeightInPixels( mode.heightInPixels );
            if ( settings.GetFullscreenState() && mode.frequency > 0 )
                settings.SetFrequency( mode.frequency );
        }
    }
    settings.SetVSyncState( CORE::StringToBool( GetWidgetProperty( "VSync", EWidgetProperty_Checked ), false ) );
    CCombobox* presentMode = GetWidgetAs< CCombobox >( "PresentMode" );
    if ( GUCEF_NULL != presentMode && presentMode->GetSelectedIndex() >= 0 )
        settings.SetPresentMode( (TPresentMode) presentMode->GetSelectedIndex() );

    CString filter = GetChoice( "PresentFilter" );
    settings.SetPresentFilter( filter == NoneChoice ? CString::Empty : filter );
    CVideoSettings::TParameterMap parameters = settings.GetPresentFilterParameters();
    CWidget* sharpnessSlider = GetWidget( "Sharpness" );
    if ( GUCEF_NULL != sharpnessSlider )
        parameters[ SharpnessParameter ] = CORE::FloatToString( sharpnessSlider->GetFloatProperty( EWidgetProperty_Value ) / 100.0f );
    settings.SetPresentFilterParameters( parameters );

    CString upscaler = GetChoice( "ImageUpscaler" );
    settings.SetImageUpscaler( upscaler == NoneChoice ? CString::Empty : upscaler );

    const CString movies = GetChoice( "MovieUpscaling" );
    if ( movies == MoviesFilterChoice )
        settings.SetMovieUpscaling( CVideoSettings::MovieUpscalingFilter );
    else
    if ( movies == MoviesCopiesChoice )
        settings.SetMovieUpscaling( CVideoSettings::MovieUpscalingConverted );
    else
    if ( GUCEF_NULL != GetWidget( "MovieUpscaling" ) && ( m_movieFilterAvailable || m_movieCopiesAvailable ) )
        settings.SetMovieUpscaling( CString::Empty );
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CVideoSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    m_settings = GetSettingsFromWidgets();
    SetHasChanges( false );

    bool success = true;
    TWindowContextPtr window = m_window.GetSharedPtr();
    if ( !m_backendName.IsNULLOrEmpty() && !window.IsNULL() )
    {
        TWindowManagerBackendPtr backend = CGuiGlobal::Instance()->GetWindowManager().GetBackend( m_backendName );
        if ( backend.IsNULL() || !backend->ApplyVideoSettings( window, m_settings ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoSettingsForm: The window manager backend \"" << m_backendName << "\" did not apply the video settings" );
            success = false;
        }
    }

    NotifyObservers( SettingsAppliedEvent );
    return success;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::ShowProgress( const CString& text, Float32 fraction )
{GUCEF_TRACE;

    if ( fraction < 0.0f )
        fraction = 0.0f;
    if ( fraction > 1.0f )
        fraction = 1.0f;
    CWidget* progressText = GetWidget( "ProgressText" );
    if ( GUCEF_NULL != progressText )
    {
        progressText->SetProperty( EWidgetProperty_Text, text );
        progressText->SetVisibility( true );
    }
    CWidget* progress = GetWidget( "Progress" );
    if ( GUCEF_NULL != progress )
    {
        progress->SetFloatProperty( EWidgetProperty_Value, fraction * 1000.0f );
        progress->SetVisibility( true );
    }
    CWidget* stop = GetWidget( "StopProgress" );
    if ( GUCEF_NULL != stop )
        stop->SetVisibility( true );
}

/*-------------------------------------------------------------------------*/

void
CVideoSettingsForm::HideProgress( void )
{GUCEF_TRACE;

    const char* names[] = { "ProgressText", "Progress", "StopProgress" };
    for ( UInt32 i=0; i<3; ++i )
        if ( GUCEF_NULL != GetWidget( names[ i ] ) )
            GetWidget( names[ i ] )->SetVisibility( false );
}

/*-------------------------------------------------------------------------*/

bool
CVideoSettingsForm::IsProgressShown( void ) const
{GUCEF_TRACE;

    CVideoSettingsForm* self = const_cast< CVideoSettingsForm* >( this );
    CWidget* progress = self->GetWidget( "Progress" );
    return GUCEF_NULL != progress && progress->GetBoolProperty( EWidgetProperty_Visible );
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CVideoSettingsForm";
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
