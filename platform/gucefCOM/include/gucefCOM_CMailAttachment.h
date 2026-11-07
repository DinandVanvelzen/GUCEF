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

#ifndef GUCEF_COM_CMAILATTACHMENT_H
#define GUCEF_COM_CMAILATTACHMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  A file that travels with a message, or a part of a message that is not its text. The data is the decoded
 *  content (what the file holds), not the base64 text.
 */
class GUCEF_COM_PUBLIC_CPP CMailAttachment
{
    public:

    typedef GUCEF::vector< CMailAttachment > TMailAttachmentVector;

    CORE::CString fileName;             /**< as the sender wrote it, UTF-8. Do not use it as a path: GetSafeFileName() */
    CORE::CString contentType;          /**< for example application/octet-stream */
    CORE::CString contentId;            /**< without the angle brackets; set for inline content that the HTML refers to as cid: */
    bool isInline;                      /**< Content-Disposition inline */
    CORE::CDynamicBuffer data;

    CMailAttachment( void );

    CMailAttachment( const CORE::CString& fileName, const CORE::CString& contentType, const void* data, UInt32 dataSize );

    CMailAttachment( const CMailAttachment& src );

    ~CMailAttachment();

    CMailAttachment& operator=( const CMailAttachment& src );

    /**
     *  The name for use as the name of a file in a directory: without any directory part (both separators),
     *  control characters and the characters Windows refuses (:*?"<>|), without leading and trailing dots and
     *  spaces, at most 200 bytes, and not a reserved device name. 'fallback' when nothing is left.
     */
    CORE::CString GetSafeFileName( const CORE::CString& fallback = "attachment" ) const;

    /** The same cleaning for any name */
    static CORE::CString MakeSafeFileName( const CORE::CString& name, const CORE::CString& fallback = "attachment" );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILATTACHMENT_H ? */
