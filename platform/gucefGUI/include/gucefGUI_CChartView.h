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

#ifndef GUCEF_GUI_CCHARTVIEW_H
#define GUCEF_GUI_CCHARTVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CCHARTSERIES_H
#include "gucefGUI_CChartSeries.h"
#define GUCEF_GUI_CCHARTSERIES_H
#endif /* GUCEF_GUI_CCHARTSERIES_H ? */

#ifndef GUCEF_GUI_CCHARTAXIS_H
#include "gucefGUI_CChartAxis.h"
#define GUCEF_GUI_CCHARTAXIS_H
#endif /* GUCEF_GUI_CCHARTAXIS_H ? */

#ifndef GUCEF_GUI_CCHARTDECIMATOR_H
#include "gucefGUI_CChartDecimator.h"
#define GUCEF_GUI_CCHARTDECIMATOR_H
#endif /* GUCEF_GUI_CCHARTDECIMATOR_H ? */

#ifndef GUCEF_GUI_CCHARTMARKER_H
#include "gucefGUI_CChartMarker.h"
#define GUCEF_GUI_CCHARTMARKER_H
#endif /* GUCEF_GUI_CCHARTMARKER_H ? */

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
 *  A chart of series over a time axis (milliseconds since the Unix epoch, UTC) or a numeric x axis: lines,
 *  filled areas and bars (stacked per stack group), a value axis with chosen ticks (linear or log), a legend,
 *  and a crosshair with a tooltip that follows the pointer. The chart draws itself with the commands of a
 *  draw surface, so every driver that shows a draw surface shows it.
 *
 *  The drawing is rebuilt only when something changed (data, settings, properties, theme values, size,
 *  crosshair); large series are reduced to at most four points per pixel column (CChartDecimator), and the
 *  buffers of a rebuild are reused. Lookups (GetNearestPoints()) work on the data alone, without a window.
 *
 *  Markers (CChartMarker) annotate points in time, on a series or in a lane at the bottom of the plot: they are
 *  drawn with the chart, hit tested, have a tooltip, are activated by a click or by Enter (MarkerActivatedEvent)
 *  and are selected with the arrow keys, Home and End while a chart with Focusable set has the keyboard focus.
 *
 *  Properties (beyond those of DrawSurface): Title, Font, TextColour, AxisTextColour, BackgroundColour,
 *  AxisColour, GridColour, CrosshairColour, TooltipColour, SeriesColours, CrosshairVisible, LegendVisible,
 *  LegendPosition, XAxisType, LogScale, IncludeZero, ValuePrefix, ValueSuffix, Decimals, EmphasisColour
 *  (markers without a colour), SelectionColour (the selected marker, the frame of the focus).
 */
class GUCEF_GUI_PUBLIC_CPP CChartView : public CDrawSurface
{
    public:

    /** The crosshair moved to another x, or went away; not routed, no data: read GetCrosshairPoints() */
    static const CORE::CEvent CrosshairChangedEvent;

    /** A marker was clicked, or Enter was pressed while a marker is selected; routed, CItemEventData: the index and the id of the marker */
    static const CORE::CEvent MarkerActivatedEvent;

    /** The selected marker changed; not routed, no data: read GetSelectedMarker() */
    static const CORE::CEvent MarkerSelectionChangedEvent;

    /** The width and height of a marker shape, and the pixels around it that still hit it */
    static const Int32 MarkerSize;
    static const Int32 MarkerHitSlop;

    static void RegisterEvents( void );

    /** What a lookup finds for one series */
    struct SSeriesPoint
    {
        UInt32 seriesIndex;
        UInt32 pointIndex;
        Int64 x;
        Float64 value;          /**< the value of the point */
        Float64 plotValue;      /**< where it is drawn: the top of its segment for a stacked bar, else the value */
    };
    typedef struct SSeriesPoint TSeriesPoint;
    typedef GUCEF::vector< TSeriesPoint > TSeriesPointVector;
    typedef GUCEF::vector< CChartSeries > TSeriesVector;
    typedef GUCEF::vector< CChartMarker > TMarkerVector;

    CChartView( void );

    virtual ~CChartView();

    /** Adds a series at the end; returns its index, which is its place in the legend and in the palette */
    UInt32 AddSeries( const CString& name, CChartSeries::TSeriesType type );

    UInt32 GetSeriesCount( void ) const;

    /** NULL for a bad index; the pointer is good until the next AddSeries() or RemoveSeries() */
    CChartSeries* GetSeries( UInt32 index );

    const CChartSeries* GetSeries( UInt32 index ) const;

    /** -1 when there is none */
    Int32 FindSeries( const CString& name ) const;

    bool RemoveSeries( UInt32 index );

    void RemoveAllSeries( void );

    /** CChartSeries::SetData() of the series with that index */
    bool SetSeriesData( UInt32 index, const Int64* x, const Float64* values, UInt32 count );

    bool SetSeriesData( UInt32 index, const UInt64* x, const Float64* values, UInt32 count );

