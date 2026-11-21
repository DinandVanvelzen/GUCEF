/*
 *  AssetConverterLib: reads the size of a picture from the first bytes of its file
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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "AssetConverterLib_CPackIndexImageProbe.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt32 Be32( const UInt8* p ) { return ( (UInt32) p[ 0 ] << 24 ) | ( (UInt32) p[ 1 ] << 16 ) | ( (UInt32) p[ 2 ] << 8 ) | (UInt32) p[ 3 ]; }
UInt32 Le32( const UInt8* p ) { return ( (UInt32) p[ 3 ] << 24 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 1 ] << 8 ) | (UInt32) p[ 0 ]; }
UInt32 Be16( const UInt8* p ) { return ( (UInt32) p[ 0 ] << 8 ) | (UInt32) p[ 1 ]; }
UInt32 Le16( const UInt8* p ) { return ( (UInt32) p[ 1 ] << 8 ) | (UInt32) p[ 0 ]; }

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexImageProbe::IsSupported( const CORE::CString& e )
{GUCEF_TRACE;

    return "png" == e || "jpg" == e || "jpeg" == e || "bmp" == e || "gif" == e || "tga" == e || "dds" == e;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexImageProbe::ProbeBytes( const UInt8* b, UInt32 size, const CORE::CString& e, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitDepth )
{GUCEF_TRACE;

    width = height = channels = bitDepth = 0;
    if ( "png" == e )
    {
        static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
        if ( size < 26 || 0 != memcmp( b, signature, 8 ) )
            return false;
        width = Be32( b + 16 );
        height = Be32( b + 20 );
        bitDepth = b[ 24 ];
        switch ( b[ 25 ] )
        {
            case 0: channels = 1; break;    // grey
            case 2: channels = 3; break;    // RGB
            case 3: channels = 3; break;    // palette
            case 4: channels = 2; break;    // grey and alpha
            case 6: channels = 4; break;    // RGBA
            default: return false;
        }
        return width > 0 && height > 0;
    }
    if ( "jpg" == e || "jpeg" == e )
    {
        if ( size < 4 || 0xFF != b[ 0 ] || 0xD8 != b[ 1 ] )
            return false;
        UInt32 i = 2;
        while ( i + 9 < size )
        {
            if ( 0xFF != b[ i ] )
            {
                ++i;
                continue;
            }
            const UInt8 marker = b[ i+1 ];
            if ( 0xFF == marker )
            {
                ++i;
                continue;
            }
            if ( ( marker >= 0xC0 && marker <= 0xCF ) && 0xC4 != marker && 0xC8 != marker && 0xCC != marker )
            {
                bitDepth = b[ i+4 ];
                height = Be16( b + i + 5 );
                width = Be16( b + i + 7 );
                channels = b[ i+9 ];
                return width > 0 && height > 0;
            }
            if ( 0xD8 == marker || 0x01 == marker || ( marker >= 0xD0 && marker <= 0xD7 ) )
            {
                i += 2;
                continue;
            }
            i += 2 + Be16( b + i + 2 );
        }
        return false;
    }
    if ( "bmp" == e )
    {
        if ( size < 30 || 'B' != b[ 0 ] || 'M' != b[ 1 ] )
            return false;
        width = Le32( b + 18 );
        const Int32 signedHeight = (Int32) Le32( b + 22 );
        height = (UInt32) ( signedHeight < 0 ? -signedHeight : signedHeight );
        bitDepth = Le16( b + 28 );
        channels = bitDepth >= 32 ? 4 : ( bitDepth >= 24 ? 3 : 1 );
        return width > 0 && height > 0;
    }
    if ( "gif" == e )
    {
        if ( size < 10 || 'G' != b[ 0 ] || 'I' != b[ 1 ] || 'F' != b[ 2 ] )
            return false;
        width = Le16( b + 6 );
        height = Le16( b + 8 );
        channels = 3;
        bitDepth = 8;
        return width > 0 && height > 0;
    }
    if ( "tga" == e )
    {
        if ( size < 18 )
            return false;
        width = Le16( b + 12 );
        height = Le16( b + 14 );
        bitDepth = b[ 16 ];
        channels = bitDepth >= 32 ? 4 : ( bitDepth >= 24 ? 3 : 1 );
        return width > 0 && height > 0 && ( 8 == bitDepth || 15 == bitDepth || 16 == bitDepth || 24 == bitDepth || 32 == bitDepth );
    }
    if ( "dds" == e )
    {
        if ( size < 20 || 'D' != b[ 0 ] || 'D' != b[ 1 ] || 'S' != b[ 2 ] || ' ' != b[ 3 ] )
            return false;
        height = Le32( b + 12 );
        width = Le32( b + 16 );
        return width > 0 && height > 0;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexImageProbe::Probe( const CORE::CString& vfsPath, const CORE::CString& e, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitDepth )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "rb", false );
    if ( resource.IsNULL() )
        return false;
    CORE::IOAccessPtr access = resource->GetAccess();
    if ( access.IsNULL() )
        return false;
    UInt8 bytes[ 65536 ];
    const UInt32 read = access->Read( bytes, 1, sizeof( bytes ) );
    return ProbeBytes( bytes, read, e, width, height, channels, bitDepth );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
