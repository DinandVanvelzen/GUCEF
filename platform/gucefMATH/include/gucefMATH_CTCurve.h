/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTCURVE_H
#define GUCEF_MATH_CTCURVE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <cmath>

#ifndef GUCEF_MATH_CTHERMITESPLINE_H
#include "gucefMATH_CTHermiteSpline.h"
#define GUCEF_MATH_CTHERMITESPLINE_H
#endif /* GUCEF_MATH_CTHERMITESPLINE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      ENUMERATIONS                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the tangents of a key of a CTCurve are determined */
enum ECurveTangentMode
{
    CURVETANGENT_FREE = 0,      /**< the stored in and out tangents are used */
    CURVETANGENT_LINEAR,        /**< in: slope from the previous key, out: slope to the next key (0 without neighbour); a segment between two linear keys is a straight line */
    CURVETANGENT_CONSTANT,      /**< the value of this key is held until the next key (a step); the in tangent is flat */
    CURVETANGENT_AUTO,          /**< Catmull-Rom: slope between the neighbouring keys, slope to the only neighbour at the ends */
    CURVETANGENT_CLAMPED_AUTO   /**< as AUTO, flat at local extrema and at the ends and limited to three times the smaller neighbouring secant slope (Fritsch-Carlson), so every segment is monotone and the curve never overshoots its keys */
};

typedef enum ECurveTangentMode TCurveTangentMode;

/** What a CTCurve does before its first and after its last key */
enum ECurveWrapMode
{
    CURVEWRAP_CLAMP = 0,        /**< hold the first or last value */
    CURVEWRAP_LOOP,             /**< repeat the curve */
    CURVEWRAP_PINGPONG,         /**< repeat the curve alternately forwards and backwards */
    CURVEWRAP_OFFSET_LOOP       /**< repeat the curve and add the value change of one cycle for every cycle passed */
};

