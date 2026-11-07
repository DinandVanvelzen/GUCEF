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

#ifndef GUCEF_GUI_CCHARTMARKER_H
#define GUCEF_GUI_CCHARTMARKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
 *  One marker of a chart (CChartView): an annotation of a point in time, such as a split or a dividend on a price line.
 *  It has an id of the application (given back when the marker is activated), an x (milliseconds since the Unix epoch in
 *  UTC on a time axis, else the integer x), the series it sits on (or none: the lane at the bottom of the plot), a shape,
 *  a colour, a style class (the shape and colour of a group of markers, see CChartView::SetMarkerStyle()), a layer (a group
 *  that is shown or hidden together), a short label drawn above it and the text of its tooltip (lines separated by '\n').
 */
class GUCEF_GUI_PUBLIC_CPP CChartMarker
{
    public:

    enum EShape
    {
        SHAPE_DEFAULT       = 0 ,   /**< the shape of the style class, else a diamond */
        SHAPE_DIAMOND       = 1 ,
        SHAPE_TRIANGLE_UP   = 2 ,
        SHAPE_TRIANGLE_DOWN = 3 ,
        SHAPE_SQUARE        = 4 ,
        SHAPE_CIRCLE        = 5 ,
        SHAPE_FLAG          = 6     /**< a pole with a pennant */
    };
    typedef enum EShape TShape;

    CChartMarker( void );

    CChartMarker( const CString& id, Int64 x );

    /** The key of the application, given back with CChartView::MarkerActivatedEvent */
    void SetId( const CString& id );

    const CString& GetId( void ) const;

    /** Milliseconds since the epoch (UTC) on a time axis, else the integer x */
    void SetX( Int64 x );

    Int64 GetX( void ) const;

    /** The index of the series the marker sits on; -1 (the default): the lane at the bottom of the plot */
    void SetSeriesIndex( Int32 seriesIndex );

    Int32 GetSeriesIndex( void ) const;

    void SetShape( TShape shape );

    TShape GetShape( void ) const;

    /** An explicit colour; without one the marker takes the colour of its style class, else EmphasisColour of the chart */
    void SetColour( const CColour& colour );

    void ResetColour( void );

    bool HasColour( void ) const;

    const CColour& GetColour( void ) const;

    void SetStyleClass( const CString& styleClass );

    const CString& GetStyleClass( void ) const;

    /** Empty: the default layer */
    void SetLayer( const CString& layer );

    const CString& GetLayer( void ) const;

    /** A short text drawn above the marker, for example "2:1"; may be empty */
    void SetLabel( const CString& label );

    const CString& GetLabel( void ) const;

    /** The text of the tooltip of the marker, lines separated by '\n' */
    void SetTooltip( const CString& tooltip );

    const CString& GetTooltip( void ) const;

    /** "diamond", "triangle-up", "triangle-down", "square", "circle", "flag", "default" */
    static const char* ShapeToString( TShape shape );

    private:

    CString m_id;
    Int64 m_x;
    Int32 m_seriesIndex;
    TShape m_shape;
    bool m_hasColour;
    CColour m_colour;
    CString m_styleClass;
    CString m_layer;
    CString m_label;
    CString m_tooltip;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTMARKER_H ? */
