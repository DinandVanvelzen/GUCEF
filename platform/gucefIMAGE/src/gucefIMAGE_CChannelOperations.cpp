/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#include "gucefIMAGE_CPixelMapChannelAccess.h"
#define GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H
#endif /* GUCEF_IMAGE_CPIXELMAPCHANNELACCESS_H ? */

#include "gucefIMAGE_CChannelOperations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A source channel that is a constant instead */
const Int32 CHANNEL_CONSTANT_ZERO = -1;
const Int32 CHANNEL_CONSTANT_ONE = -2;
const Int32 CHANNEL_INVALID = -3;

/** The storage index of the channel a mapping letter names in a format, or one of the constants above */
Int32
ResolveSwizzleLetter( TPixelStorageFormat format, char letter )
{
    if ( '0' == letter )
        return CHANNEL_CONSTANT_ZERO;
    if ( '1' == letter )
        return CHANNEL_CONSTANT_ONE;

    switch ( format )
    {
        case PSF_RGB : case PSF_RGBA : case PSF_HSV : case PSF_HSVA : case PSF_HSL : case PSF_HSLA :
        {
            switch ( letter )
            {
                case 'R' : return 0;
                case 'G' : return 1;
                case 'B' : return 2;
                case 'A' : return CPixelMap::GetChannelCountForFormat( format ) > 3 ? 3 : CHANNEL_INVALID;
                default : return CHANNEL_INVALID;
            }
        }
        case PSF_BGR : case PSF_BGRA :
        {
            switch ( letter )
            {
                case 'B' : return 0;
                case 'G' : return 1;
                case 'R' : return 2;
                case 'A' : return PSF_BGRA == format ? 3 : CHANNEL_INVALID;
                default : return CHANNEL_INVALID;
            }
        }
        case PSF_SINGLE_CHANNEL_ALPHA :
        {
            return 'A' == letter ? 0 : CHANNEL_INVALID;
        }
        default :
        {
            return 'R' == letter || 'G' == letter || 'B' == letter ? 0 : CHANNEL_INVALID;
        }
    }
}

/** Copies one channel value, converting when the data types differ */
inline void
CopyValue( const UInt8* source, TBuildinDataType sourceType, UInt32 sourceSize, UInt8* target, TBuildinDataType targetType )
{
    if ( sourceType == targetType )
        memcpy( target, source, sourceSize );
    else
        CPixelMapChannelAccess::WriteValue( CPixelMapChannelAccess::ReadValue( source, sourceType ), targetType, target );
}

bool
CheckSupported( const CPixelMap& map, const char* operation )
{
    if ( !CPixelMapChannelAccess::IsSupported( map ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: " << operation << ": the picture has no data, is palettised or has a storage format or data type that is not supported" );
        return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CChannelOperations::ExtractChannel( const CPixelMap& source, UInt32 channelIndex, TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( !CheckSupported( source, "ExtractChannel" ) )
        return false;

    const UInt32 channelCount = source.GetNumberOfChannelsPerPixel();
    if ( channelIndex >= channelCount )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: ExtractChannel: the picture has no channel " << channelIndex );
        return false;
    }

    const TBuildinDataType dataType = source.GetPixelComponentDataType();
    const UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    const UInt32 pixelCount = source.GetPixelCount();
    TPixelMapPtr target( GUCEF_NEW CPixelMap( GUCEF_NULL, source.GetWidthInPixels(), source.GetHeightInPixels(), PSF_SINGLE_CHANNEL_GRAYSCALE, dataType ) );

    const UInt8* in = static_cast< const UInt8* >( source.GetDataPtr() ) + (size_t) channelIndex * valueSize;
    UInt8* out = static_cast< UInt8* >( target->GetDataPtr() );
    for ( UInt32 i=0; i<pixelCount; ++i )
        memcpy( out + (size_t) i * valueSize, in + (size_t) i * channelCount * valueSize, valueSize );

    result = target;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::ExtractChannel( CPixelMap& map, UInt32 channelIndex )
{GUCEF_TRACE;

    TPixelMapPtr result;
    if ( !ExtractChannel( map, channelIndex, result ) )
        return false;
    map = *result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::Swizzle( const CPixelMap& source, const CORE::CString& mapping, TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( !CheckSupported( source, "Swizzle" ) )
        return false;

    const UInt32 mappingLength = mapping.Length();
    TPixelStorageFormat targetFormat;
    switch ( mappingLength )
    {
        case 1 : targetFormat = PSF_SINGLE_CHANNEL_GRAYSCALE; break;
        case 3 : targetFormat = PSF_RGB; break;
        case 4 : targetFormat = PSF_RGBA; break;
        default :
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: Swizzle: the mapping \"" << mapping << "\" must have 1, 3 or 4 characters" );
            return false;
        }
    }

    const TPixelStorageFormat sourceFormat = source.GetPixelStorageFormat();
    Int32 channels[ 4 ] = { CHANNEL_INVALID, CHANNEL_INVALID, CHANNEL_INVALID, CHANNEL_INVALID };
    for ( UInt32 i=0; i<mappingLength; ++i )
    {
        char letter = mapping.C_String()[ i ];
        if ( letter >= 'a' && letter <= 'z' )
            letter = (char) ( letter - 'a' + 'A' );
        channels[ i ] = ResolveSwizzleLetter( sourceFormat, letter );
        if ( CHANNEL_INVALID == channels[ i ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: Swizzle: the mapping \"" << mapping << "\" names a channel that the picture does not have" );
            return false;
        }
    }

    const TBuildinDataType dataType = source.GetPixelComponentDataType();
    const UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    const UInt32 sourceChannelCount = source.GetNumberOfChannelsPerPixel();
    const UInt32 pixelCount = source.GetPixelCount();
    TPixelMapPtr target( GUCEF_NEW CPixelMap( GUCEF_NULL, source.GetWidthInPixels(), source.GetHeightInPixels(), targetFormat, dataType ) );

    // The constants as values of the data type
    UInt8 zero[ 8 ], one[ 8 ];
    CPixelMapChannelAccess::WriteValue( 0.0f, dataType, zero );
    CPixelMapChannelAccess::WriteValue( 1.0f, dataType, one );

    const UInt8* in = static_cast< const UInt8* >( source.GetDataPtr() );
    UInt8* out = static_cast< UInt8* >( target->GetDataPtr() );
    for ( UInt32 i=0; i<pixelCount; ++i )
    {
        const UInt8* inPixel = in + (size_t) i * sourceChannelCount * valueSize;
        UInt8* outPixel = out + (size_t) i * mappingLength * valueSize;
        for ( UInt32 c=0; c<mappingLength; ++c )
        {
            if ( CHANNEL_CONSTANT_ZERO == channels[ c ] )
                memcpy( outPixel + c * valueSize, zero, valueSize );
            else
            if ( CHANNEL_CONSTANT_ONE == channels[ c ] )
                memcpy( outPixel + c * valueSize, one, valueSize );
            else
                memcpy( outPixel + c * valueSize, inPixel + (size_t) channels[ c ] * valueSize, valueSize );
        }
    }

    result = target;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::Swizzle( CPixelMap& map, const CORE::CString& mapping )
{GUCEF_TRACE;

    TPixelMapPtr result;
    if ( !Swizzle( map, mapping, result ) )
        return false;
    map = *result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::Invert( CPixelMap& map, UInt32 channelMask )
{GUCEF_TRACE;

    if ( !CheckSupported( map, "Invert" ) )
        return false;

    const TBuildinDataType dataType = map.GetPixelComponentDataType();
    const UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    const UInt32 channelCount = map.GetNumberOfChannelsPerPixel();
    const UInt32 pixelCount = map.GetPixelCount();
    const bool negate = CPixelMapChannelAccess::IsSignedInteger( dataType );

    UInt8* data = static_cast< UInt8* >( map.GetDataPtr() );
    for ( UInt32 c=0; c<channelCount && c<32; ++c )
    {
        if ( 0 == ( channelMask & ( (UInt32) 1 << c ) ) )
            continue;
        for ( UInt32 i=0; i<pixelCount; ++i )
        {
            UInt8* value = data + ( (size_t) i * channelCount + c ) * valueSize;
            Float32 current = CPixelMapChannelAccess::ReadValue( value, dataType );
            CPixelMapChannelAccess::WriteValue( negate ? -current : 1.0f - current, dataType, value );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::Invert( const CPixelMap& source, UInt32 channelMask, TPixelMapPtr& result )
{GUCEF_TRACE;

    TPixelMapPtr copy( GUCEF_NEW CPixelMap( source ) );
    if ( !Invert( *copy, channelMask ) )
        return false;
    result = copy;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::ScaleBias( CPixelMap& map, UInt32 channelMask, Float32 scale, Float32 bias )
{GUCEF_TRACE;

    if ( !CheckSupported( map, "ScaleBias" ) )
        return false;

    const TBuildinDataType dataType = map.GetPixelComponentDataType();
    const UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    const UInt32 channelCount = map.GetNumberOfChannelsPerPixel();
    const UInt32 pixelCount = map.GetPixelCount();

    UInt8* data = static_cast< UInt8* >( map.GetDataPtr() );
    for ( UInt32 c=0; c<channelCount && c<32; ++c )
    {
        if ( 0 == ( channelMask & ( (UInt32) 1 << c ) ) )
            continue;
        for ( UInt32 i=0; i<pixelCount; ++i )
        {
            UInt8* value = data + ( (size_t) i * channelCount + c ) * valueSize;
            Float32 current = CPixelMapChannelAccess::ReadValue( value, dataType );
            CPixelMapChannelAccess::WriteValue( current * scale + bias, dataType, value );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::ScaleBias( const CPixelMap& source, UInt32 channelMask, Float32 scale, Float32 bias, TPixelMapPtr& result )
{GUCEF_TRACE;

    TPixelMapPtr copy( GUCEF_NEW CPixelMap( source ) );
    if ( !ScaleBias( *copy, channelMask, scale, bias ) )
        return false;
    result = copy;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::Pack( const TPixelMapPtr sources[ 4 ], const UInt32 sourceChannels[ 4 ], TPixelMapPtr& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sources || GUCEF_NULL == sourceChannels )
        return false;

    // The size and data type come from the sources that are there
    const CPixelMap* first = GUCEF_NULL;
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( sources[ i ].IsNULL() )
            continue;
        const CPixelMap& source = *sources[ i ];
        if ( !CheckSupported( source, "Pack" ) )
            return false;
        if ( sourceChannels[ i ] >= source.GetNumberOfChannelsPerPixel() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: Pack: source " << i << " has no channel " << sourceChannels[ i ] );
            return false;
        }
        if ( GUCEF_NULL == first )
            first = &source;
        else
        if ( source.GetWidthInPixels() != first->GetWidthInPixels() || source.GetHeightInPixels() != first->GetHeightInPixels() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: Pack: the sources differ in size, resample them to the same size first" );
            return false;
        }
    }
    if ( GUCEF_NULL == first )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: Pack: no source to take the size from" );
        return false;
    }

    const TBuildinDataType dataType = first->GetPixelComponentDataType();
    const UInt32 valueSize = CPixelMap::GetPixelChannelSize( dataType );
    const UInt32 pixelCount = first->GetPixelCount();
    TPixelMapPtr target( GUCEF_NEW CPixelMap( GUCEF_NULL, first->GetWidthInPixels(), first->GetHeightInPixels(), PSF_RGBA, dataType ) );

    UInt8 zero[ 8 ], one[ 8 ];
    CPixelMapChannelAccess::WriteValue( 0.0f, dataType, zero );
    CPixelMapChannelAccess::WriteValue( 1.0f, dataType, one );

    UInt8* out = static_cast< UInt8* >( target->GetDataPtr() );
    for ( UInt32 c=0; c<4; ++c )
    {
        if ( sources[ c ].IsNULL() )
        {
            const UInt8* constant = 3 == c ? one : zero;
            for ( UInt32 i=0; i<pixelCount; ++i )
                memcpy( out + ( (size_t) i * 4 + c ) * valueSize, constant, valueSize );
        }
        else
        {
            const CPixelMap& source = *sources[ c ];
            const TBuildinDataType sourceType = source.GetPixelComponentDataType();
            const UInt32 sourceValueSize = CPixelMap::GetPixelChannelSize( sourceType );
            const UInt32 sourceChannelCount = source.GetNumberOfChannelsPerPixel();
            const UInt8* in = static_cast< const UInt8* >( source.GetDataPtr() );
            for ( UInt32 i=0; i<pixelCount; ++i )
                CopyValue( in + ( (size_t) i * sourceChannelCount + sourceChannels[ c ] ) * sourceValueSize, sourceType, sourceValueSize,
                           out + ( (size_t) i * 4 + c ) * valueSize, dataType );
        }
    }

    result = target;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::ConvertColourSpace( CPixelMap& map, TColourSpace from, TColourSpace to )
{GUCEF_TRACE;

    if ( !CheckSupported( map, "ConvertColourSpace" ) )
        return false;

    const TPixelStorageFormat format = map.GetPixelStorageFormat();
    if ( 0 == CPixelMapChannelAccess::GetColourChannelCount( format ) || CPixelMapChannelAccess::IsHueBased( format ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ChannelOperations: ConvertColourSpace: the picture has no red, green and blue channels" );
        return false;
    }
    if ( from == to )
        return true;

    CPixelMapChannelAccess::TFloatVector values;
    if ( !CPixelMapChannelAccess::Read( map, values ) )
        return false;
    CPixelMapChannelAccess::ConvertColourSpace( &values[ 0 ], map.GetPixelCount(), format, from, to );
    return CPixelMapChannelAccess::Write( &values[ 0 ], map );
}

/*-------------------------------------------------------------------------*/

bool
CChannelOperations::ConvertColourSpace( const CPixelMap& source, TColourSpace from, TColourSpace to, TPixelMapPtr& result )
{GUCEF_TRACE;

    TPixelMapPtr copy( GUCEF_NEW CPixelMap( source ) );
    if ( !ConvertColourSpace( *copy, from, to ) )
        return false;
    result = copy;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
