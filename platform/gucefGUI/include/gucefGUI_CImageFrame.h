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

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#define GUCEF_GUI_CIMAGEFRAME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

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
 *  A picture. Image is the name of an image registered with the context, or a VFS path. Pixels can also be
 *  handed over directly (SetImage), which registers them under a name of the frame's own; after changing
 *  those pixels call UpdateImage(). An animation is a list of image names with a duration each, played by
 *  the frame itself. Properties: Image, SubRect (part of the image, empty for all), ScaleMode (Stretch,
 *  None, Fit, Integer, Center), Tint, BackgroundColour.
 */
class GUCEF_GUI_PUBLIC_CPP CImageFrame : public CWidget
{
    public:

    static const CORE::CEvent ImageChangedEvent;
    static const CORE::CEvent AnimationEndedEvent;

    static void RegisterEvents( void );

    public:

    typedef GUCEF::vector< CString > TStringVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    CImageFrame( void );

    CImageFrame( const CString& widgetTypeName );

    virtual ~CImageFrame();

    /** Shows the first frame of the image */
    virtual bool SetImage( const IMAGE::CImage& srcImage );

    /** Shows the given pixels; they are shared, not copied, so later changes show after UpdateImage() */
    bool SetImage( const IMAGE::TPixelMapPtr& pixels );

    /** Shows a registered image or a VFS path */
    bool SetImageName( const CString& imageName );

    CString GetImageName( void ) const;

    /** The pixels handed over with SetImage() changed */
    bool UpdateImage( void );

    /** Plays the images one after the other, each for its duration in milliseconds */
    bool SetAnimation( const TStringVector& images, const TUInt32Vector& durationsInMs, bool loop );

    void PlayAnimation( void );

    void StopAnimation( void );

    bool IsAnimationPlaying( void ) const;

    UInt32 GetAnimationFrame( void ) const;

    virtual bool GetNaturalSize( Int32& width, Int32& height ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CWidget* Create( void );

    protected:

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name under which pixels handed over with SetImage() are registered */
    const CString& GetOwnImageName( void );

    private:

    CImageFrame( const CImageFrame& src );
    CImageFrame& operator=( const CImageFrame& src );

    private:

    CString m_ownImage;
    TStringVector m_animationImages;
    TUInt32Vector m_animationDurations;
    bool m_animationLoop;
    bool m_animationPlaying;
    UInt32 m_animationFrame;
    UInt32 m_animationTime;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */
