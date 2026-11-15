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
#ifndef GUCEF_GUI_CIMAGEVIEWFORM_H
#define GUCEF_GUI_CIMAGEVIEWFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_ETYPES_H
#include "gucefIMAGE_ETypes.h"
#define GUCEF_IMAGE_ETYPES_H
#endif /* GUCEF_IMAGE_ETYPES_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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

class CImageFrame;

/**
 *  A viewer of pictures, as an image tool needs one: the picture in a panel that pans (drag with the left button) and zooms
 *  (the wheel, about the pointer), fitted to the panel or at 1:1 (the buttons Fit and 1:1, the keys F and 1), with a tool bar
 *  of the choices that change what is shown:
 *      channel         RGB (colour), R, G, B or A (alpha as a grey picture); the buttons and SetChannel()
 *      alpha           alpha over a checkerboard, for RGB
 *      level           the mip level, from the levels of the image, or made from the first one when it has none
 *      frame           a frame of the image, called a cube face or an array layer when the application says so (SetFrameMeaning())
 *      colour space    shown as sRGB or as linear values, from the colour space the data is in (SetSourceColourSpace())
 *      tiling          the picture once (1x1) or 3 times in 3 rows (3x3)
 *      compare         with a second picture (SetImages()) the panel shows the first at the right and the second at the left of a
 *                      split line that the user drags: both with the same choices and the same pan and zoom
 *  and a status line with the pixel under the pointer: its coordinates, the value as stored and the value as shown.
 *
 *  What is shown is made when a choice changes, not every frame: the channel, the colour space and the level are gucefIMAGE
 *  operations (CChannelOperations::Swizzle() and ConvertColourSpace(), CPixelMapResampler for a level the image does not
 *  have) on a copy of the level, which becomes the picture of an ImageFrame; panning and zooming only move and size that frame.
 *  GetRebuildCount() counts the pictures made.
 *
 *  The images are not owned: they have to be there while the form shows them (SetImage( NULL ) lets go). The form knows nothing
 *  of what the picture is.
 *
 *  Events: PixelProbedEvent (TPixelProbedEventData) when the pointer moves over a pixel, ViewChangedEvent (TViewChangedEventData)
 *  for every change of the zoom, the pan, the channel, the level, the frame and the other choices.
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP CImageViewForm : public CFormEx
{
    public:

    static const CORE::CEvent PixelProbedEvent;
    static const CORE::CEvent ViewChangedEvent;

    static void RegisterEvents( void );

    enum EChannel
    {
        CHANNEL_RGB = 0 ,
        CHANNEL_RED     ,
        CHANNEL_GREEN   ,
        CHANNEL_BLUE    ,
        CHANNEL_ALPHA
    };

    /** What the frames of the image are, for the names the form gives them */
    enum EFrameMeaning
    {
        FRAMEMEANING_FRAMES = 0 ,
        FRAMEMEANING_CUBE_FACES ,
        FRAMEMEANING_ARRAY_LAYERS
    };

    enum EViewLimits
    {
        CHECKER_CELL_SIZE = 8
    };

    /** The data of ViewChangedEvent */
    struct SViewState
    {
        Float32 zoom;
        Int32 panX;             /**< where the top left of the picture is in the panel, in pixels */
        Int32 panY;
        EChannel channel;
        UInt32 frame;
        UInt32 level;
    };
    typedef struct SViewState TViewState;
    typedef CORE::CTCloneableObj< TViewState > TViewChangedEventData;

    /** The data of PixelProbedEvent */
    struct GUCEF_GUI_PUBLIC_CPP SPixelProbe
    {
        Int32 x;                    /**< the pixel of the picture (of the tile with 3x3) */
        Int32 y;
        bool inSecond;              /**< the pixel is of the second picture of a comparison */
        CString storedValue;        /**< the channels as they are in the picture: "R 255  G 128  B 0  A 255" */
        CString shownValue;         /**< the same of what is shown */

        SPixelProbe( void );
    };
    typedef struct SPixelProbe TPixelProbe;
    typedef CORE::CTCloneableObj< TPixelProbe > TPixelProbedEventData;

    CImageViewForm( void );

    explicit CImageViewForm( CIGUIContext* context );

    virtual ~CImageViewForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Image" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  What is shown
     */

    /** Shows a picture (not owned; NULL for none), fitted to the panel; every frame and level of it can be chosen */
    void SetImage( IMAGE::CImage* image );

    /** Shows two pictures to compare: 'first' at the right of the split line, 'second' at the left */
    void SetImages( IMAGE::CImage* first, IMAGE::CImage* second );

    IMAGE::CImage* GetImage( void ) const;

    IMAGE::CImage* GetSecondImage( void ) const;

    bool IsComparing( void ) const;

    /** The colour space the data of the pictures is in, sRGB by default */
    void SetSourceColourSpace( IMAGE::TColourSpace colourSpace );

    IMAGE::TColourSpace GetSourceColourSpace( void ) const;

    /*
     *  The choices of the tool bar
     */

    void SetChannel( EChannel channel );

    EChannel GetChannel( void ) const;

    void SetAlphaCheckerboard( bool on );

    bool GetAlphaCheckerboard( void ) const;

    /** The level is kept within the levels there are; level 0 is the picture in full size */
    void SetLevel( UInt32 level );

    UInt32 GetLevel( void ) const;

    /** How many levels there are: those of the first frame of the image, else the ones down to 1x1 that are made from it */
    UInt32 GetLevelCount( void ) const;

    void SetFrame( UInt32 frame );

    UInt32 GetFrame( void ) const;

    UInt32 GetFrameCount( void ) const;

    void SetFrameMeaning( EFrameMeaning meaning );

    EFrameMeaning GetFrameMeaning( void ) const;

    /** How the data is shown: as it is for sRGB data when this is sRGB, in linear light (converted) when linear, and the other way round */
    void SetDisplayColourSpace( IMAGE::TColourSpace colourSpace );

    IMAGE::TColourSpace GetDisplayColourSpace( void ) const;

    /** 1 for the picture once, 3 for 3x3 tiles; other numbers are refused */
    bool SetTiling( UInt32 tiles );

    UInt32 GetTiling( void ) const;

    /** The place of the split line as a part of the width of the panel, 0 to 1; with a comparison only */
    void SetSplit( Float32 part );

    Float32 GetSplit( void ) const;

    /*
     *  Pan and zoom
     */

    /** The zoom: 1 is one pixel of the picture on one pixel of the panel; kept between 1/64 and 64; the picture stays where it is on the panel */
    void SetZoom( Float32 zoom );

    Float32 GetZoom( void ) const;

    /** Zooms by a factor about a point of the panel (the wheel does this about the pointer) */
    void ZoomAt( Float32 factor, Int32 viewX, Int32 viewY );

    /** The picture as large as the panel allows, in the middle; it stays fitted when the panel changes size until the user pans or zooms */
    void FitToView( void );

    /** One pixel of the picture on one of the panel, in the middle */
    void ActualSize( void );

    void SetPan( Int32 panX, Int32 panY );

    Int32 GetPanX( void ) const;

    Int32 GetPanY( void ) const;

    bool IsFitted( void ) const;

    /** The size of the panel in pixels, as of the last frame */
    void GetViewSize( Int32& width, Int32& height ) const;

    /** Looks at a point of the panel as the pointer does: fills the status line and notifies PixelProbedEvent; false when no pixel is there */
    bool ProbeAt( Int32 viewX, Int32 viewY );

    /** What is shown now: the picture of the first image (second image) after the choices were applied, NULL when there is none */
    IMAGE::TPixelMapPtr GetShownPixels( void ) const;

    IMAGE::TPixelMapPtr GetSecondShownPixels( void ) const;

    /** How many times the shown pictures were made, not counting the second picture of a comparison */
    UInt32 GetRebuildCount( void ) const;

    /** The text of the status line */
    CString GetStatusText( void );

    /** Sees whether the panel changed size (a fitted picture follows it) and brings the pictures up to date; done after every frame of the context of the form */
    void Update( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CImageViewForm > TEventCallback;

    /** One of the two pictures: its source and what is made of it */
    struct SSlot
    {
        IMAGE::CImage* image;
        IMAGE::TPixelMapPtr shown;
        GUCEF::map< UInt32, IMAGE::TPixelMapPtr > made;     /**< levels made from the first one, by level */
        UInt32 shownWidth;
        UInt32 shownHeight;

        SSlot( void ) : image( GUCEF_NULL ), shown(), made(), shownWidth( 0 ), shownHeight( 0 ) {}
    };

    void OnChannelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFitActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnActualActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCheckerChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCheckerUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTileChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTileUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLevelValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnColourSpaceValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewMouseDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewMouseUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewMouseMove( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewMouseWheel( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewCaptureLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnViewKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameRendered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    bool GetLevelMap( SSlot& slot, UInt32 frame, UInt32 level, IMAGE::TPixelMapPtr& map );
    bool MakeShown( SSlot& slot );
    void RebuildShown( void );
    void ShowChoices( void );
    void ShowChannelButtons( void );
    void ApplyView( void );
    void NotifyViewChanged( void );
    void ClampLevelAndFrame( void );
    Float32 FittedZoom( void ) const;
    void SetZoomKeeping( Float32 zoom, Int32 viewX, Int32 viewY, bool user );
    bool ReadProbe( Int32 viewX, Int32 viewY, TPixelProbe& probe );
    CString FormatStored( const IMAGE::CPixelMap& map, UInt32 x, UInt32 y ) const;
    CString FormatShown( const IMAGE::CPixelMap& map, UInt32 x, UInt32 y ) const;
    CImageFrame* GetFrameWidget( const CString& name );

    CImageViewForm( const CImageViewForm& src );
    CImageViewForm& operator=( const CImageViewForm& src );

    private:

    SSlot m_first;
    SSlot m_second;
    IMAGE::TColourSpace m_sourceSpace;
    IMAGE::TColourSpace m_displaySpace;
    EChannel m_channel;
    bool m_checkerboard;
    UInt32 m_level;
    UInt32 m_frame;
    EFrameMeaning m_frameMeaning;
    UInt32 m_tiling;
    Float32 m_split;
    Float32 m_zoom;
    Int32 m_panX;
    Int32 m_panY;
    bool m_fitted;
    Int32 m_viewWidth;
    Int32 m_viewHeight;
    UInt32 m_rebuilds;
    bool m_panning;
    bool m_splitting;
    Int32 m_lastPointerX;
    Int32 m_lastPointerY;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
    CString m_status;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIMAGEVIEWFORM_H ? */