    /** The colour a series is drawn in: its own colour, else the palette (SeriesColours) colour of its index */
    CColour GetSeriesColour( UInt32 index ) const;

    CChartAxis& GetXAxis( void );

    const CChartAxis& GetXAxis( void ) const;

    CChartAxis& GetValueAxis( void );

    const CChartAxis& GetValueAxis( void ) const;

    /** The drawing is out of date: it is rebuilt before the next frame is drawn */
    void Invalidate( void );

    bool IsDirty( void ) const;

    /** Rebuilds the drawing now for the size of the widget */
    void Rebuild( void );

    /** Rebuilds the drawing now for a given size, for a chart that has no context or layout */
    void Rebuild( Int32 width, Int32 height );

    /** The plot area of the last rebuild, relative to the widget */
    const CRect& GetPlotRect( void ) const;

    /** The pixel column of x in the last rebuild, relative to the widget */
    Int32 XToPixel( Int64 x ) const;

    Int64 PixelToX( Int32 localX ) const;

    /** The pixel row of a value in the last rebuild, relative to the widget; a value a log scale cannot show gives the bottom of the plot */
    Int32 ValueToPixel( Float64 value ) const;

    Float64 PixelToValue( Int32 localY ) const;

    /**
     *  The point of every visible, non empty series nearest to x, in series order; 'points' is cleared first.
     *  Points without a value (NaN) are skipped. Needs no window, no context and no rebuild. Returns the number of points.
     */
    UInt32 GetNearestPoints( Int64 x, TSeriesPointVector& points ) const;

    /** The same for a pixel column of the last rebuild */
    UInt32 GetNearestPointsAtPixel( Int32 localX, TSeriesPointVector& points ) const;

    /** Puts the crosshair at the x of the point nearest to 'x' over all visible series; false when there are no points */
    bool SetCrosshairX( Int64 x );

    /** The same for a pixel column of the last rebuild; false outside the plot or without points */
    bool SetCrosshairPixel( Int32 localX );

    void ClearCrosshair( void );

    bool HasCrosshair( void ) const;

    Int64 GetCrosshairX( void ) const;

    /** The nearest points at the crosshair */
    const TSeriesPointVector& GetCrosshairPoints( void ) const;

    /** Adds a marker at the end; returns its index, good until the next AddMarker(), SetMarkers() or RemoveMarker() */
    UInt32 AddMarker( const CChartMarker& marker );

    /** Replaces every marker; the selection stays on a marker with the id of the selected one, if there is one */
    void SetMarkers( const TMarkerVector& markers );

    UInt32 GetMarkerCount( void ) const;

    /** NULL for a bad index */
    const CChartMarker* GetMarker( UInt32 index ) const;

    /** The index of the first marker with the id, -1 when there is none */
    Int32 FindMarker( const CString& id ) const;

    bool RemoveMarker( UInt32 index );

    void RemoveAllMarkers( void );

    /** The shape and colour of the markers of a style class that have none of their own */
    void SetMarkerStyle( const CString& styleClass, CChartMarker::TShape shape, const CColour& colour );

    bool GetMarkerStyle( const CString& styleClass, CChartMarker::TShape& shape, CColour& colour ) const;

    void RemoveMarkerStyle( const CString& styleClass );

    /** The markers of a hidden layer are not drawn, not hit and not reachable by keys; every layer is visible until it is hidden */
    void SetMarkerLayerVisible( const CString& layer, bool visible );

    bool IsMarkerLayerVisible( const CString& layer ) const;

    /** The shape a marker is drawn with: its own, else that of its style class, else a diamond */
    CChartMarker::TShape GetMarkerShape( UInt32 index ) const;

    /** The colour a marker is drawn in: its own, else that of its style class, else EmphasisColour */
    CColour GetMarkerColour( UInt32 index ) const;

    /** Whether the last rebuild drew the marker (its layer visible, inside the x range, its series visible and with points) */
    bool IsMarkerShown( UInt32 index ) const;

    /** The rectangle of the shape of a shown marker in the last rebuild, relative to the widget */
    bool GetMarkerRect( UInt32 index, CRect& rect ) const;

    /** The shown marker under a point of the widget (its rectangle widened by MarkerHitSlop), the one whose centre is nearest when several are; -1 when none */
    Int32 HitTestMarker( Int32 localX, Int32 localY ) const;

    /** Selects a shown marker, or none with -1; false for a marker that is not shown */
    bool SelectMarker( Int32 index );

    bool SelectMarkerById( const CString& id );

    /** The selected marker, -1 when none */
    Int32 GetSelectedMarker( void ) const;

    /** Selects the shown marker after (or before) the selected one in the order of x; the first (or last) when none is selected. False when there is none */
    bool SelectNextMarker( bool forward );

    /** Selects the first (or the last) shown marker in the order of x; false when none is shown */
    bool SelectFirstMarker( bool first );

    /** Selects a shown marker and routes MarkerActivatedEvent for it; false for a marker that is not shown */
    bool ActivateMarker( UInt32 index );

