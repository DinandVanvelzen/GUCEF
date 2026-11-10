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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits>

#ifndef GUCEF_CORE_CDATE_H
#include "gucefCORE_CDate.h"
#define GUCEF_CORE_CDATE_H
#endif /* GUCEF_CORE_CDATE_H ? */

#include "gucefGUI_CChartAxis.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline bool IsNaN( Float64 value ) { return value != value; }

inline bool IsFinite( Float64 value ) { return value == value && ( value - value ) == 0.0; }

const Int64 MsPerSecond = 1000;
const Int64 MsPerMinute = 60 * MsPerSecond;
const Int64 MsPerHour = 60 * MsPerMinute;
const Int64 MsPerDay = 24 * MsPerHour;
const Int64 MsPerWeek = 7 * MsPerDay;

/** 1970-01-05 was the first Monday after the epoch */
const Int64 FirstMondayMs = 4 * MsPerDay;

const char* const MonthNames[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/** The steps a time axis chooses from, in ascending order */
struct STimeStep
{
    CChartAxis::ETimeUnit unit;
    UInt32 count;
    Int64 approximateMs;
};

const STimeStep TimeSteps[] =
{
    { CChartAxis::TIMEUNIT_SECOND, 1, MsPerSecond },
    { CChartAxis::TIMEUNIT_SECOND, 5, 5 * MsPerSecond },
    { CChartAxis::TIMEUNIT_SECOND, 15, 15 * MsPerSecond },
    { CChartAxis::TIMEUNIT_SECOND, 30, 30 * MsPerSecond },
    { CChartAxis::TIMEUNIT_MINUTE, 1, MsPerMinute },
    { CChartAxis::TIMEUNIT_MINUTE, 5, 5 * MsPerMinute },
    { CChartAxis::TIMEUNIT_MINUTE, 15, 15 * MsPerMinute },
    { CChartAxis::TIMEUNIT_MINUTE, 30, 30 * MsPerMinute },
    { CChartAxis::TIMEUNIT_HOUR, 1, MsPerHour },
    { CChartAxis::TIMEUNIT_HOUR, 3, 3 * MsPerHour },
    { CChartAxis::TIMEUNIT_HOUR, 6, 6 * MsPerHour },
    { CChartAxis::TIMEUNIT_HOUR, 12, 12 * MsPerHour },
    { CChartAxis::TIMEUNIT_DAY, 1, MsPerDay },
    { CChartAxis::TIMEUNIT_DAY, 2, 2 * MsPerDay },
    { CChartAxis::TIMEUNIT_WEEK, 1, MsPerWeek },
    { CChartAxis::TIMEUNIT_MONTH, 1, 2629746000LL },        /* the mean Gregorian month */
    { CChartAxis::TIMEUNIT_MONTH, 3, 3 * 2629746000LL },
    { CChartAxis::TIMEUNIT_MONTH, 6, 6 * 2629746000LL },
    { CChartAxis::TIMEUNIT_YEAR, 1, 31556952000LL },        /* the mean Gregorian year */
    { CChartAxis::TIMEUNIT_YEAR, 2, 2 * 31556952000LL },
    { CChartAxis::TIMEUNIT_YEAR, 5, 5 * 31556952000LL },
    { CChartAxis::TIMEUNIT_YEAR, 10, 10 * 31556952000LL },
    { CChartAxis::TIMEUNIT_YEAR, 20, 20 * 31556952000LL },
    { CChartAxis::TIMEUNIT_YEAR, 50, 50 * 31556952000LL },
    { CChartAxis::TIMEUNIT_YEAR, 100, 100 * 31556952000LL }
};
const UInt32 TimeStepCount = sizeof( TimeSteps ) / sizeof( TimeSteps[ 0 ] );

/** Division rounding towards minus infinity */
inline Int64
FloorDiv( Int64 a, Int64 b )
{
    Int64 q = a / b;
    if ( ( a % b != 0 ) && ( ( a < 0 ) != ( b < 0 ) ) )
        --q;
    return q;
}

/** The smallest multiple of 'step' (counted from 'origin') at or above 'value' */
inline Int64
CeilToStep( Int64 value, Int64 origin, Int64 step )
{
    Int64 k = FloorDiv( value - origin, step );
    Int64 t = origin + k * step;
    if ( t < value )
        t += step;
    return t;
}

/** Digits after the point needed to write multiples of 'step' exactly (0 .. 10) */
Int32
DecimalsForStep( Float64 step )
{
    if ( !( step > 0.0 ) || !IsFinite( step ) )
        return 0;
    Int32 decimals = 0;
    Float64 scaled = step;
    while ( decimals < 10 && fabs( scaled - floor( scaled + 0.5 ) ) > 1e-6 * ( scaled > 1.0 ? scaled : 1.0 ) )
    {
        scaled *= 10.0;
        ++decimals;
    }
    return decimals;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CChartAxis::CChartAxis( void )
    : m_kind( AXISKIND_NUMERIC )
    , m_scale( SCALE_LINEAR )
    , m_autoRange( true )
    , m_fixedMin( 0.0 )
    , m_fixedMax( 1.0 )
    , m_includeZero( true )
    , m_extendToTicks( true )
    , m_prefix()
    , m_suffix()
    , m_decimals( -1 )
    , m_min( 0.0 )
    , m_max( 1.0 )
    , m_ticks()
    , m_tickCount( 0 )
    , m_timeUnit( TIMEUNIT_DAY )
    , m_timeStep( 1 )
    , m_tickStep( 0.0 )
    , m_labelDecimals( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CChartAxis::~CChartAxis()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetKind( EAxisKind kind )
{GUCEF_TRACE;

    m_kind = kind;
}

/*-------------------------------------------------------------------------*/

CChartAxis::EAxisKind
CChartAxis::GetKind( void ) const
{GUCEF_TRACE;

    return m_kind;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetScale( EScale scale )
{GUCEF_TRACE;

    m_scale = scale;
}

/*-------------------------------------------------------------------------*/

CChartAxis::EScale
CChartAxis::GetScale( void ) const
{GUCEF_TRACE;

    return m_scale;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetRange( Float64 minValue, Float64 maxValue )
{GUCEF_TRACE;

    if ( maxValue < minValue )
    {
        Float64 swap = minValue;
        minValue = maxValue;
        maxValue = swap;
    }
    m_fixedMin = minValue;
    m_fixedMax = maxValue;
    m_autoRange = false;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetAutoRange( void )
{GUCEF_TRACE;

    m_autoRange = true;
}

/*-------------------------------------------------------------------------*/

bool
CChartAxis::IsAutoRange( void ) const
{GUCEF_TRACE;

    return m_autoRange;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetIncludeZero( bool includeZero )
{GUCEF_TRACE;

    m_includeZero = includeZero;
}

/*-------------------------------------------------------------------------*/

bool
CChartAxis::GetIncludeZero( void ) const
{GUCEF_TRACE;

    return m_includeZero;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetExtendToTicks( bool extendToTicks )
{GUCEF_TRACE;

    m_extendToTicks = extendToTicks;
}

/*-------------------------------------------------------------------------*/

bool
CChartAxis::GetExtendToTicks( void ) const
{GUCEF_TRACE;

    return m_extendToTicks;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetValuePrefix( const CString& prefix )
{GUCEF_TRACE;

    if ( m_prefix != prefix )
        m_prefix = prefix;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartAxis::GetValuePrefix( void ) const
{GUCEF_TRACE;

    return m_prefix;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetValueSuffix( const CString& suffix )
{GUCEF_TRACE;

    if ( m_suffix != suffix )
        m_suffix = suffix;
}

/*-------------------------------------------------------------------------*/

const CString&
CChartAxis::GetValueSuffix( void ) const
{GUCEF_TRACE;

    return m_suffix;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetDecimals( Int32 decimals )
{GUCEF_TRACE;

    m_decimals = decimals < -1 ? -1 : ( decimals > 10 ? 10 : decimals );
}

/*-------------------------------------------------------------------------*/

Int32
CChartAxis::GetDecimals( void ) const
{GUCEF_TRACE;

    return m_decimals;
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::GetMin( void ) const
{GUCEF_TRACE;

    return m_min;
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::GetMax( void ) const
{GUCEF_TRACE;

    return m_max;
}

/*-------------------------------------------------------------------------*/

const CChartAxis::TTickVector&
CChartAxis::GetTicks( void ) const
{GUCEF_TRACE;

    return m_ticks;
}

/*-------------------------------------------------------------------------*/

CChartAxis::ETimeUnit
CChartAxis::GetTimeUnit( void ) const
{GUCEF_TRACE;

    return m_timeUnit;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartAxis::GetTimeStep( void ) const
{GUCEF_TRACE;

    return m_timeStep;
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::GetTickStep( void ) const
{GUCEF_TRACE;

    return m_tickStep;
}

/*-------------------------------------------------------------------------*/

Int32
CChartAxis::GetLabelDecimals( void ) const
{GUCEF_TRACE;

    return m_labelDecimals;
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::NiceStep( Float64 roughStep )
{GUCEF_TRACE;

    if ( !( roughStep > 0.0 ) || !IsFinite( roughStep ) )
        return 1.0;
    Float64 power = pow( 10.0, floor( log10( roughStep ) ) );
    Float64 fraction = roughStep / power;
    // tolerate the rounding of log10 / pow: a rough step of exactly 2 must give 2
    const Float64 epsilon = 1e-9;
    Float64 nice;
    if ( fraction <= 1.0 + epsilon )
        nice = 1.0;
    else if ( fraction <= 2.0 + epsilon )
        nice = 2.0;
    else if ( fraction <= 2.5 + epsilon )
        nice = 2.5;
    else if ( fraction <= 5.0 + epsilon )
        nice = 5.0;
    else
        nice = 10.0;
    return nice * power;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SetTick( UInt32 index, Float64 value, const char* label )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_ticks.size() )
        m_ticks.resize( index + 1 );
    TTick& tick = m_ticks[ index ];
    tick.value = value;
    // the same label as last time costs no allocation
    if ( tick.label != label )
        tick.label = label;
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::Compute( Float64 dataMin, Float64 dataMax, UInt32 maxTicks )
{GUCEF_TRACE;

    if ( maxTicks < 2 )
        maxTicks = 2;

    Float64 low = m_autoRange ? dataMin : m_fixedMin;
    Float64 high = m_autoRange ? dataMax : m_fixedMax;
    if ( !IsFinite( low ) || !IsFinite( high ) )
    {
        low = 0.0;
        high = 1.0;
    }
    if ( high < low )
    {
        Float64 swap = low;
        low = high;
        high = swap;
    }

    m_tickCount = 0;
    m_tickStep = 0.0;
    if ( AXISKIND_TIME == m_kind )
        ComputeTime( low, high, maxTicks );
    else
    if ( SCALE_LOG == m_scale )
        ComputeLog( low, high, maxTicks );
    else
        ComputeLinear( low, high, maxTicks );

    // the vector keeps the strings of the ticks it still needs
    if ( m_tickCount < (UInt32) m_ticks.size() )
        m_ticks.resize( m_tickCount );
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::ComputeLinear( Float64 low, Float64 high, UInt32 maxTicks )
{GUCEF_TRACE;

    if ( m_autoRange && m_includeZero )
    {
        if ( low > 0.0 )
            low = 0.0;
        if ( high < 0.0 )
            high = 0.0;
    }
    if ( high == low )
    {
        if ( 0.0 == low )
            high = 1.0;
        else
        {
            low -= 1.0;
            high += 1.0;
        }
    }

    Float64 step = NiceStep( ( high - low ) / (Float64) ( maxTicks - 1 ) );
    if ( m_autoRange && m_extendToTicks )
    {
        // extending to whole steps can add ticks: take the next nice step until they fit
        for ( UInt32 attempt=0; attempt<8; ++attempt )
        {
            Float64 extendedLow = floor( low / step + 1e-9 ) * step;
            Float64 extendedHigh = ceil( high / step - 1e-9 ) * step;
            Float64 count = floor( ( extendedHigh - extendedLow ) / step + 0.5 ) + 1.0;
            if ( count <= (Float64) maxTicks || 7 == attempt )
            {
                low = extendedLow;
                high = extendedHigh;
                break;
            }
            step = NiceStep( step * 1.0001 );
        }
    }
    m_min = low;
    m_max = high;
    m_tickStep = step;
    m_labelDecimals = m_decimals >= 0 ? m_decimals : DecimalsForStep( step );

    char label[ 128 ];
    Float64 first = ceil( low / step - 1e-9 ) * step;
    for ( UInt32 i=0; i<1000; ++i )
    {
        Float64 value = first + (Float64) i * step;
        if ( value > high + step * 1e-9 )
            break;
        if ( fabs( value ) < step * 1e-9 )
            value = 0.0;
        FormatValueInto( label, sizeof( label ), value, m_labelDecimals );
        SetTick( m_tickCount++, value, label );
    }
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::ComputeLog( Float64 low, Float64 high, UInt32 maxTicks )
{GUCEF_TRACE;

    // a log scale shows positive values only
    if ( !( high > 0.0 ) )
        high = 1.0;
    if ( !( low > 0.0 ) )
        low = high / 1000.0;
    Float64 lowExponent = floor( log10( low ) + 1e-9 );
    Float64 highExponent = ceil( log10( high ) - 1e-9 );
    if ( m_autoRange )
    {
        if ( highExponent <= lowExponent )
            highExponent = lowExponent + 1.0;
        low = pow( 10.0, lowExponent );
        high = pow( 10.0, highExponent );
    }
    else
    if ( high <= low )
    {
        high = low * 10.0;
    }
    m_min = low;
    m_max = high;
    m_labelDecimals = m_decimals;

    Int32 firstDecade = (Int32) floor( log10( low ) + 1e-9 );
    Int32 lastDecade = (Int32) ceil( log10( high ) - 1e-9 );
    Int32 decades = lastDecade - firstDecade;
    // every decade while they fit, else every second, fifth ... one
    Int32 decadeStep = 1;
    while ( decades / decadeStep + 1 > (Int32) maxTicks )
        ++decadeStep;
    bool intermediate = decades <= 3 && ( decades * 3 + 1 ) <= (Int32) maxTicks;

    char label[ 128 ];
    for ( Int32 decade=firstDecade; decade<=lastDecade; decade+=decadeStep )
    {
        Float64 power = pow( 10.0, (Float64) decade );
        for ( Int32 m=0; m<3; ++m )
        {
            if ( m > 0 && !intermediate )
                break;
            Float64 value = power * ( 0 == m ? 1.0 : ( 1 == m ? 2.0 : 5.0 ) );
            if ( value < low * ( 1.0 - 1e-9 ) || value > high * ( 1.0 + 1e-9 ) )
                continue;
            Int32 decimals = m_decimals >= 0 ? m_decimals : ( decade < 0 ? -decade : 0 );
            FormatValueInto( label, sizeof( label ), value, decimals );
            SetTick( m_tickCount++, value, label );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::ComputeTime( Float64 low, Float64 high, UInt32 maxTicks )
{GUCEF_TRACE;

    if ( high == low )
    {
        low -= (Float64) ( MsPerDay / 2 );
        high += (Float64) ( MsPerDay / 2 );
    }
    m_min = low;
    m_max = high;
    m_labelDecimals = 0;

    const Int64 lowMs = (Int64) ceil( low );
    const Int64 highMs = (Int64) floor( high );
    const Float64 span = high - low;

    // the finest step whose ticks fit
    UInt32 choice = TimeStepCount - 1;
    for ( UInt32 i=0; i<TimeStepCount; ++i )
    {
        if ( floor( span / (Float64) TimeSteps[ i ].approximateMs ) + 1.0 <= (Float64) maxTicks )
        {
            choice = i;
            break;
        }
    }
    m_timeUnit = TimeSteps[ choice ].unit;
    m_timeStep = TimeSteps[ choice ].count;
    if ( TIMEUNIT_YEAR == m_timeUnit )
    {
        // beyond centuries: more of the largest step
        while ( floor( span / ( (Float64) m_timeStep * 31556952000.0 ) ) + 1.0 > (Float64) maxTicks && m_timeStep < 100000 )
            m_timeStep *= 10;
    }

    char label[ 64 ];
    switch ( m_timeUnit )
    {
        case TIMEUNIT_SECOND:
        case TIMEUNIT_MINUTE:
        case TIMEUNIT_HOUR:
        case TIMEUNIT_DAY:
        case TIMEUNIT_WEEK:
        {
            Int64 unitMs = MsPerSecond;
            Int64 origin = 0;
            if ( TIMEUNIT_MINUTE == m_timeUnit )
                unitMs = MsPerMinute;
            else if ( TIMEUNIT_HOUR == m_timeUnit )
                unitMs = MsPerHour;
            else if ( TIMEUNIT_DAY == m_timeUnit )
                unitMs = MsPerDay;
            else if ( TIMEUNIT_WEEK == m_timeUnit )
            {
                unitMs = MsPerDay;
                origin = FirstMondayMs;
            }
            Int64 stepMs = unitMs * (Int64) m_timeStep * ( TIMEUNIT_WEEK == m_timeUnit ? 7 : 1 );
            for ( Int64 t=CeilToStep( lowMs, origin, stepMs ); t<=highMs && m_tickCount<1000; t+=stepMs )
            {
                FormatTimeTickInto( label, sizeof( label ), t );
                SetTick( m_tickCount++, (Float64) t, label );
            }
            break;
        }
        case TIMEUNIT_MONTH:
        case TIMEUNIT_YEAR:
        {
            Int32 year, month, day, hour, minute, second, millisecond;
            SplitTime( lowMs, year, month, day, hour, minute, second, millisecond );
            const bool atMonthStart = 1 == day && 0 == hour && 0 == minute && 0 == second && 0 == millisecond;
            if ( TIMEUNIT_MONTH == m_timeUnit )
            {
                // months counted from year 0, so a step of 3 starts quarters and 6 halves
                Int64 index = (Int64) year * 12 + ( month - 1 );
                if ( !atMonthStart )
                    ++index;
                while ( index % (Int64) m_timeStep != 0 )
                    ++index;
                for ( ; m_tickCount<1000; index+=m_timeStep )
                {
                    Int64 t = MakeTime( (Int32) ( index / 12 ), (Int32) ( index % 12 ) + 1, 1 );
                    if ( t > highMs )
                        break;
                    FormatTimeTickInto( label, sizeof( label ), t );
                    SetTick( m_tickCount++, (Float64) t, label );
                }
            }
            else
            {
                Int64 y = year;
                if ( !( atMonthStart && 1 == month ) )
                    ++y;
                while ( y % (Int64) m_timeStep != 0 )
                    ++y;
                for ( ; m_tickCount<1000 && y<=32767; y+=m_timeStep )
                {
                    Int64 t = MakeTime( (Int32) y, 1, 1 );
                    if ( t > highMs )
                        break;
                    FormatTimeTickInto( label, sizeof( label ), t );
                    SetTick( m_tickCount++, (Float64) t, label );
                }
            }
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::ToUnit( Float64 value ) const
{GUCEF_TRACE;

    if ( AXISKIND_NUMERIC == m_kind && SCALE_LOG == m_scale )
    {
        if ( !( value > 0.0 ) || !( m_min > 0.0 ) || !( m_max > m_min ) )
            return std::numeric_limits< Float64 >::quiet_NaN();
        return ( log10( value ) - log10( m_min ) ) / ( log10( m_max ) - log10( m_min ) );
    }
    if ( m_max == m_min )
        return 0.5;
    return ( value - m_min ) / ( m_max - m_min );
}

/*-------------------------------------------------------------------------*/

Float64
CChartAxis::FromUnit( Float64 unit ) const
{GUCEF_TRACE;

    if ( AXISKIND_NUMERIC == m_kind && SCALE_LOG == m_scale && m_min > 0.0 && m_max > m_min )
        return pow( 10.0, log10( m_min ) + unit * ( log10( m_max ) - log10( m_min ) ) );
    return m_min + unit * ( m_max - m_min );
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::FormatValueInto( char* buffer, UInt32 bufferSize, Float64 value, Int32 decimals ) const
{GUCEF_TRACE;

    if ( decimals < 0 )
        decimals = 0;
    if ( decimals > 10 )
        decimals = 10;
    if ( IsNaN( value ) )
    {
        snprintf( buffer, bufferSize, "-" );
        return;
    }

    char digits[ 96 ];
    snprintf( digits, sizeof( digits ), "%.*f", (int) decimals, fabs( value ) );
    // a value that rounds to zero is written without a sign
    bool negative = value < 0.0;
    if ( negative )
    {
        bool allZero = true;
        for ( const char* c=digits; '\0' != *c; ++c )
            if ( *c >= '1' && *c <= '9' )
                allZero = false;
        if ( allZero )
            negative = false;
    }

    // thousands separators in the integer part
    const char* point = strchr( digits, '.' );
    size_t integerLength = GUCEF_NULL != point ? (size_t) ( point - digits ) : strlen( digits );
    char grouped[ 128 ];
    size_t out = 0;
    for ( size_t i=0; i<integerLength && out<sizeof( grouped )-2; ++i )
    {
        if ( i > 0 && 0 == ( integerLength - i ) % 3 )
            grouped[ out++ ] = ',';
        grouped[ out++ ] = digits[ i ];
    }
    if ( GUCEF_NULL != point )
    {
        for ( const char* c=point; '\0' != *c && out<sizeof( grouped )-1; ++c )
            grouped[ out++ ] = *c;
    }
    grouped[ out ] = '\0';

    snprintf( buffer, bufferSize, "%s%s%s%s", negative ? "-" : "", m_prefix.C_String(), grouped, m_suffix.C_String() );
}

/*-------------------------------------------------------------------------*/

CString
CChartAxis::FormatValue( Float64 value, Int32 decimals ) const
{GUCEF_TRACE;

    char buffer[ 192 ];
    FormatValueInto( buffer, sizeof( buffer ), value, decimals );
    return CString( buffer );
}

/*-------------------------------------------------------------------------*/

CString
CChartAxis::FormatTickLabel( Float64 value ) const
{GUCEF_TRACE;

    char buffer[ 192 ];
    if ( AXISKIND_TIME == m_kind )
        FormatTimeTickInto( buffer, sizeof( buffer ), (Int64) value );
    else
        FormatValueInto( buffer, sizeof( buffer ), value, m_labelDecimals );
    return CString( buffer );
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::FormatTimeTickInto( char* buffer, UInt32 bufferSize, Int64 millisecondsSinceEpoch ) const
{GUCEF_TRACE;

    Int32 year, month, day, hour, minute, second, millisecond;
    SplitTime( millisecondsSinceEpoch, year, month, day, hour, minute, second, millisecond );
    const char* monthName = MonthNames[ ( month - 1 ) % 12 ];
    switch ( m_timeUnit )
    {
        case TIMEUNIT_YEAR:
            snprintf( buffer, bufferSize, "%d", (int) year );
            break;
        case TIMEUNIT_MONTH:
            snprintf( buffer, bufferSize, "%s %d", monthName, (int) year );
            break;
        case TIMEUNIT_WEEK:
        case TIMEUNIT_DAY:
            snprintf( buffer, bufferSize, "%s %d", monthName, (int) day );
            break;
        case TIMEUNIT_HOUR:
        case TIMEUNIT_MINUTE:
            if ( 0 == hour && 0 == minute )
                snprintf( buffer, bufferSize, "%s %d", monthName, (int) day );
            else
                snprintf( buffer, bufferSize, "%02d:%02d", (int) hour, (int) minute );
            break;
        case TIMEUNIT_SECOND:
        default:
            snprintf( buffer, bufferSize, "%02d:%02d:%02d", (int) hour, (int) minute, (int) second );
            break;
    }
}

/*-------------------------------------------------------------------------*/

CString
CChartAxis::FormatTime( Int64 millisecondsSinceEpoch )
{GUCEF_TRACE;

    Int32 year, month, day, hour, minute, second, millisecond;
    SplitTime( millisecondsSinceEpoch, year, month, day, hour, minute, second, millisecond );
    char buffer[ 64 ];
    if ( 0 != second )
        snprintf( buffer, sizeof( buffer ), "%04d-%02d-%02d %02d:%02d:%02d", (int) year, (int) month, (int) day, (int) hour, (int) minute, (int) second );
    else
    if ( 0 != hour || 0 != minute )
        snprintf( buffer, sizeof( buffer ), "%04d-%02d-%02d %02d:%02d", (int) year, (int) month, (int) day, (int) hour, (int) minute );
    else
        snprintf( buffer, sizeof( buffer ), "%04d-%02d-%02d", (int) year, (int) month, (int) day );
    return CString( buffer );
}

/*-------------------------------------------------------------------------*/

void
CChartAxis::SplitTime( Int64 millisecondsSinceEpoch, Int32& year, Int32& month, Int32& day,
                       Int32& hour, Int32& minute, Int32& second, Int32& millisecond )
{GUCEF_TRACE;

    Int64 days = FloorDiv( millisecondsSinceEpoch, MsPerDay );
    Int64 rest = millisecondsSinceEpoch - days * MsPerDay;
    CORE::CDate date;
    if ( !date.FromDaysSinceUnixEpoch( (Int32) days ) )
        date.FromDaysSinceUnixEpoch( 0 );
    year = date.GetYear();
    month = date.GetMonth();
    day = date.GetDay();
    hour = (Int32) ( rest / MsPerHour );
    minute = (Int32) ( ( rest % MsPerHour ) / MsPerMinute );
    second = (Int32) ( ( rest % MsPerMinute ) / MsPerSecond );
    millisecond = (Int32) ( rest % MsPerSecond );
}

/*-------------------------------------------------------------------------*/

Int64
CChartAxis::MakeTime( Int32 year, Int32 month, Int32 day )
{GUCEF_TRACE;

    CORE::CDate date( (UInt16) year, (UInt8) month, (UInt8) day );
    return (Int64) date.ToDaysSinceUnixEpoch() * MsPerDay;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
