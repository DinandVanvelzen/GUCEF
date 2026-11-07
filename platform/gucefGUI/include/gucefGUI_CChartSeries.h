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

#ifndef GUCEF_GUI_CCHARTSERIES_H
#define GUCEF_GUI_CCHARTSERIES_H

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
 *  One series of a chart (CChartView): its name, how it is drawn, its colour, its stack group and its
 *  points as two parallel arrays, x (Int64: milliseconds since the Unix epoch in UTC on a time axis, plain
 *  integers on a numeric axis) and values (Float64). The x values are kept ascending; a NaN value is a gap.
 *  The data is copied in, so the chart does not depend on the lifetime of the caller's buffers; replacing
 *  the data reuses the storage of the previous data.
 */
class GUCEF_GUI_PUBLIC_CPP CChartSeries
{
    public:

    enum ESeriesType
    {
        SERIESTYPE_LINE = 0 ,   /**< a polyline through the points; NaN values break the line */
        SERIESTYPE_AREA = 1 ,   /**< the line with the area between it and the baseline filled */
        SERIESTYPE_BAR  = 2     /**< one bar per point; bars of the same stack group are stacked */
    };
    typedef enum ESeriesType TSeriesType;

    typedef GUCEF::vector< Int64 > TInt64Vector;
    typedef GUCEF::vector< Float64 > TFloat64Vector;

    CChartSeries( void );

    CChartSeries( const CString& name, TSeriesType type );

    CChartSeries( const CChartSeries& src );

    CChartSeries& operator=( const CChartSeries& src );

    ~CChartSeries();

    void SetName( const CString& name );

    const CString& GetName( void ) const;

    void SetType( TSeriesType type );

    TSeriesType GetType( void ) const;

    /** An explicit colour; without one the series takes the palette colour of its index */
    void SetColour( const CColour& colour );

    void ResetColour( void );

    bool HasColour( void ) const;

    const CColour& GetColour( void ) const;

    /** Bar series with the same non-empty stack group are stacked; empty: the series is a group of its own */
    void SetStackGroup( const CString& stackGroup );

    const CString& GetStackGroup( void ) const;

    void SetVisible( bool visible );

    bool IsVisible( void ) const;

    /** Width of the line of line and area series in logical pixels, default 2, at least 1 */
    void SetLineWidth( Int32 width );

    Int32 GetLineWidth( void ) const;

    /**
     *  Copies the points. The x values must be ascending; when they are not, the points are sorted
     *  by x (stable). False for a NULL array with a count above 0.
     */
    bool SetData( const Int64* x, const Float64* values, UInt32 count );

    /** The same for unsigned x values; false (and nothing changed) when an x is above the Int64 range */
    bool SetData( const UInt64* x, const Float64* values, UInt32 count );

    /** Adds a point at the end; false when x is below the last x */
    bool Append( Int64 x, Float64 value );

    void Reserve( UInt32 count );

    void Clear( void );

    UInt32 GetCount( void ) const;

    Int64 GetX( UInt32 index ) const;

    Float64 GetValue( UInt32 index ) const;

    /** NULL when empty */
    const Int64* GetXData( void ) const;

    /** NULL when empty */
    const Float64* GetValueData( void ) const;

    /** The index of the point with the x nearest to 'x' (the lower one on a tie), GetCount() when empty */
    UInt32 FindNearestIndex( Int64 x ) const;

    /** The same, skipping points whose value is NaN; GetCount() when no point has a value */
    UInt32 FindNearestValidIndex( Int64 x ) const;

    /** The first index whose x is at or above 'x' (GetCount() when none is) */
    UInt32 LowerBound( Int64 x ) const;

    /** The first index whose x is above 'x' (GetCount() when none is) */
    UInt32 UpperBound( Int64 x ) const;

    /** The smallest and largest x; false when empty */
    bool GetXRange( Int64& minX, Int64& maxX ) const;

    /** The smallest and largest finite value; false when there is none */
    bool GetValueRange( Float64& minValue, Float64& maxValue ) const;

    /** Goes up with every change of the data or the look */
    UInt32 GetRevision( void ) const;

    private:

    /** Sorts the points by x, equal x values keep their order */
    void SortByX( void );

    void Changed( void );

    private:

    CString m_name;
    TSeriesType m_type;
    CColour m_colour;
    bool m_hasColour;
    CString m_stackGroup;
    bool m_visible;
    Int32 m_lineWidth;
    TInt64Vector m_x;
    TFloat64Vector m_values;
    UInt32 m_revision;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTSERIES_H ? */