typedef enum ECurveWrapMode TCurveWrapMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A keyframed one dimensional curve with the semantics of an animation curve: keys of {time, value, in tangent,
 *  out tangent, tangent mode} sorted by time, cubic Hermite interpolation between neighbouring keys, and wrap modes
 *  before the first and after the last key. Tangents are slopes (value per time).
 *
 *  - T: Float32 or Float64.
 *  - An empty curve evaluates to 0; a curve of one key to the value of that key (with the offset of OFFSET_LOOP,
 *    which is zero for one key).
 *  - Two keys at the same time are not allowed: AddKey() at an existing time replaces that key.
 *  - Evaluate( time ) searches the segment by bisection; Evaluate( time, hint ) first tries the segment in 'hint'
 *    (which the call updates) so that evaluation at increasing times is constant time. The hint is owned by the
 *    caller, so concurrent const use of one curve is safe.
 *  - Queries are realtime-safe; AddKey, RemoveKey and the key setters allocate or change keys (not realtime-safe).
 *  - DataNode: attributes 'preWrap' and 'postWrap', a child node 'key' per key with the attributes time, value, in,
 *    out and mode.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTCurve
{
    public:

    typedef T TScalar;

    struct SKey
    {
        T time;
        T value;
        T inTangent;
        T outTangent;
        TCurveTangentMode mode;

        SKey( void ) : time( 0 ), value( 0 ), inTangent( 0 ), outTangent( 0 ), mode( CURVETANGENT_AUTO ) {}
    };

    CTCurve( void );

    /** The value AddKey() returns when it rejects a key */
    enum { INVALID_INDEX = 0xFFFFFFFF };

    /** Adds a key with an automatic tangent (or replaces the key at the same time); returns its index, or INVALID_INDEX (nothing changed) when time or value is not finite */
    UInt32 AddKey( const T& time, const T& value );

    /** Adds a key with free tangents (or replaces the key at the same time); returns its index, or INVALID_INDEX (nothing changed) when time, value or a tangent is not finite */
    UInt32 AddKey( const T& time, const T& value, const T& inTangent, const T& outTangent );

    /** Removes key 'index'; false when the index is out of range */
    bool RemoveKey( UInt32 index );

    /** Sets the tangent mode of key 'index'; false when the index is out of range */
    bool SetTangentMode( UInt32 index, TCurveTangentMode mode );

    /** Sets the stored tangents of key 'index' and its mode to FREE; false when the index is out of range */
    bool SetTangents( UInt32 index, const T& inTangent, const T& outTangent );

    void Clear( void );

    void SetPreWrapMode( TCurveWrapMode mode );
    void SetPostWrapMode( TCurveWrapMode mode );
    GUCEF_MATH_FORCE_INLINE TCurveWrapMode GetPreWrapMode( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_preWrap; }
    GUCEF_MATH_FORCE_INLINE TCurveWrapMode GetPostWrapMode( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_postWrap; }

    GUCEF_MATH_FORCE_INLINE UInt32 GetKeyCount( void ) const GUCEF_MATH_REALTIME_CONTEXT { return (UInt32) m_keys.size(); }

    /** Key 'index' as stored; the default key when out of range */
    SKey GetKey( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The tangents of key 'index' as used for evaluation, with the tangent mode applied; false when out of range */
    bool GetEffectiveTangents( UInt32 index, T& inTangent, T& outTangent ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Time of the first and last key (0 for an empty curve) */
    T GetStartTime( void ) const GUCEF_MATH_REALTIME_CONTEXT;
    T GetEndTime( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The value at 'time', with the wrap modes applied outside the keys */
    T Evaluate( const T& time ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** As Evaluate( time ); 'hint' is the index of the segment found last time and is updated */
    T Evaluate( const T& time, UInt32& hint ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The slope at 'time' (0 on constant steps, mirrored on ping-pong repeats, zero for fewer than two keys) */
    T GetDerivative( const T& time ) const GUCEF_MATH_REALTIME_CONTEXT;

    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /**
     *  Maps 'time' into the key range: the wrapped time, the value offset (offset loop) and the sign of the slope
     *  ( -1 on mirrored repeats ); false when the curve has no usable duration
     */
    bool Wrap( const T& time, T& wrapped, T& offset, T& slopeSign ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The segment with time i <= t <= time i + 1 ( i < count - 1 ), trying 'hint' first */
    UInt32 FindSegment( const T& t, UInt32 hint ) const GUCEF_MATH_REALTIME_CONTEXT;

    T EvaluateInRange( const T& t, UInt32& hint ) const GUCEF_MATH_REALTIME_CONTEXT;
    T DerivativeInRange( const T& t, UInt32 hint ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF::vector< SKey > m_keys;
    TCurveWrapMode m_preWrap;
    TCurveWrapMode m_postWrap;
};

/*-------------------------------------------------------------------------*/

template < typename T >
CTCurve< T >::CTCurve( void )
    : m_keys()
    , m_preWrap( CURVEWRAP_CLAMP )
    , m_postWrap( CURVEWRAP_CLAMP )
{
    GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, CTCurve_requires_a_floating_point_kind );
}

template < typename T >
UInt32
CTCurve< T >::AddKey( const T& time, const T& value, const T& inTangent, const T& outTangent )
{
    if ( !CTNumericTraits< T >::IsFinite( time ) || !CTNumericTraits< T >::IsFinite( value ) ||
         !CTNumericTraits< T >::IsFinite( inTangent ) || !CTNumericTraits< T >::IsFinite( outTangent ) )
        return (UInt32) INVALID_INDEX;
    SKey key;
    key.time = time;
    key.value = value;
    key.inTangent = inTangent;
    key.outTangent = outTangent;
    key.mode = CURVETANGENT_FREE;
    UInt32 index = 0;
    while ( index < m_keys.size() && m_keys[ index ].time < time )
        ++index;
    if ( index < m_keys.size() && m_keys[ index ].time == time )
        m_keys[ index ] = key;
    else
        m_keys.insert( m_keys.begin() + index, key );
    return index;
}

template < typename T >
UInt32
CTCurve< T >::AddKey( const T& time, const T& value )
{
    const UInt32 index = AddKey( time, value, T( 0 ), T( 0 ) );
    if ( (UInt32) INVALID_INDEX == index )
        return index;
    m_keys[ index ].mode = CURVETANGENT_AUTO;
    return index;
}

template < typename T >
bool
CTCurve< T >::RemoveKey( UInt32 index )
{
    if ( index >= m_keys.size() )
        return false;
    m_keys.erase( m_keys.begin() + index );
    return true;
}

template < typename T >
bool
CTCurve< T >::SetTangentMode( UInt32 index, TCurveTangentMode mode )
{
    if ( index >= m_keys.size() )
        return false;
    m_keys[ index ].mode = mode;
    return true;
}

template < typename T >
bool
CTCurve< T >::SetTangents( UInt32 index, const T& inTangent, const T& outTangent )
{
    if ( index >= m_keys.size() )
        return false;
    m_keys[ index ].inTangent = inTangent;
    m_keys[ index ].outTangent = outTangent;
    m_keys[ index ].mode = CURVETANGENT_FREE;
    return true;
}

template < typename T >
void
CTCurve< T >::Clear( void )
{
    m_keys.clear();
}

template < typename T >
void
CTCurve< T >::SetPreWrapMode( TCurveWrapMode mode )
{
    m_preWrap = mode;
}

template < typename T >
void
CTCurve< T >::SetPostWrapMode( TCurveWrapMode mode )
{
    m_postWrap = mode;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTCurve< T >::SKey
CTCurve< T >::GetKey( UInt32 index ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return index < m_keys.size() ? m_keys[ index ] : SKey();
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::GetStartTime( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_keys.empty() ? T( 0 ) : m_keys.front().time;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::GetEndTime( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_keys.empty() ? T( 0 ) : m_keys.back().time;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTCurve< T >::GetEffectiveTangents( UInt32 index, T& inTangent, T& outTangent ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const size_t count = m_keys.size();
    if ( index >= count )
        return false;
    const SKey& key = m_keys[ index ];
    const bool hasPrevious = index > 0;
    const bool hasNext = index + 1 < count;
    switch ( key.mode )
    {
        case CURVETANGENT_LINEAR:
        {
            inTangent = hasPrevious ? ( key.value - m_keys[ index - 1 ].value ) / ( key.time - m_keys[ index - 1 ].time ) : T( 0 );
            outTangent = hasNext ? ( m_keys[ index + 1 ].value - key.value ) / ( m_keys[ index + 1 ].time - key.time ) : T( 0 );
            return true;
        }
        case CURVETANGENT_CONSTANT:
        {
            inTangent = T( 0 );
            outTangent = T( 0 );
            return true;
        }
        case CURVETANGENT_AUTO:
        case CURVETANGENT_CLAMPED_AUTO:
        {
            T slope = T( 0 );
            if ( hasPrevious && hasNext )
            {
                slope = ( m_keys[ index + 1 ].value - m_keys[ index - 1 ].value ) / ( m_keys[ index + 1 ].time - m_keys[ index - 1 ].time );
                if ( CURVETANGENT_CLAMPED_AUTO == key.mode )
                {
                    // flat at a local extremum; otherwise at most three times the smaller neighbouring secant slope
                    // (Fritsch-Carlson), which keeps every segment monotone between its keys
                    const T before = ( key.value - m_keys[ index - 1 ].value ) / ( key.time - m_keys[ index - 1 ].time );
                    const T after = ( m_keys[ index + 1 ].value - key.value ) / ( m_keys[ index + 1 ].time - key.time );
                    if ( !( before * after > T( 0 ) ) )
                        slope = T( 0 );
                    else
                    {
                        const T smaller = std::fabs( before ) < std::fabs( after ) ? std::fabs( before ) : std::fabs( after );
                        const T limit = T( 3 ) * smaller;
                        if ( std::fabs( slope ) > limit )
                            slope = slope < T( 0 ) ? -limit : limit;
                    }
                }
            }
            else if ( CURVETANGENT_AUTO == key.mode )
            {
                if ( hasNext )
                    slope = ( m_keys[ index + 1 ].value - key.value ) / ( m_keys[ index + 1 ].time - key.time );
                else if ( hasPrevious )
                    slope = ( key.value - m_keys[ index - 1 ].value ) / ( key.time - m_keys[ index - 1 ].time );
            }
            inTangent = slope;
            outTangent = slope;
            return true;
        }
        case CURVETANGENT_FREE:
        default:
        {
            inTangent = key.inTangent;
            outTangent = key.outTangent;
            return true;
        }
    }
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTCurve< T >::FindSegment( const T& t, UInt32 hint ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const UInt32 segments = (UInt32) m_keys.size() - 1;
    if ( hint < segments && m_keys[ hint ].time <= t && t <= m_keys[ hint + 1 ].time )
        return hint;
    if ( hint + 1 < segments && m_keys[ hint + 1 ].time <= t && t <= m_keys[ hint + 2 ].time )
        return hint + 1;
    UInt32 low = 0;
    UInt32 high = segments;
    while ( high - low > 1 )
    {
        const UInt32 mid = low + ( high - low ) / 2;
        if ( m_keys[ mid ].time <= t )
            low = mid;
        else
            high = mid;
    }
    return low;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::EvaluateInRange( const T& t, UInt32& hint ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( m_keys.size() < 2 )
        return m_keys.empty() ? T( 0 ) : m_keys[ 0 ].value;
    const UInt32 i = FindSegment( t, hint );
    hint = i;
    const SKey& a = m_keys[ i ];
    const SKey& b = m_keys[ i + 1 ];
    if ( t >= b.time )
        return b.value;
    if ( CURVETANGENT_CONSTANT == a.mode )
        return a.value;
    const T span = b.time - a.time;
    const T u = ( t - a.time ) / span;
    if ( CURVETANGENT_LINEAR == a.mode && CURVETANGENT_LINEAR == b.mode )
        return a.value + ( b.value - a.value ) * u;
    T aIn, aOut, bIn, bOut;
    GetEffectiveTangents( i, aIn, aOut );
    GetEffectiveTangents( i + 1, bIn, bOut );
    return HermiteEvaluate< T >( a.value, aOut * span, b.value, bIn * span, u );
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::DerivativeInRange( const T& t, UInt32 hint ) const GUCEF_MATH_REALTIME_CONTEXT
{
    if ( m_keys.size() < 2 )
        return T( 0 );
    const UInt32 i = FindSegment( t, hint );
    const SKey& a = m_keys[ i ];
    const SKey& b = m_keys[ i + 1 ];
    if ( CURVETANGENT_CONSTANT == a.mode )
        return T( 0 );
    const T span = b.time - a.time;
    const T u = ( t - a.time ) / span;
    if ( CURVETANGENT_LINEAR == a.mode && CURVETANGENT_LINEAR == b.mode )
        return ( b.value - a.value ) / span;
    T aIn, aOut, bIn, bOut;
    GetEffectiveTangents( i, aIn, aOut );
    GetEffectiveTangents( i + 1, bIn, bOut );
    return HermiteDerivative< T >( a.value, aOut * span, b.value, bIn * span, u ) / span;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTCurve< T >::Wrap( const T& time, T& wrapped, T& offset, T& slopeSign ) const GUCEF_MATH_REALTIME_CONTEXT
{
    wrapped = time;
    offset = T( 0 );
    slopeSign = T( 1 );
    if ( m_keys.size() < 2 )
        return false;
    const T first = m_keys.front().time;
    const T last = m_keys.back().time;
    const T duration = last - first;
    if ( time >= first && time <= last )
        return true;
    const TCurveWrapMode mode = time < first ? m_preWrap : m_postWrap;
    switch ( mode )
    {
        case CURVEWRAP_LOOP:
        case CURVEWRAP_OFFSET_LOOP:
        {
            const T cycles = std::floor( ( time - first ) / duration );
            wrapped = time - cycles * duration;
            if ( wrapped < first ) wrapped = first;
            if ( wrapped > last ) wrapped = last;
            if ( CURVEWRAP_OFFSET_LOOP == mode )
                offset = cycles * ( m_keys.back().value - m_keys.front().value );
            return true;
        }
        case CURVEWRAP_PINGPONG:
        {
            const T cycle = std::floor( ( time - first ) / duration );
            const T within = ( time - first ) - cycle * duration;
            const bool odd = T( 1 ) == std::fabs( std::fmod( cycle, T( 2 ) ) );
            wrapped = odd ? last - within : first + within;
            if ( wrapped < first ) wrapped = first;
            if ( wrapped > last ) wrapped = last;
            slopeSign = odd ? T( -1 ) : T( 1 );
            return true;
        }
        case CURVEWRAP_CLAMP:
        default:
        {
            wrapped = time < first ? first : last;
            slopeSign = T( 0 );
            return true;
        }
    }
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::Evaluate( const T& time, UInt32& hint ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T wrapped, offset, sign;
    if ( !Wrap( time, wrapped, offset, sign ) )
        return m_keys.empty() ? T( 0 ) : m_keys[ 0 ].value;
    return EvaluateInRange( wrapped, hint ) + offset;
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::Evaluate( const T& time ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 hint = 0;
    return Evaluate( time, hint );
}

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTCurve< T >::GetDerivative( const T& time ) const GUCEF_MATH_REALTIME_CONTEXT
{
    T wrapped, offset, sign;
    if ( !Wrap( time, wrapped, offset, sign ) )
        return T( 0 );
    return sign * DerivativeInRange( wrapped, 0 );
}

template < typename T >
bool
CTCurve< T >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& ) const
{
    if ( !node.SetAttribute( CORE::CString( "preWrap" ), (UInt32) m_preWrap ) || !node.SetAttribute( CORE::CString( "postWrap" ), (UInt32) m_postWrap ) )
        return false;
    const char* const names[ 4 ] = { "time", "value", "in", "out" };
    for ( size_t i=0; i<m_keys.size(); ++i )
    {
        CORE::CDataNode* child = node.AddChild( CORE::CString( "key" ) );
        const T components[ 4 ] = { m_keys[ i ].time, m_keys[ i ].value, m_keys[ i ].inTangent, m_keys[ i ].outTangent };
        if ( GUCEF_NULL == child || !SerializeComponents( *child, names, components ) || !child->SetAttribute( CORE::CString( "mode" ), (UInt32) m_keys[ i ].mode ) )
            return false;
    }
    return true;
}

template < typename T >
bool
CTCurve< T >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& )
{
    const CORE::CDataNode::TKeyValuePair* pre = node.GetAttribute( CORE::CString( "preWrap" ) );
    const CORE::CDataNode::TKeyValuePair* post = node.GetAttribute( CORE::CString( "postWrap" ) );
    if ( GUCEF_NULL == pre || GUCEF_NULL == post )
        return false;
    const UInt32 preWrap = pre->second.AsUInt32();
    const UInt32 postWrap = post->second.AsUInt32();
    if ( preWrap > (UInt32) CURVEWRAP_OFFSET_LOOP || postWrap > (UInt32) CURVEWRAP_OFFSET_LOOP )
        return false;

    const char* const names[ 4 ] = { "time", "value", "in", "out" };
    const CORE::CString keyName( "key" );
    GUCEF::vector< SKey > keys;
    for ( const CORE::CDataNode* child = node.GetFirstChild(); GUCEF_NULL != child; child = child->GetNext() )
    {
        if ( child->GetName() != keyName )
            continue;
        T components[ 4 ];
        const CORE::CDataNode::TKeyValuePair* mode = child->GetAttribute( CORE::CString( "mode" ) );
        if ( GUCEF_NULL == mode || !DeserializeComponents( *child, names, components ) || mode->second.AsUInt32() > (UInt32) CURVETANGENT_CLAMPED_AUTO )
            return false;
        SKey key;
        key.time = components[ 0 ];
        key.value = components[ 1 ];
        key.inTangent = components[ 2 ];
        key.outTangent = components[ 3 ];
        key.mode = (TCurveTangentMode) mode->second.AsUInt32();
        if ( !keys.empty() && !( keys.back().time < key.time ) )
            return false;
        keys.push_back( key );
    }
    m_keys.swap( keys );
    m_preWrap = (TCurveWrapMode) preWrap;
    m_postWrap = (TCurveWrapMode) postWrap;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTCurve< Float32 > )
GUCEF_MATH_EXTERN_TEMPLATE_CLASS( CTCurve< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTCURVE_H ? */
