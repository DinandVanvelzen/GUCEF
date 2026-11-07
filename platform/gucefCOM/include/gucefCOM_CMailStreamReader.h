/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CMAILSTREAMREADER_H
#define GUCEF_COM_CMAILSTREAMREADER_H

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

#ifndef GUCEF_COM_MAILTYPES_H
#include "gucefCOM_MailTypes.h"
#define GUCEF_COM_MAILTYPES_H
#endif /* GUCEF_COM_MAILTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The receive side of a line oriented mail protocol, without sockets: bytes go in, lines and counted blocks of
 *  bytes come out. SMTP and POP3 only have lines; IMAP has lines in which a literal ({n}) announces n bytes that
 *  follow the line, and the protocol layer asks for them with ExpectBytes().
 *
 *  A line ends at LF, a CR in front of the LF is not part of it; text that is not valid UTF-8 and NUL bytes in a
 *  line are replaced by U+FFFD so that a line is always a valid string. The reader never holds more than a line
 *  of the maximum length (without its end) or the counted block that was asked for: a server that sends a line
 *  that does not end, or a block larger than the maximum, makes it fail with ITEM_OVERFLOW and it stays failed
 *  until Reset().
 */
class GUCEF_COM_PUBLIC_CPP CMailStreamReader
{
    public:

    enum EItem
    {
        ITEM_NONE = 0,      /**< nothing complete yet */
        ITEM_LINE,          /**< a line, without its line end */
        ITEM_BYTES,         /**< the counted block that ExpectBytes() asked for */
        ITEM_OVERFLOW       /**< a limit was exceeded, GetOverflowReason() says which */
    };

    CMailStreamReader( void );

    CMailStreamReader( UInt32 maxLineLength, UInt32 maxBytes );

    void SetLimits( UInt32 maxLineLength, UInt32 maxBytes );

    /** Forgets all data and a failure, keeps the limits */
    void Reset( void );

    /** Adds received bytes. @return false when the reader has failed */
    bool Feed( const void* data, UInt32 size );

    /** The next item. For ITEM_LINE 'line' is set, for ITEM_BYTES 'bytes' */
    EItem Next( CORE::CString& line, CORE::CDynamicBuffer& bytes );

    /**
     *  The next 'count' bytes, whatever they are, are delivered as one block by Next() before any line.
     *  @return false (and the reader fails) when the count is larger than the maximum
     */
    bool ExpectBytes( UInt32 count );

    /**
     *  The bytes up to the line that has only a dot (CR LF "." CR LF, the end of a POP3 multi line response) are delivered
     *  as one block by Next(), as they came: dot stuffing is not undone and the lines are not checked for text.
     *  The terminator is not part of the block. A block of more than 'maxBytes' without terminator makes the reader fail.
     */
    void ExpectDotTerminatedBytes( UInt32 maxBytes );

    bool IsExpectingBytes( void ) const;

    /** Bytes received and not delivered yet */
    UInt32 GetBufferedSize( void ) const;

    bool HasBufferedData( void ) const;

    bool HasFailed( void ) const;

    const CORE::CString& GetOverflowReason( void ) const;

    private:

    void Compact( void );

    private:

    CORE::CDynamicBuffer m_buffer;
    UInt32 m_start;                 /**< where the data that was not delivered starts */
    UInt32 m_scan;                  /**< where the search for a line end continues */
    UInt32 m_maxLineLength;
    UInt32 m_maxBytes;
    UInt32 m_expected;
    bool m_expecting;
    bool m_expectingDot;
    UInt32 m_dotMax;
    bool m_failed;
    CORE::CString m_overflowReason;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILSTREAMREADER_H ? */