    /** The marker under the pointer, -1 when none */
    Int32 GetHoveredMarker( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "ChartView"; }

    static CWidget* Create( void );

    protected:

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The values of the look a rebuild used, to see whether the theme changed them */
    struct SLook
    {
        CColour textColour;
        CColour axisTextColour;
        CColour axisColour;
        CColour gridColour;
        CColour crosshairColour;
        CColour tooltipColour;
        CColour markerColour;
        CColour selectionColour;
        CString font;
        CString title;
        CString seriesColours;
        CString valuePrefix;
        CString valueSuffix;
        CString xAxisType;
        CString legendPosition;
        Int32 decimals;
        bool logScale;
        bool includeZero;
        bool legendVisible;
        bool crosshairVisible;
    };

    /** Totals of one stack group per x */
    struct SStackColumn
    {
        Int64 x;
        Float64 positive;
        Float64 negative;
    };
    typedef GUCEF::vector< SStackColumn > TStackColumnVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;
    typedef GUCEF::vector< CColour > TColourVector;

    bool LookChanged( void ) const;
    void CaptureLook( void );
    UInt32 SeriesRevisionSum( void ) const;

    /** The properties onto the axes and the palette */
    void ApplyProperties( void );

    Int32 LineHeight( void ) const;
    Int32 TextWidth( const CString& text ) const;

    /** The x range of the visible series (with half a bar slot on each side when there are bars) */
    bool GetDataXRange( Float64& minX, Float64& maxX, Float64& barSlot ) const;

    /** The value range of the visible series in the x range, stacked totals included */
    bool GetDataValueRange( Float64 minX, Float64 maxX, Float64& minValue, Float64& maxValue );

    /** The index of the stack group of a bar series and the number of groups */
    UInt32 GetBarGroup( UInt32 seriesIndex, UInt32& groupCount ) const;

    /** Collects the x values of the bar series of a group into m_stack, totals zeroed */
    void PrepareStack( UInt32 group );

    /** Where a point is drawn: for a stacked bar the top of its segment */
    Float64 GetPlotValue( UInt32 seriesIndex, UInt32 pointIndex ) const;

    void DrawSeriesArea( UInt32 seriesIndex, const CColour& colour, Int32 baselineY );
    void DrawSeriesLine( UInt32 seriesIndex, const CColour& colour );
    void DrawBars( Float64 barSlot );
    void DrawLegend( Int32 top, Int32 width );
    void DrawCrosshair( void );
    void DrawMarkers( void );
    void DrawMarkerShape( CChartMarker::TShape shape, Int32 left, Int32 top, Int32 size, const CColour& colour );
    void DrawMarkerTooltip( UInt32 index );

    /** Rebuilds now when the drawing is out of date and the widget has a size, so the geometry of the markers is current */
    void EnsureBuilt( void );

    /** The shown markers in the order of x (then of their index) into m_markerOrder */
    void OrderShownMarkers( void );

    /** Takes the selection and the hover away from markers that are not shown any more */
    void DropHiddenSelection( void );

    void SetHoveredMarker( Int32 index );

    /** Fills m_keep with the indexes of a series to draw: the visible ones, reduced when there are many */
    void CollectDrawnPoints( const CChartSeries& series );

    Int32 LegendHeight( Int32 width ) const;
    UInt32 VisibleSeriesCount( void ) const;

    void UpdateCrosshairPoints( void );

    CChartView( const CChartView& src );
    CChartView& operator=( const CChartView& src );

    private:

    /** The style of a style class of markers */
    struct SMarkerStyle
    {
        CChartMarker::TShape shape;
        CColour colour;
    };
    typedef GUCEF::map< CString, SMarkerStyle > TMarkerStyleMap;
    typedef GUCEF::set< CString > TStringSet;
    typedef GUCEF::vector< CRect > TRectVector;
    typedef GUCEF::vector< UInt32 > TUInt32Vector;

    TSeriesVector m_series;
    CChartAxis m_xAxis;
    CChartAxis m_valueAxis;
    bool m_dirty;
    Int32 m_builtWidth;
    Int32 m_builtHeight;
    UInt32 m_builtSeriesRevision;
    CRect m_plotRect;
    Float64 m_barSlot;
    bool m_hasCrosshair;
    Int64 m_crosshairX;
    TSeriesPointVector m_crosshairPoints;
    SLook m_look;
    TColourVector m_palette;
    CString m_paletteText;
    CChartDecimator::TIndexVector m_keep;
    TStackColumnVector m_stack;
    TInt32Vector m_columnTop;
    TInt32Vector m_columnBottom;
    Int32 m_lineHeight;
    TMarkerVector m_markers;
    TRectVector m_markerRects;              /**< per marker the rectangle of the last rebuild, empty when it was not shown */
    TUInt32Vector m_markerOrder;            /**< the shown markers in the order of x */
    TMarkerStyleMap m_markerStyles;
    TStringSet m_hiddenMarkerLayers;
    Int32 m_selectedMarker;
    Int32 m_hoveredMarker;
    bool m_focused;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTVIEW_H ? */
