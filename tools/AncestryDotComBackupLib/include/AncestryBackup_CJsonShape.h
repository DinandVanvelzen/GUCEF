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

#ifndef ANCESTRYBACKUP_CJSONSHAPE_H
#define ANCESTRYBACKUP_CJSONSHAPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include <vector>

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

/** What the structure of a JSON document looks like, without its data */
struct SJsonShape
{
    std::vector< CORE::CString > lines;             /**< one line per field: path: type [= sample] */
    std::vector< CORE::CString > paginationHints;   /**< fields that look like they are about paging: path = value */
    std::vector< CORE::CString > urlFields;         /**< fields holding a URL or an absolute path: path = value */
    bool truncated;                                 /**< the document had more fields than the line limit allowed */

    SJsonShape( void ) : lines(), paginationHints(), urlFields(), truncated( false ) {}
};

/**
 *  Describes the structure of a JSON document: which fields it has and of what type. Values are left out,
 *  except for fields that tell how an endpoint pages and fields that hold URLs, because those are what is
 *  needed to learn how to use an endpoint. A genealogy document is full of personal data which does not
 *  belong in a report that gets shared.
 */
class CJsonShape
{
    public:

    static void Describe( const CORE::CDataNode& root, SJsonShape& shape, UInt32 maxDepth = 5, UInt32 maxLines = 150 );

    /** The keys through which APIs tell how paging works: page, offset, limit, next, cursor, total... */
    static bool IsPaginationKey( const CORE::CString& key );

    /** json, array, string, ... */
    static CORE::CString GetTypeName( const CORE::CVariant& value );

    private:

    static void Walk( const CORE::CDataNode& node, const CORE::CString& path, UInt32 depth, UInt32 maxDepth, UInt32 maxLines, SJsonShape& shape );

    static void AddLine( const CORE::CString& line, UInt32 maxLines, SJsonShape& shape );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CJSONSHAPE_H ? */
