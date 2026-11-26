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

#ifndef GUCEF_CORE_MACCLIPBOARD_H
#define GUCEF_CORE_MACCLIPBOARD_H

/* Private to gucefCORE: the macOS side of StringToClipboard() and StringFromClipboard() of DVOSWRAP.h */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

namespace GUCEF {
namespace CORE {

/** Puts the UTF-8 text on the general pasteboard of the user session, replacing what was there. False without a window server session */
bool MacClipboardSetUtf8( const char* utf8Text, UInt32 byteCount );

/** Copies the UTF-8 text of the general pasteboard into the buffer (zero terminated, cut off at size-1 bytes), skipping the first offset bytes of the text. False when there is no text */
bool MacClipboardGetUtf8( char* dest, UInt32 size, UInt32 offset, UInt32& bytesWritten );

}; /* namespace CORE */
}; /* namespace GUCEF */

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */

#endif /* GUCEF_CORE_MACCLIPBOARD_H ? */
