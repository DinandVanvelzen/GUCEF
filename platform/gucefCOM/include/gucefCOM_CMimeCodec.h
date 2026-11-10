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

#ifndef GUCEF_COM_CMIMECODEC_H
#define GUCEF_COM_CMIMECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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
 *  The encodings of Internet mail as static functions: base64 with line wrapping and a lenient decoder,
 *  quoted-printable (RFC 2045), encoded words (RFC 2047), header parameters including the continuations and
 *  character sets of RFC 2231, header folding (RFC 5322), the character sets that mail carries, dates and ids.
 *
 *  Text that comes out of a decoder is always valid UTF-8: bytes that are not valid in the character set
 *  are replaced by U+FFFD. Nothing here throws or reads outside of the data it is given. Decoders that grow a
 *  result take a maximum size and fail when it would be exceeded.
 */
class GUCEF_COM_PUBLIC_CPP CMimeCodec
{
    public:

    struct SParam
    {
        CORE::CString name;     /**< lower case, without the *n* of RFC 2231 */
        CORE::CString value;    /**< decoded, UTF-8 */
    };
    typedef struct SParam TParam;
    typedef GUCEF::vector< TParam > TParamVector;

    /** The longest line a header may have on the wire, without the CR LF (RFC 5322 section 2.1.1) */
    static const UInt32 MaxHeaderLineLength = 998;

    /** The line length the composer folds at */
    static const UInt32 PreferredHeaderLineLength = 78;

    /** The length of the lines of base64 and quoted-printable bodies */
    static const UInt32 EncodedBodyLineLength = 76;

    /** Appends the base64 of the data in lines of 'lineLength' characters, each line ended with CR LF (also the last) */
    static void AppendBase64Wrapped( CORE::CDynamicBuffer& out, const void* data, UInt32 size, UInt32 lineLength = EncodedBodyLineLength );

    /**
     *  Decodes base64 that may have white space and line ends anywhere, missing padding and stray characters.
     *  Stops at the first '=' that ends the data. 'hadErrors' is set when characters were skipped that are not base64.
     *  @return false when the result would exceed 'maxSize' (the buffer then holds what was decoded until then)
     */
    static bool DecodeBase64Lenient( const char* text, UInt32 size, CORE::CDynamicBuffer& out, UInt32 maxSize, bool* hadErrors = GUCEF_NULL );

    /** Appends quoted-printable of text: CR LF and LF are line ends and become CR LF, lines are at most 76 characters, space and tab at the end of a line are encoded */
    static void AppendQuotedPrintable( CORE::CDynamicBuffer& out, const char* text, UInt32 size );

    /**
     *  Decodes quoted-printable. With 'qEncoding' (the Q encoding of encoded words) an underscore is a space.
     *  An '=' that is not followed by two hex digits or a line end is kept as it is. Soft line breaks are removed.
     *  @return false when the result would exceed 'maxSize'
     */
    static bool DecodeQuotedPrintable( const char* text, UInt32 size, CORE::CDynamicBuffer& out, bool qEncoding, UInt32 maxSize );

    /** Whether all bytes are below 0x80 */
    static bool IsAscii( const char* data, UInt32 size );

    /** Whether the text has CR, LF or NUL, which must never be written to a protocol line or a header as they are */
    static bool HasLineBreakOrNul( const CORE::CString& text );

    /** Whether the text has a control character (below 0x20 or 0x7F) */
    static bool HasControlCharacter( const CORE::CString& text );

    /** Valid UTF-8 from bytes that may not be: invalid sequences (overlong, surrogates, truncated) become U+FFFD */
    static CORE::CString SanitizeUtf8( const char* data, UInt32 size );

    /**
     *  Converts text of the character set (US-ASCII, UTF-8, ISO-8859-1, ISO-8859-15, Windows-1252, UTF-16, UTF-16LE,
     *  UTF-16BE, case insensitive, any RFC 2231 language suffix is ignored) to UTF-8. A character set that is not
     *  known is treated as UTF-8 with the invalid sequences replaced and 'lossy' is set.
     */
    static CORE::CString ToUtf8( const CORE::CString& charset, const char* data, UInt32 size, bool& lossy );

    /** Whether the text can go to a header as it is: printable ASCII only and no "=?" that could be read as an encoded word */
    static bool IsPlainHeaderText( const CORE::CString& text );

    /**
     *  Text for an unstructured header (a subject) or a phrase: unchanged when IsPlainHeaderText(), otherwise
     *  the whole text as RFC 2047 encoded words ("=?UTF-8?B?...?=", each at most 75 characters, separated by a
     *  space). Control characters are encoded too, so the result never has one.
     */
    static CORE::CString EncodeHeaderText( const CORE::CString& utf8Text );

    /** The text with its encoded words decoded (B and Q, any supported character set, white space between two encoded words dropped) */
    static CORE::CString DecodeHeaderText( const CORE::CString& wireText );

    /** The value of a header that was split over lines: the line ends in front of white space removed. Other CR and LF are removed too */
    static CORE::CString Unfold( const char* data, UInt32 size );

    /**
     *  Appends "Name: value" folded at white space to PreferredHeaderLineLength and ended with CR LF.
     *  @return false (nothing appended) when the name is not a valid field name, the value has CR, LF or NUL,
     *          or a line cannot be folded below MaxHeaderLineLength
     */
    static bool AppendHeaderField( CORE::CDynamicBuffer& out, const CORE::CString& name, const CORE::CString& wireValue );

    /** Whether the name is a valid field name: printable ASCII except the colon, not empty */
    static bool IsValidHeaderName( const CORE::CString& name );

    /**
     *  Takes a structured value like 'text/plain; charset="utf-8"; name*=utf-8''m%C3%BCller.txt' apart: the value
     *  in front of the first semicolon and the parameters. Quoted strings, RFC 2231 extended values and continuations
     *  are decoded. Parameter names are lower case.
     */
    static void ParseValueWithParameters( const CORE::CString& wireValue, CORE::CString& value, TParamVector& params );

    /** The value of the parameter, empty when there is none */
    static const CORE::CString& FindParameter( const TParamVector& params, const CORE::CString& lowerCaseName );

    /**
     *  A parameter for a header: name=token, name="quoted" or, for text with non ASCII or control characters,
     *  the RFC 2231 form name*=utf-8''... (in sections name*0*=, name*1*= when it is long), joined with "; "
     */
    static CORE::CString FormatParameter( const CORE::CString& name, const CORE::CString& utf8Value );

    /** An RFC 5322 date: "Mon, 02 Nov 2020 10:11:12 +0000" */
    static CORE::CString FormatDate( const CORE::CDateTime& date );

    /** Reads an RFC 5322 date (also the obsolete forms: two digit years, zone names, comments) */
    static bool ParseDate( const CORE::CString& text, CORE::CDateTime& date );

    /** A message id of the form <random@domain>, the domain falls back to "localhost.localdomain" when it is not a usable host name */
    static CORE::CString GenerateMessageId( const CORE::CString& domain );

    /** A random multipart boundary that needs no quoting */
    static CORE::CString GenerateBoundary( void );

    /** Whether the id is "<local@domain>" in a safe subset: no spaces, control characters, angle brackets or quotes inside */
    static bool IsValidMessageId( const CORE::CString& messageId );

    private:

    CMimeCodec( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMIMECODEC_H ? */
