/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#define GUCEF_WEB_CHTTPRESPONSEDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

class GUCEF_WEB_PUBLIC_CPP CHttpResponseData : public CORE::CICloneable
{
    public:

    typedef CORE::CString::StringVector                TStringVector;
    typedef GUCEF::map< CORE::CString, CORE::CString >   TStringMap;

    TStringVector allowedMethods;
    CString cacheability;
    CString eTag;
    CString lastModified;
    CORE::CDynamicBuffer content;
    CString contentEncoding;
    int statusCode;
    CString contentType;
    CString location;
    TStringVector acceptedTypes;
    bool keepConnectionsAlive;
    TStringMap otherHeaders;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Writes the response: status line, header fields and the content, with a Content-Length.
     *  A response whose header fields would break the framing of the message is refused and nothing is written:
     *  a field name that is not a token, or a value or a content type, location, ETag, cache control or last
     *  modified text with a CR, LF or NUL in it. That keeps text taken from a request from adding header fields
     *  or responses of its own (response splitting).
     *  @return false when the response was refused
     */
    bool Serialize( CORE::CDynamicBuffer& outputBuffer ) const;

    /**
     *  As above with the Connection field decided by the caller, the server that knows whether the connection stays:
     *  a Connection field among the other header fields is not written then
     */
    bool Serialize( CORE::CDynamicBuffer& outputBuffer, bool keepConnectionAlive ) const;

    /**
     *  Writes the status line and the header fields of a response whose body follows as a stream that ends with the
     *  connection: Connection: close, no Content-Length, no Transfer-Encoding. The content is not written.
     *  The header fields are checked as in Serialize().
     */
    bool SerializeStreamHead( CORE::CDynamicBuffer& outputBuffer ) const;

    /** Whether the header fields can be written as they are, see Serialize() */
    bool HasSafeHeaderFields( void ) const;

    /** The reason phrase of a status code, "OK" for 200; empty for a code it does not know */
    static const char* GetReasonPhrase( int statusCode );

    /**
     *  Resets all fields to their defaults so the object can be reused for another response.
     *  The content buffer keeps its allocated memory.
     */
    void Clear( void );

    CHttpResponseData( void );
    CHttpResponseData( const CHttpResponseData& src );
    virtual ~CHttpResponseData();

    private:

    /** connectionMode: -1 as the fields say, 0 close, 1 keep-alive, 2 a stream that ends with the connection */
    bool SerializeHead( CString& head, int connectionMode ) const;

    bool SerializeWithConnectionMode( CORE::CDynamicBuffer& outputBuffer, int connectionMode ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */
