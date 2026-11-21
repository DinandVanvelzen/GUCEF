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

#ifndef GUCEF_WEB_CWEBBODYSHAPE_H
#include "gucefWEB_CWebBodyShape.h"
#define GUCEF_WEB_CWEBBODYSHAPE_H
#endif /* GUCEF_WEB_CWEBBODYSHAPE_H ? */

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

/** What the structure of a JSON document looks like, see WEB::CWebBodyShape */
typedef WEB::CWebBodyShape SJsonShape;

/**
 *  Describes the structure of a JSON document: which fields it has and of what type. The tool keeps the values of
 *  the fields that tell how an endpoint pages and of the fields that hold URLs (never of secret fields), because those
 *  are what is needed to learn how to use an endpoint; its report stays on the user's own disk. Other values are left
 *  out: a genealogy document is full of personal data. The work is done by WEB::CWebBodyShape.
 */
class CJsonShape
{
    public:

    static void Describe( const CORE::CDataNode& root, SJsonShape& shape, UInt32 maxDepth = 5, UInt32 maxLines = 150 );

    /** The keys through which APIs tell how paging works: page, offset, limit, next, cursor, total... */
    static bool IsPaginationKey( const CORE::CString& key );

    /** json, array, string, ... */
    static CORE::CString GetTypeName( const CORE::CVariant& value );
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
