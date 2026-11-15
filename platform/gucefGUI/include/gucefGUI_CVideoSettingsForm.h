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

#ifndef GUCEF_GUI_CVIDEOSETTINGSFORM_H
#define GUCEF_GUI_CVIDEOSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The video settings: a prebaked page that shows a CVideoSettings and picks from what CVideoOptions
 *  offers. Widgets (by name):
 *
 *      DisplayMode     Window or Full screen
 *      Resolution      the usual game resolutions the display can show (CVideoOptions::GetCommonResolutions())
 *      VSync           wait for the vertical blank
 *      PresentMode     how the picture is scaled to the window: sharp pixels, smooth, whole multiples, stretched
 *      PresentFilter   "Screen filter": an upscaling filter of the GUI driver on the picture as it is shown, "None" first
 *      Sharpness       the "sharpness" parameter of the filter, 0 .. 100 %
 *      ImageUpscaler   "Art upscaler": an upscaler for the pictures of the application (made once), "None" first
 *      MovieUpscaling  "Movies": as they are, an upscaling filter while they play, or upscaled copies made once;
 *                      only the ways the application offers (SetAvailableMovieUpscaling())
 *      Renderer        the renderer the game draws with: "Automatic" first (the choice of the window manager
 *                      backend), then the renderers the application says are available (SetAvailableRenderers(),
 *                      from CWindowManagerBackend::GetAvailableRenderers())
 *      RendererNote    shown while the renderer picked is not the one in use: it is used from the next start
 *      Progress        with ProgressText: how far work after Apply is (ShowProgress()), hidden otherwise
 *      StopProgress    a button with the progress that asks to stop that work (ProgressStoppedEvent)
 *
 *  Only the filters and upscalers the application says are available are offered; without any those
 *  rows are disabled. Apply takes the values into the settings, applies the window part (size, full
 *  screen, vsync) to the window given with SetWindow() and notifies SettingsAppliedEvent; the application
 *  does the rest, for example the present filter and the upscaled pictures.
 */
class GUCEF_GUI_PUBLIC_CPP CVideoSettingsForm : public CSettingsPageForm
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    static const CString NoneChoice;
    static const CString AutomaticChoice;   /**< "Automatic": the first renderer choice, no renderer in the settings */
    static const CString RendererNoteText;  /**< the text of RendererNote */

    /** The texts of the Movies choices */
    static const CString MoviesAsTheyAreChoice;
    static const CString MoviesFilterChoice;
    static const CString MoviesCopiesChoice;

    static const CORE::CEvent ProgressStoppedEvent; /**< the user pressed StopProgress: the work of the progress should stop */

    static void RegisterEvents( void );

    CVideoSettingsForm( void );

    virtual ~CVideoSettingsForm();

    /** The settings in use; ReadSettings() shows them */
    void SetVideoSettings( const CVideoSettings& settings );

    const CVideoSettings& GetVideoSettings( void ) const;

    /** What the display offers; by default what the system says (CVideoOptions::RetrieveOptionsFromOS()) */
    void SetVideoOptions( const CVideoOptions& options );

    const CVideoOptions& GetVideoOptions( void ) const;

    /** The present filters that can be picked, by name */
    void SetAvailablePresentFilters( const TStringVector& filterNames );

    /** The image upscalers that can be picked, by name */
    void SetAvailableImageUpscalers( const TStringVector& upscalerNames );

    /** Which ways of upscaling movies the application has: a filter while they play, upscaled copies */
    void SetAvailableMovieUpscaling( bool filter, bool copies );

    /**
     *  The renderers that can be picked, by name, and the one the windows draw with now (the property "RENDERER"
     *  of the window, empty when it is not known). Without renderers the row is disabled and Apply keeps the
     *  renderer of the settings as it is
     */
    void SetAvailableRenderers( const TStringVector& rendererNames, const CString& rendererInUse );

    /** The window Apply resizes, switches to full screen and sets the vsync of, through the window manager backend; held weakly */
    void SetWindow( const CString& windowManagerBackendName, TWindowContextPtr window );

    /** Shows how far work after Apply is, 'fraction' 0 .. 1, with a text */
    void ShowProgress( const CString& text, Float32 fraction );

    void HideProgress( void );

    bool IsProgressShown( void ) const;

    /** The settings as the widgets have them now */
    CVideoSettings GetSettingsFromWidgets( void );

    virtual CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The text of a resolution in the list, like "1280 x 960" */
    static CString ResolutionToString( UInt32 width, UInt32 height );

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnValueChanged( CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CVideoSettingsForm > TEventCallback;

    /** The button that stops what the progress shows was activated */
    void OnStopProgressActivated( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );

    void FillResolutions( bool fullscreen, UInt32 width, UInt32 height );

    void FillChoices( const CString& widgetName, const TStringVector& choices, const CString& selected );
    void FillChoices( const CString& widgetName, const CString& firstChoice, const TStringVector& choices, const CString& selected );
    void UpdateRendererNote( void );

    void UpdateEnabledRows( void );

    CString GetChoice( const CString& widgetName );

    CVideoSettingsForm( const CVideoSettingsForm& src );
    CVideoSettingsForm& operator=( const CVideoSettingsForm& src );

    private:

    CVideoSettings m_settings;
    CVideoOptions m_options;
    CVideoOptions::TDisplayModeVector m_resolutions;    /**< what the Resolution list shows, in its order */
    TStringVector m_presentFilters;
    TStringVector m_imageUpscalers;
    bool m_movieFilterAvailable;
    bool m_movieCopiesAvailable;
    TStringVector m_renderers;
    CString m_rendererInUse;
    CString m_backendName;
    CORE::CTWeakPtr< CWindowContext, MT::CMutex > m_window;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVIDEOSETTINGSFORM_H ? */
