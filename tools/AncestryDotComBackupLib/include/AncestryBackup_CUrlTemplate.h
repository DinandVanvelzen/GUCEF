/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#define ANCESTRYBACKUP_CURLTEMPLATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <vector>
#include <utility>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The parts of a URL that matter when working out which kind of request it is */
struct SUrlTemplate
{
    CORE::CString scheme;
    CORE::CString host;                 /**< lower case, including the port if there is one */
    CORE::CString path;                 /**< as it appeared in the URL */
    CORE::CString pathTemplate;         /**< the path with identifiers replaced by {n}, {guid}, {hex} and {id} */
    std::vector< std::pair< CORE::CString, CORE::CString > > queryParams;   /**< in the order they appeared, values are not decoded */

    SUrlTemplate( void ) : scheme(), host(), path(), pathTemplate(), queryParams() {}

    /** host + path template, what requests of the same kind have in common */
    CORE::CString GetKey( void ) const { return host + pathTemplate; }
};

/**
 *  Reduces URLs to the kind of request they are. The endpoints of a web application are shaped like
 *  /api/tree/1234567/person/89012345/facts, to describe the endpoint instead of one request the
 *  identifiers in the path have to go: /api/tree/{n}/person/{n}/facts
 */
class CUrlTemplate
{
    public:

    /** Splits a URL, fails if it has no scheme */
    static bool Parse( const CORE::CString& url, SUrlTemplate& result );

    /** Replaces identifier like path segments, the segment can have a file extension which is kept */
    static CORE::CString TemplatePath( const CORE::CString& path );

    static CORE::CString TemplateSegment( const CORE::CString& segment );

    /** Is this a segment which varies per object: a number, a GUID, a hash, a generated id */
    static bool LooksLikeId( const CORE::CString& segment );

    /** The segment following the given one when that is an identifier, for example the tree id in /tree/12345/person */
    static CORE::CString FindIdAfterSegment( const CORE::CString& path, const CORE::CString& segmentName );

    /** Upper/lower case hex digits, '%' escapes and '+' are decoded */
    static CORE::CString PercentDecode( const CORE::CString& text );

    /** Escapes a string for use inside a JSON document */
    static CORE::CString EscapeJson( const CORE::CString& text );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */
