/*
 *  gucefCORE: GUCEF module providing O/S abstractions and basic framework
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCORE_MacClipboard.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#include <string.h>
#include <ApplicationServices/ApplicationServices.h>

/*-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------*/

/** The most that is taken from, or put on, the clipboard: the same limit as the X11 clipboard has */
static const UInt32 MaxTextSizeInBytes = 16 * 1024 * 1024;

/*-------------------------------------------------------------------------*/

bool
MacClipboardSetUtf8( const char* utf8Text, UInt32 byteCount )
{
    if ( GUCEF_NULL == utf8Text || byteCount > MaxTextSizeInBytes )
        return false;

    PasteboardRef pasteboard = GUCEF_NULL;
    if ( noErr != ::PasteboardCreate( kPasteboardClipboard, &pasteboard ) || GUCEF_NULL == pasteboard )
        return false;

    bool success = false;
    if ( noErr == ::PasteboardClear( pasteboard ) )
    {
        CFDataRef data = ::CFDataCreate( kCFAllocatorDefault, (const UInt8*) utf8Text, (CFIndex) byteCount );
        if ( GUCEF_NULL != data )
        {
            success = noErr == ::PasteboardPutItemFlavor( pasteboard, (PasteboardItemID) 1, CFSTR( "public.utf8-plain-text" ), data, kPasteboardFlavorNoFlags );
            ::CFRelease( data );
        }
    }
    ::CFRelease( pasteboard );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
MacClipboardGetUtf8( char* dest, UInt32 size, UInt32 offset, UInt32& bytesWritten )
{
    bytesWritten = 0;
    if ( GUCEF_NULL == dest || size < 2 )
        return false;

    PasteboardRef pasteboard = GUCEF_NULL;
    if ( noErr != ::PasteboardCreate( kPasteboardClipboard, &pasteboard ) || GUCEF_NULL == pasteboard )
        return false;
    ::PasteboardSynchronize( pasteboard );

    bool success = false;
    ItemCount itemCount = 0;
    if ( noErr == ::PasteboardGetItemCount( pasteboard, &itemCount ) )
    {
        for ( CFIndex i=1; i<=(CFIndex) itemCount && !success; ++i )
        {
            PasteboardItemID item = GUCEF_NULL;
            if ( noErr != ::PasteboardGetItemIdentifier( pasteboard, (CFIndex) i, &item ) )
                continue;

            CFDataRef data = GUCEF_NULL;
            if ( noErr != ::PasteboardCopyItemFlavorData( pasteboard, item, CFSTR( "public.utf8-plain-text" ), &data ) || GUCEF_NULL == data )
                continue;

            UInt32 textSize = (UInt32) ::CFDataGetLength( data );
            if ( textSize > MaxTextSizeInBytes )
                textSize = MaxTextSizeInBytes;
            if ( textSize > offset )
            {
                UInt32 copySize = textSize - offset;
                if ( copySize > size-1 )
                    copySize = size-1;
                ::memcpy( dest, ::CFDataGetBytePtr( data ) + offset, copySize );
                dest[ copySize ] = '\0';
                bytesWritten = copySize;
                success = true;
            }
            ::CFRelease( data );
        }
    }
    ::CFRelease( pasteboard );
    return success;
}

/*-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */
