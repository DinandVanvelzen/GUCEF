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

#ifndef GUCEF_GUI_CSLIDER_H
#define GUCEF_GUI_CSLIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

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
 *  A value between Min and Max picked by dragging a thumb along a track. Registered as "Slider" and as
 *  "Scrollbar" (whose thumb is as long as PageSize is of the range). Properties: Value, Min, Max, Step,
 *  Orientation (Horizontal or Vertical), ThumbSize, PageSize, TrackColour, ThumbColour, TrackImage, ThumbImage.
 */
class GUCEF_GUI_PUBLIC_CPP CSlider : public CWidget
{
    public:

    CSlider( void );

    CSlider( const CString& widgetTypeName );

    virtual ~CSlider();

    Float32 GetValue( void ) const;

    bool SetValue( Float32 value );

    bool IsVertical( void ) const;

    /** Where the thumb is, relative to the widget */
    CRect GetThumbRect( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "Slider"; }

    static CWidget* Create( void );

    static CWidget* CreateScrollbar( void );

    protected:

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Int32 GetThumbLength( void ) const;
    Float32 ValueAt( Int32 localX, Int32 localY ) const;
    Float32 Snap( Float32 value ) const;

    CSlider( const CSlider& src );
    CSlider& operator=( const CSlider& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSLIDER_H ? */
