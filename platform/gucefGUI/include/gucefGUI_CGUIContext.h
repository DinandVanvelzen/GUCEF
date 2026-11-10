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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#define GUCEF_GUI_CGUICONTEXT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CBITMAPFONT_H
#include "gucefGUI_CBitmapFont.h"
#define GUCEF_GUI_CBITMAPFONT_H
#endif /* GUCEF_GUI_CBITMAPFONT_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

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

class CGUIDriver;
class CForm;
class CScrollPanel;

/**
 *  The part of a GUI context that is the same for every backend. A driver derives its context from
 *  this class and adds rendering, frame capture and, if it wants, its own text measuring.
 *
 *  It owns the widgets, lays them out (UDim positions and sizes, alignment and the container widgets),
 *  keeps the shown forms in order (the last one on top, modal ones keep the input), turns raw input
 *  into routed widget events (hit testing, hover, press and click, double clicks, keyboard focus,
 *  navigation), holds the named images and fonts, applies the theme, and passes changes to the
 *  driver's widget handlers once per frame.
 *
 *  Input is given in logical pixels with the Inject*() methods (tests, scripts and replays use the
 *  same path), or in window pixels with the ProcessWindow*() methods, which map through the present
 *  rectangle first.
 */
class GUCEF_GUI_PUBLIC_CPP CGUIContext : public CIGUIContext                ,
                                         public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent FrameStartedEvent;        /**< before a frame is updated and drawn; TCloneableUInt32 with the frame time in milliseconds */
    static const CORE::CEvent FrameRenderedEvent;       /**< after a frame was drawn */
    static const CORE::CEvent FormShownEvent;           /**< TCloneableString: the name of the root widget of the form */
    static const CORE::CEvent FormHiddenEvent;

    static void RegisterEvents( void );

    public:

    struct SImageEntry
    {
        IMAGE::TPixelMapPtr pixels;     /**< RGBA, 8 bits per channel */
        UInt32 revision;                /**< goes up whenever the pixels change */
        UInt32 density;                 /**< pixels per logical pixel in each direction */

        SImageEntry( void );
    };
    typedef struct SImageEntry TImageEntry;

    typedef GUCEF::vector< CWidget* > TWidgetVector;

    CGUIContext( CGUIDriver* driver, TWindowContextPtr windowContext );

    virtual ~CGUIContext();

    /*
     *  Widgets and forms
     */

    /** Makes a widget of a registered type, below the given parent (NULL: a root, hidden until shown as a form) */
    CWidget* CreateWidget( const CString& widgetType, const CString& widgetName, CWidget* parent );

    /** CIGUIContext: makes a widget of the given type without a name or parent */
    virtual CWidget* CreateWidget( const CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE;

    /** Destroys the widget and every widget below it */
    virtual void DestroyWidget( CWidget* widget ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes a form of a type registered with the GUI manager ("Form", "FormEx" or an application type) */
    virtual CForm* CreateForm( const CString& formType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyForm( CForm* form ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFormTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The registered widget types the driver has a handler for */
    virtual TStringSet GetAvailableWidgetTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** A backend native layout loader, for layouts that are not gucefGUI layouts; NULL when the driver has none */
    virtual CFormBackend* CreateFormBackend( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyFormBackend( CFormBackend* formBackend ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CGUIDriver* GetDriver( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TWidgetSet GetOwnedWidgets( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TFormSet GetOwnedForms( void ) GUCEF_VIRTUAL_OVERRIDE;

    TWindowContextPtr GetWindowContext( void ) const;

    /** Every root widget (the roots of the forms), in drawing order: the last is on top */
    const TWidgetVector& GetRootWidgets( void ) const;

    /** Shows a root widget on top of the others; with 'modal' input outside of it is ignored until it is hidden */
    bool ShowRoot( CWidget* root, bool modal );

    bool HideRoot( CWidget* root );

    bool IsModalRoot( const CWidget* root ) const;

    /** Whether the root is shown (one of GetRootWidgets()) */
    bool IsRootShown( const CWidget* root ) const;

    /** The topmost visible modal root, NULL when none */
    CWidget* GetModalRoot( void ) const;

    /** Opens a popup widget (a menu, the list of a combo box) at the given place on top of everything; it closes on a click outside */
    bool ShowPopup( CWidget* popup, Int32 x, Int32 y );

    bool ClosePopup( void );

    CWidget* GetPopup( void ) const;

    /*
     *  Images and fonts
     */

    /** Makes pixels available under a name; widgets refer to images by these names. RGBA pixels are expected; other formats are converted */
    bool RegisterImage( const CString& alias, const IMAGE::TPixelMapPtr& pixels );

    /**
     *  As RegisterImage() for a picture with 'density' pixels per logical pixel in each direction: one that
     *  was made for a render scale (see SetRenderScale()). Its logical size is its size in pixels divided by
     *  the density and source rectangles of it (SubRect, CDrawSurface::DrawImage()) are in logical pixels.
     */
    bool RegisterImage( const CString& alias, const IMAGE::TPixelMapPtr& pixels, UInt32 density );

    /** The density of an image, 1 for images that are not registered */
    UInt32 GetImageDensity( const CString& alias ) const;

    /** The size of an image in logical pixels; false when the reference does not resolve */
    bool GetImageLogicalSize( const CString& reference, Int32& width, Int32& height );

    /** Tells the context and the driver that the pixels of a registered image changed */
    bool UpdateImage( const CString& alias );

    bool UnregisterImage( const CString& alias );

    bool HasImage( const CString& alias ) const;

    /**
     *  The pixels for an image reference: a registered alias, else a file in the VFS (loaded with the
     *  gucefIMAGE codecs and remembered under its path)
     */
    bool ResolveImage( const CString& reference, IMAGE::TPixelMapPtr& pixels, UInt32& revision );

    /** A name that is not used for an image yet, for images widgets register for themselves */
    CString MakeUniqueImageAlias( const CString& prefix );

    bool RegisterFont( const CString& name, const TBitmapFontPtr& font );

    /**
     *  Rasterizes an outline font (TrueType, OpenType: a VFS path) at a pixel size, Latin-1, with the font
     *  rasterizers of the loaded plugins and registers it; false when no rasterizer reads it
     */
    bool RegisterFont( const CString& name, const CString& fontPath, UInt32 pixelSize );

    bool UnregisterFont( const CString& name );

    TBitmapFontPtr GetBitmapFont( const CString& name ) const;

    /** The height of a line of text in a font: the bitmap font's line height, else what MeasureText() says, else 16 */
    Int32 GetLineHeight( const CString& fontName );

    /** The size of text in a font; the default handles the registered bitmap fonts, a driver adds its own fonts */
    virtual bool MeasureText( const CString& fontName, const CString& text, Int32 maxWidth, Int32& width, Int32& height );

    /*
     *  Theme
     */

    /**
     *  Sets the theme: a DataNode "GUITheme" with "Style" children. A style has optional "type", "class",
     *  "name" and "state" attributes (state: hover, pressed, focused, disabled, checked; several joined with ',')
     *  and "Property" children with "name" and "value". The most specific matching style wins
     *  (name before class before type, more states before fewer). Visible, Enabled and Checked are the
     *  state styles select on and are never taken from the theme.
     *
     *  The tree is read once, when it is set: every style is kept with the readings of its attributes and
     *  the styles are indexed by the id of the standard properties they set (by name for any other), so
     *  looking up a value takes only the styles that set the property and compares no property names.
     */
    bool SetTheme( const CORE::CDataNode& theme );

    const CORE::CDataNode& GetTheme( void ) const;

    /** The value the theme gives the property of the widget in its current state */
    bool GetThemeProperty( const CWidget& widget, EWidgetProperty property, CString& value ) const;
    bool GetThemeProperty( const CWidget& widget, const CString& name, CString& value ) const;

    /**
     *  The same without a copy: NULL when the theme gives no value. The value (which remembers what it was read as)
     *  is good until the next SetTheme().
     */
    const CWidgetPropertyValue* FindThemeValue( const CWidget& widget, EWidgetProperty property ) const;
    const CWidgetPropertyValue* FindThemeValue( const CWidget& widget, const CString& name ) const;

    /*
     *  Presentation
     */

    /** The size of the picture everything is laid out in; 0 x 0 means the size of the window */
    void SetLogicalResolution( UInt32 width, UInt32 height );

    UInt32 GetLogicalWidth( void ) const;

    UInt32 GetLogicalHeight( void ) const;

    void SetPresentMode( TPresentMode mode );

    TPresentMode GetPresentMode( void ) const;

    /**
     *  How many pixels a logical pixel has in each direction when the context draws: the frame is the logical
     *  resolution times the scale, layout and drawing stay in logical pixels. Images and fonts with that
     *  density (pictures made for the scale, upscaled art) are drawn pixel for pixel, others are scaled up.
     *  1 by default.
     */
    void SetRenderScale( UInt32 scale );

    UInt32 GetRenderScale( void ) const;

    /** The size of the frame: the logical resolution times the render scale */
    UInt32 GetRenderWidth( void ) const;

    UInt32 GetRenderHeight( void ) const;

    typedef GUCEF::map< CString, CString > TPresentFilterParameters;
    typedef GUCEF::vector< CString > TPresentFilterNames;

    /**
     *  The present filters the driver offers for this context: upscalers that work on the logical picture as it
     *  is presented in the window (drivers with them report the capability "PresentFilters"). None by default.
     */
    virtual TPresentFilterNames GetAvailablePresentFilters( void ) const;

    /** Picks the present filter, empty for none; false (and nothing changes) when the driver does not offer it */
    bool SetPresentFilter( const CString& filterName, const TPresentFilterParameters& parameters );

    const CString& GetPresentFilter( void ) const;

    const TPresentFilterParameters& GetPresentFilterParameters( void ) const;

    /** Where the logical picture lands in the window */
    CRect GetPresentRect( void ) const;

    /** Window pixels to logical pixels; false when the point is outside the picture */
    bool WindowToLogical( Int32 windowX, Int32 windowY, Int32& x, Int32& y ) const;

    /** A picture drawn at the pointer instead of the system cursor (hidden over the window); an empty alias goes back to the system cursor */
    void SetCursor( const CString& imageAlias, Int32 hotspotX, Int32 hotspotY );

    void SetCursorVisible( bool visible );

    bool IsCursorVisible( void ) const;

    const CString& GetCursorImage( void ) const;

    Int32 GetCursorHotspotX( void ) const;

    Int32 GetCursorHotspotY( void ) const;

    /** Where the pointer is, in logical pixels */
    Int32 GetPointerX( void ) const;

    Int32 GetPointerY( void ) const;

    /*
     *  Input
     */

    void InjectMouseMove( Int32 x, Int32 y );

    void InjectMouseButton( TMouseButton button, bool down );

    void InjectMouseWheel( Int32 delta );

    /**
     *  Ends the press of the widget that holds the pointer without a release (the window lost the focus, the input was
     *  cancelled): the widget gets MouseCaptureLostEvent. Done by the context itself when that widget is hidden or disabled
     *  or when a layer above takes the input. The reason (optional) is what the user action log says.
     */
    void CancelPointerCapture( const char* reason = GUCEF_NULL );

    void InjectKey( INPUT::KeyCode keyCode, bool down, UInt32 modifiers );

    /** UTF-8 text typed by the user */
    void InjectText( const CString& text );

    void InjectNavigation( TNavigation direction );

    void ProcessWindowMouseMove( Int32 windowX, Int32 windowY );

    /** The topmost widget at a point that takes input (visible, not input transparent), respecting modal roots and the popup */
    CWidget* HitTest( Int32 x, Int32 y ) const;

    CWidget* GetFocus( void ) const;

    bool SetFocus( CWidget* widget );

    CWidget* GetHovered( void ) const;

    /** How long the pointer has been on the widget it is on now, in context time */
    UInt64 GetHoverDurationInMs( void ) const;

    /** Moves the focus like the navigation keys do; false when there is nowhere to go */
    bool Navigate( TNavigation direction );

    /*
     *  Frames
     */

    /** Milliseconds the context has run, the sum of the frame times */
    UInt64 GetTimeInMs( void ) const;

    /** Updates (animations, video, caret), lays out, passes changes to the handlers, then draws; for a frame of the given duration */
    void RenderFrame( UInt32 deltaInMs );

    /** Lays out and passes changes to the handlers without drawing */
    void FlushChanges( void );

    /** Asks for a new layout pass at the next flush */
    void InvalidateLayout( void );

    /** The last frame as RGBA pixels at the render size (the logical resolution times the render scale); false when the driver cannot */
    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels );

    /**
     *  The picture of the window as the last frame left it, RGBA pixels at the size of the window: what the present
     *  pass made of the logical picture (scaled, filtered, other layers). False when the driver cannot
     */
    virtual bool CapturePresentedFrame( IMAGE::TPixelMapPtr& pixels );

    /*
     *  Layers: several contexts can draw in one window. The lowest layer is drawn first and fills the window, the
     *  layers above are drawn over it with their transparency (each at its own logical resolution). Input goes to
     *  the highest layer that shows something (has a visible root); the layers below get none while it does.
     */

    /** The layer of the context in its window; contexts with the same number keep the order they were made in */
    void SetLayer( Int32 layer );

    Int32 GetLayer( void ) const;

    /** Whether the context is the lowest layer of its window (also without a window) */
    bool IsBottomLayer( void ) const;

    /** Whether a root is shown */
    bool HasVisibleRoots( void ) const;

    /** Whether a higher layer of the window shows something and so takes the input */
    bool HasHigherLayerTakingInput( void ) const;

    /** Draws a frame of every layer of the window from the bottom up, or of this context when it has no window */
    void RenderWindowLayers( UInt32 deltaInMs );

    protected:

    /** The driver draws the context */
    virtual void OnRender( void );

    /** Destroys every form and widget; a driver context calls it in its destructor while its backend still exists */
    void DestroyAllWidgets( void );

    /** The order of the roots changed (shown, hidden, brought to front) */
    virtual void OnRootOrderChanged( void );

    /** A registered image was added or changed or removed */
    virtual void OnImageChanged( const CString& alias );

    /** The driver's handler for a widget type, the type chain is searched by the context */
    virtual CWidgetHandler* GetWidgetHandler( const CString& widgetType );

    virtual CGUIContext* AsGUIContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    friend class CWidget;

    typedef CORE::CTEventHandlerFunctor< CGUIContext > TEventCallback;

    /** The window context wants the layers drawn */
    void OnWindowRedraw( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    /** The window context changed size */
    void OnWindowSize( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    /** The window was activated or deactivated */
    void OnWindowActivate( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnWidgetDirty( CWidget* widget );
    void OnWidgetDestroyed( CWidget* widget );
    void OnWidgetParentChanged( CWidget* widget, CWidget* oldParent );
    void OnWidgetVisibilityChanged( CWidget* widget );
    void UpdateLayout( void );
    void FlushLayoutIfNeeded( void );
    void ResolveSize( const CWidget& widget, const CRect& area, Int32& width, Int32& height ) const;
    void LayoutChildren( CWidget& widget );
    void LayoutScrollPanel( CScrollPanel& panel );
    bool ScrollPanelByKey( INPUT::KeyCode keyCode, UInt32 modifiers );
    void ShiftSubtree( CWidget& widget, Int32 dy );
    void PlaceWidget( CWidget& widget, const CRect& area, bool sizeFromArea );
    void ApplyLayoutResult( CWidget& widget, const CRect& rect );
    /** Updates the hover for the pointer position; the widget at the position (what HitTest finds, also when it takes no input) */
    CWidget* UpdatePointerTarget( void );
    void ForgetInteraction( CWidget* widget, bool widgetIsIntact );
    void ProcessPendingDestroys( void );
    CWidget* GetKeyboardTarget( void ) const;
    void SetHovered( CWidget* widget );
    void DispatchPointer( CWidget* target, const CORE::CEvent& eventId, TMouseButton button, UInt32 clickCount );
    bool AcceptsInput( const CWidget* widget ) const;
    CWidget* HitTestWidget( CWidget* widget, Int32 x, Int32 y ) const;
    CWidget* GetRootOf( CWidget* widget ) const;
    void CollectFocusable( CWidget* widget, TWidgetVector& focusable ) const;
    void FlushWidget( CWidget& widget );
    void ApplyPropertyToHandler( CWidget& widget, CWidgetHandler& handler, const CString& name, const CString& value );
    void MarkThemeDependentDirty( CWidget& widget );

    private:

    CGUIContext( const CGUIContext& src );
    CGUIContext& operator=( const CGUIContext& src );

    /** Keeps the widgets that observers run for alive while an event is dispatched, see the implementation */
    class CDispatchScope;
    friend class CDispatchScope;

    typedef GUCEF::map< CString, TImageEntry > TImageMap;
    typedef GUCEF::map< CString, TBitmapFontPtr > TFontMap;
    typedef GUCEF::map< CString, CWidgetHandler* > THandlerCache;

    /** A property a style of the theme sets */
    struct SThemeProperty
    {
        EWidgetProperty property;
        CString name;
        CWidgetPropertyValue value;
    };

    /** A style of the theme with the readings of its attributes */
    struct SThemeStyle
    {
        CString type;
        CString styleClass;
        CString name;
        bool hasState;
        GUCEF::vector< UInt32 > stateFlags;                 /**< CWidget::EState flags of the state names; 0 for a name that is none */
        GUCEF::vector< SThemeProperty > properties;

        SThemeStyle( void );
    };

    /** Where a style of the index sets a property: the style and the (last) mention of the property in it */
    struct SThemeIndexEntry
    {
        UInt32 style;
        UInt32 property;
    };

    typedef GUCEF::vector< SThemeIndexEntry > TThemeIndexVector;

    /** The styles of the theme by the property they set, in the order of the theme: a property that no style sets is found without looking at a style */
    typedef GUCEF::map< CString, TThemeIndexVector > TThemeCustomIndex;

    bool StyleMatches( const SThemeStyle& style, const CWidget& widget, bool ignoreState, Int32& score, UInt32& widgetState, bool& widgetStateKnown ) const;
    const CWidgetPropertyValue* FindThemeValue( const CWidget& widget, const TThemeIndexVector& index ) const;
    void CompileTheme( void );

    CGUIDriver* m_driver;
    TWindowContextPtr m_windowContext;
    TWidgetSet m_widgets;
    TFormSet m_forms;
    TWidgetVector m_roots;
    TWidgetSet m_modalRoots;
    TWidgetSet m_dirtyWidgets;
    TWidgetSet m_updateWidgets;
    TImageMap m_images;
    TFontMap m_fonts;
    THandlerCache m_handlerCache;
    CORE::CDataNode m_theme;
    GUCEF::vector< SThemeStyle > m_themeStyles;
    GUCEF::vector< UInt32 > m_themeStateStyles;
    TThemeIndexVector m_themeIndex[ EWidgetProperty_Count ];
    TThemeCustomIndex m_themeCustomIndex;
    UInt32 m_logicalWidth;
    UInt32 m_logicalHeight;
    UInt32 m_renderScale;
    Int32 m_layer;
    TPresentMode m_presentMode;
    CString m_presentFilter;
    TPresentFilterParameters m_presentFilterParameters;
    CString m_cursorImage;
    Int32 m_cursorHotspotX;
    Int32 m_cursorHotspotY;
    bool m_cursorVisible;
    Int32 m_pointerX;
    Int32 m_pointerY;
    UInt32 m_buttonStates;
    UInt32 m_modifiers;
    CWidget* m_hovered;
    CWidget* m_pressed;
    TMouseButton m_pressedButton;
    UInt32 m_pressedMoveCount;
    CWidget* m_focus;
    CWidget* m_popup;
    CWidget* m_lastClickTarget;
    UInt64 m_lastClickTime;
    Int32 m_lastClickX;
    Int32 m_lastClickY;
    UInt32 m_clickCount;
    UInt64 m_timeInMs;
    UInt64 m_hoverStartTime;
    UInt64 m_lastRealTick;
    UInt32 m_uniqueCounter;
    bool m_layoutDirty;
    bool m_rootOrderDirty;
    bool m_inFlush;
    UInt32 m_dispatchDepth;
    TWidgetVector m_pendingDestroy;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGUICONTEXT_H ? */
