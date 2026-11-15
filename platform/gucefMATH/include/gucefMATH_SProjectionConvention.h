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

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#define GUCEF_MATH_SPROJECTIONCONVENTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The conventions of a projection matrix (section 4.5.6 of the design). Every projection builder of CTMatrix4D takes
 *  one: there is no default, so the depth range, handedness and Y direction of the target API are always stated.
 *
 *  - handedness: of view space; a right-handed view looks down -Z, a left-handed view down +Z.
 *  - depthRange: the normalised device depth range, [0, 1] (Vulkan, Direct3D, Metal) or [-1, 1] (OpenGL).
 *  - reversedZ: the near plane maps to the far end of the depth range and the far plane to the near end.
 *  - infiniteFar: the far plane is at infinity (perspective only; the far distance argument is ignored).
 *  - flipY: clip-space Y points down (Vulkan without a negative viewport height).
 *  Ogre 1.x and Ogre-next convert projection matrices for their render system internally: feed them OpenGL().
 *  Text: "(right, 0..1, reversedZ, infiniteFar, flipY)" with only the flags that are set; DataNode: attributes
 *  handedness ("right" / "left"), depthRange ("0..1" / "-1..1"), reversedZ, infiniteFar, flipY (booleans).
 *  Plain data, realtime-safe except the text and DataNode members.
 */
struct SProjectionConvention
{
    THandedness handedness;
    TDepthRange depthRange;
    bool reversedZ;
    bool infiniteFar;
    bool flipY;

    /** Right-handed, depth [-1, 1], no reversal, finite far plane, Y up */
    static SProjectionConvention OpenGL( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Make( HANDEDNESS_RIGHT, DEPTHRANGE_MINUS_ONE_TO_ONE, false, false, false );
    }

    /** Right-handed, depth [0, 1], no reversal, finite far plane, clip-space Y down */
    static SProjectionConvention Vulkan( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Make( HANDEDNESS_RIGHT, DEPTHRANGE_ZERO_TO_ONE, false, false, true );
    }

    /** Left-handed, depth [0, 1], no reversal, finite far plane, Y up */
    static SProjectionConvention Direct3D( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Make( HANDEDNESS_LEFT, DEPTHRANGE_ZERO_TO_ONE, false, false, false );
    }

    /** Depth [0, 1] with near at 1, far plane at infinity, Y up: the best depth precision for floating-point buffers */
    static SProjectionConvention ReversedZInfinite( THandedness viewHandedness ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return Make( viewHandedness, DEPTHRANGE_ZERO_TO_ONE, true, true, false );
    }

    /** Every field given */
    static SProjectionConvention Make( THandedness viewHandedness, TDepthRange range, bool reversed, bool infinite, bool flipClipY ) GUCEF_MATH_REALTIME_CONTEXT
    {
        SProjectionConvention convention;
        convention.handedness = viewHandedness;
        convention.depthRange = range;
        convention.reversedZ = reversed;
        convention.infiniteFar = infinite;
        convention.flipY = flipClipY;
        return convention;
    }

    /** The normalised device depth of the near plane: 0 or -1, or the far end of the range when reversed */
    Int32 GetNearDepth( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        Int32 low = DEPTHRANGE_ZERO_TO_ONE == depthRange ? 0 : -1;
        return reversedZ ? 1 : low;
    }

    /** The normalised device depth of the far plane (the limit for an infinite far plane) */
    Int32 GetFarDepth( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        Int32 low = DEPTHRANGE_ZERO_TO_ONE == depthRange ? 0 : -1;
        return reversedZ ? low : 1;
    }

    friend bool operator==( const SProjectionConvention& a, const SProjectionConvention& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.handedness == b.handedness && a.depthRange == b.depthRange && a.reversedZ == b.reversedZ &&
               a.infiniteFar == b.infiniteFar && a.flipY == b.flipY;
    }

    friend bool operator!=( const SProjectionConvention& a, const SProjectionConvention& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(right, 0..1, reversedZ, infiniteFar, flipY)" listing only the flags that are set */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( HANDEDNESS_LEFT == handedness ? "(left, " : "(right, " );
        text += DEPTHRANGE_ZERO_TO_ONE == depthRange ? "0..1" : "-1..1";
        if ( reversedZ )
            text += ", reversedZ";
        if ( infiniteFar )
            text += ", infiniteFar";
        if ( flipY )
            text += ", flipY";
        text += ')';
        return text;
    }

    /** Parses the ToString form (white space allowed, each flag at most once); false leaves this value unchanged */
    bool FromString( const CORE::CString& text )
    {
        const char* data = text.C_String();
        UInt32 length = text.Length();
        if ( GUCEF_NULL == data )
            return false;
        UInt32 first = 0;
        UInt32 last = length;
        while ( first < last && IsComponentTextWhitespace( data[ first ] ) )
            ++first;
        while ( last > first && IsComponentTextWhitespace( data[ last - 1 ] ) )
            --last;
        if ( last - first < 2 || '(' != data[ first ] || ')' != data[ last - 1 ] )
            return false;

        SProjectionConvention parsed = OpenGL();
        bool seenReversed = false;
        bool seenInfinite = false;
        bool seenFlip = false;
        UInt32 field = 0;
        UInt32 position = first + 1;
        while ( position <= last - 1 )
        {
            UInt32 end = position;
            while ( end < last - 1 && ',' != data[ end ] )
                ++end;
            UInt32 a = position;
            UInt32 b = end;
            while ( a < b && IsComponentTextWhitespace( data[ a ] ) )
                ++a;
            while ( b > a && IsComponentTextWhitespace( data[ b - 1 ] ) )
                --b;
            CORE::CString token( data + a, b - a );
            if ( 0 == field )
            {
                if ( token == "right" ) parsed.handedness = HANDEDNESS_RIGHT;
                else if ( token == "left" ) parsed.handedness = HANDEDNESS_LEFT;
                else return false;
            }
            else if ( 1 == field )
            {
                if ( token == "0..1" ) parsed.depthRange = DEPTHRANGE_ZERO_TO_ONE;
                else if ( token == "-1..1" ) parsed.depthRange = DEPTHRANGE_MINUS_ONE_TO_ONE;
                else return false;
            }
            else if ( token == "reversedZ" && !seenReversed ) { parsed.reversedZ = true; seenReversed = true; }
            else if ( token == "infiniteFar" && !seenInfinite ) { parsed.infiniteFar = true; seenInfinite = true; }
            else if ( token == "flipY" && !seenFlip ) { parsed.flipY = true; seenFlip = true; }
            else return false;
            ++field;
            position = end + 1;
        }
        if ( field < 2 )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the attributes handedness, depthRange, reversedZ, infiniteFar and flipY into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        return node.SetAttribute( "handedness", CORE::CVariant( HANDEDNESS_LEFT == handedness ? "left" : "right" ) ) &&
               node.SetAttribute( "depthRange", CORE::CVariant( DEPTHRANGE_ZERO_TO_ONE == depthRange ? "0..1" : "-1..1" ) ) &&
               node.SetAttribute( "reversedZ", CORE::CVariant( reversedZ ) ) &&
               node.SetAttribute( "infiniteFar", CORE::CVariant( infiniteFar ) ) &&
               node.SetAttribute( "flipY", CORE::CVariant( flipY ) );
    }

    /** Reads the attributes of Serialize; a missing or invalid attribute fails and leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        const CORE::CDataNode::TKeyValuePair* hand = node.GetAttribute( "handedness" );
        const CORE::CDataNode::TKeyValuePair* range = node.GetAttribute( "depthRange" );
        const CORE::CDataNode::TKeyValuePair* reversed = node.GetAttribute( "reversedZ" );
        const CORE::CDataNode::TKeyValuePair* infinite = node.GetAttribute( "infiniteFar" );
        const CORE::CDataNode::TKeyValuePair* flip = node.GetAttribute( "flipY" );
        if ( GUCEF_NULL == hand || GUCEF_NULL == range || GUCEF_NULL == reversed || GUCEF_NULL == infinite || GUCEF_NULL == flip )
            return false;
        SProjectionConvention parsed = OpenGL();
        CORE::CString handText = hand->second.AsString();
        CORE::CString rangeText = range->second.AsString();
        if ( handText == "right" ) parsed.handedness = HANDEDNESS_RIGHT;
        else if ( handText == "left" ) parsed.handedness = HANDEDNESS_LEFT;
        else return false;
        if ( rangeText == "0..1" ) parsed.depthRange = DEPTHRANGE_ZERO_TO_ONE;
        else if ( rangeText == "-1..1" ) parsed.depthRange = DEPTHRANGE_MINUS_ONE_TO_ONE;
        else return false;
        if ( !ReadFlag( reversed->second, parsed.reversedZ ) || !ReadFlag( infinite->second, parsed.infiniteFar ) || !ReadFlag( flip->second, parsed.flipY ) )
            return false;
        *this = parsed;
        return true;
    }

    private:

    /** A boolean attribute: a boolean or integer variant, or the text "true" / "false" / "1" / "0" */
    static bool ReadFlag( const CORE::CVariant& value, bool& out )
    {
        if ( value.IsBoolean() || value.IsInteger() )
        {
            out = value.AsBool();
            return true;
        }
        CORE::CString text = value.AsString();
        if ( text == "true" || text == "1" ) { out = true; return true; }
        if ( text == "false" || text == "0" ) { out = false; return true; }
        return false;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */
