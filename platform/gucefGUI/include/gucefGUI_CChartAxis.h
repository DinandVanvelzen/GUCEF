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

#ifndef GUCEF_GUI_CCHARTAXIS_H
#define GUCEF_GUI_CCHARTAXIS_H

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
 *  An axis of a chart (CChartView): its range, its scale, the ticks with their labels and the mapping
 *  between values and the unit interval. A time axis holds milliseconds since the Unix epoch and picks
 *  calendar ticks in UTC (seconds up to centuries); a numeric axis picks ticks at 1, 2, 2.5 or 5 times a
 *  power of 10, or at powers of 10 on a log scale. Compute() chooses the range and the ticks for the data
 *  and the space; the tick vector and its label strings are reused between calls.
 */
class GUCEF_GUI_PUBLIC_CPP CChartAxis
{
    public:

    enum EAxisKind
    {
        AXISKIND_TIME       = 0 ,
        AXISKIND_NUMERIC    = 1
    };

    enum EScale
    {
        SCALE_LINEAR    = 0 ,
        SCALE_LOG       = 1
    };

    enum ETimeUnit
    {
        TIMEUNIT_SECOND = 0 ,
        TIMEUNIT_MINUTE     ,
        TIMEUNIT_HOUR       ,
        TIMEUNIT_DAY        ,
        TIMEUNIT_WEEK       ,
        TIMEUNIT_MONTH      ,
        TIMEUNIT_YEAR
    };

    struct STick
    {
        Float64 value;      /**< position on the axis: a value, or milliseconds since the epoch on a time axis */
        CString label;
    };
    typedef struct STick TTick;
    typedef GUCEF::vector< TTick > TTickVector;

    CChartAxis( void );

    ~CChartAxis();

    void SetKind( EAxisKind kind );

    EAxisKind GetKind( void ) const;

    /** Log applies to a numeric axis only; values at or below 0 are not drawn on a log scale */
    void SetScale( EScale scale );

    EScale GetScale( void ) const;

    /** A fixed range; the auto range follows the data */
    void SetRange( Float64 minValue, Float64 maxValue );

    void SetAutoRange( void );

    bool IsAutoRange( void ) const;

    /** Whether the auto range always includes 0 (on by default; ignored on a log scale and a time axis) */
    void SetIncludeZero( bool includeZero );

    bool GetIncludeZero( void ) const;

    /** Whether an auto range of a numeric axis is extended to whole ticks (on by default; off for an x axis) */
    void SetExtendToTicks( bool extendToTicks );

    bool GetExtendToTicks( void ) const;

    /** Text before value labels, for example "$" */
    void SetValuePrefix( const CString& prefix );

    const CString& GetValuePrefix( void ) const;

    /** Text after value labels, for example "%" */
    void SetValueSuffix( const CString& suffix );

    const CString& GetValueSuffix( void ) const;

    /** Digits after the point of value labels; -1 (default) picks them from the tick step */
    void SetDecimals( Int32 decimals );

    Int32 GetDecimals( void ) const;

    /**
     *  Chooses the range and the ticks for the data range and the space available ('maxTicks' ticks at most, at
     *  least 2). A linear numeric axis extends its auto range to whole ticks (1, 2, 2.5 or 5 times a power of 10),
     *  a log axis to powers of 10; a time axis keeps the data range and picks a calendar step aligned in UTC.
     *  A range of zero width is widened by one unit (one day on a time axis).
     */
    void Compute( Float64 dataMin, Float64 dataMax, UInt32 maxTicks );

    /** The range after Compute() */
    Float64 GetMin( void ) const;

    Float64 GetMax( void ) const;

    const TTickVector& GetTicks( void ) const;

    /** The unit of the time ticks after Compute() */
    ETimeUnit GetTimeUnit( void ) const;

    /** How many units between time ticks */
    UInt32 GetTimeStep( void ) const;

    /** The distance between numeric ticks after Compute() (0 for log and time axes) */
    Float64 GetTickStep( void ) const;

    /** The decimals the value labels got after Compute() */
    Int32 GetLabelDecimals( void ) const;

    /** Position in the range: 0 at the minimum, 1 at the maximum (log aware); NaN for a value a log scale cannot show */
    Float64 ToUnit( Float64 value ) const;

    Float64 FromUnit( Float64 unit ) const;

    /** A value as the labels and the tooltip show it: sign, prefix, thousands separators, decimals, suffix: "-$1,234.50" */
    CString FormatValue( Float64 value, Int32 decimals ) const;

    /** The label of a tick at 'value' as Compute() would make it for the current ticks */
    CString FormatTickLabel( Float64 value ) const;

    /** A UTC time as a tooltip shows it: "2024-03-05", with " 14:30" (or " 14:30:15") when it is not at midnight */
    static CString FormatTime( Int64 millisecondsSinceEpoch );

    /** 1, 2, 2.5 or 5 times a power of 10, the nearest at or above 'roughStep' (which must be above 0) */
    static Float64 NiceStep( Float64 roughStep );

    /** Splits a time into its UTC calendar parts */
    static void SplitTime( Int64 millisecondsSinceEpoch, Int32& year, Int32& month, Int32& day,
                           Int32& hour, Int32& minute, Int32& second, Int32& millisecond );

    /** Milliseconds since the epoch of 00:00 UTC at the given date */
    static Int64 MakeTime( Int32 year, Int32 month, Int32 day );

    private:

    void ComputeLinear( Float64 low, Float64 high, UInt32 maxTicks );
    void ComputeLog( Float64 low, Float64 high, UInt32 maxTicks );
    void ComputeTime( Float64 low, Float64 high, UInt32 maxTicks );

    /** Sets tick 'index' (growing the vector when needed); the label is only replaced when it differs */
    void SetTick( UInt32 index, Float64 value, const char* label );

    /** Writes the value label into the buffer, the same text as FormatValue() */
    void FormatValueInto( char* buffer, UInt32 bufferSize, Float64 value, Int32 decimals ) const;

    void FormatTimeTickInto( char* buffer, UInt32 bufferSize, Int64 millisecondsSinceEpoch ) const;

    private:

    EAxisKind m_kind;
    EScale m_scale;
    bool m_autoRange;
    Float64 m_fixedMin;
    Float64 m_fixedMax;
    bool m_includeZero;
    bool m_extendToTicks;
    CString m_prefix;
    CString m_suffix;
    Int32 m_decimals;

    Float64 m_min;
    Float64 m_max;
    TTickVector m_ticks;
    UInt32 m_tickCount;
    ETimeUnit m_timeUnit;
    UInt32 m_timeStep;
    Float64 m_tickStep;
    Int32 m_labelDecimals;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTAXIS_H ? */
