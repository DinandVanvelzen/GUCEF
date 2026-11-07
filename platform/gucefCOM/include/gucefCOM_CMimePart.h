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

#ifndef GUCEF_COM_CMIMEPART_H
#define GUCEF_COM_CMIMEPART_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CEMAILHEADERS_H
#include "gucefCOM_CEMailHeaders.h"
#define GUCEF_COM_CEMAILHEADERS_H
#endif /* GUCEF_COM_CEMAILHEADERS_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

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
 *  One entity of a MIME message (RFC 2045): the whole message, or a part of a multipart. The tree that
 *  CMimeParser makes of the bytes of a message; CEMailMessage is made from it. A part that is a multipart has
 *  children, any other part has a body, already decoded from its transfer encoding.
 */
class GUCEF_COM_PUBLIC_CPP CMimePart
{
    public:

    CEMailHeaders headers;
    CORE::CString contentType;                  /**< lower case without parameters, "text/plain" when the part names none */
    CMimeCodec::TParamVector contentTypeParams;
    CORE::CString transferEncoding;             /**< lower case, empty for none */
    CORE::CString disposition;                  /**< "attachment", "inline" or empty */
    CMimeCodec::TParamVector dispositionParams;
    CORE::CString contentId;                    /**< without the angle brackets */
    CORE::CDynamicBuffer body;                  /**< decoded content of a part that is not a multipart */
    GUCEF::vector< CMimePart* > children;      /**< owned */

    CMimePart( void );

    CMimePart( const CMimePart& src );

    ~CMimePart();

    CMimePart& operator=( const CMimePart& src );

    /** Takes the headers apart: content type, parameters, transfer encoding, disposition, content id */
    void ApplyHeaders( void );

    bool IsMultipart( void ) const;

    /** The type of the part starts with 'text/' */
    bool IsText( void ) const;

    /** The charset parameter, empty when the part has none */
    const CORE::CString& GetCharset( void ) const;

    /** The boundary parameter of a multipart */
    const CORE::CString& GetBoundary( void ) const;

    /** The filename of the disposition, else the name of the content type, UTF-8, empty when there is none */
    CORE::CString GetFileName( void ) const;

    /** Appends a new empty child and returns it */
    CMimePart* AddChild( void );

    void Clear( void );

    private:

    void CopyFrom( const CMimePart& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMIMEPART_H ? */
