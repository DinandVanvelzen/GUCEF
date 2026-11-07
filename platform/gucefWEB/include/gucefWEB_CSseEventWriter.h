/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CSSEEVENTWRITER_H
#define GUCEF_WEB_CSSEEVENTWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Frames Server-Sent Events (the text/event-stream format of the HTML living standard) into a buffer, which the
 *  caller sends, for example with CHttpServerExchange::SendStreamData(). Lines end in LF.
 *
 *  Text is written as given; the line breaks in it (CR LF, LF or a CR on its own) start new "data:" lines, so no
 *  text can end an event early or add fields. A field name is refused when it holds a line break or a colon.
 */
class GUCEF_WEB_PUBLIC_CPP CSseEventWriter
{
    public:

    /**
     *  Appends one event: an "event:" line when a name is given, one "data:" line per line of the text (one empty
     *  "data:" line for empty text, so that the event is dispatched), and the empty line that ends the event
     *  @return false when the event name is not valid, nothing is appended then
     */
    static bool AppendEvent( CORE::CDynamicBuffer& out                                     ,
                             const CORE::CString& data                                     ,
                             const CORE::CString& eventName = CORE::CString::Empty         );

    /** As above for text that is not a CString, for example JSON written into a buffer */
    static bool AppendEvent( CORE::CDynamicBuffer& out                                     ,
                             const char* data                                              ,
                             UInt32 dataSize                                               ,
                             const CORE::CString& eventName = CORE::CString::Empty         );

    /**
     *  Appends a comment: a line that starts with a colon, which a client ignores. Keeps idle streams alive through
     *  intermediaries. Every line of the text becomes a comment line of its own; empty text gives ":\n".
     */
    static void AppendComment( CORE::CDynamicBuffer& out, const CORE::CString& text = CORE::CString::Empty );

    private:

    static void AppendField( CORE::CDynamicBuffer& out, const char* fieldPrefix, UInt32 prefixSize, const char* data, UInt32 dataSize );

    CSseEventWriter( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CSSEEVENTWRITER_H ? */
