/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNNAMESPACELISTPAGE_H
#define GUCEF_DRN_CDRNNAMESPACELISTPAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One page of a directory listing (LIST answer). A page is consistent within itself; the pages of one listing are not
 *  a snapshot of the directory (it may change between pages), the revision tells whether it did.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceListPage
{
    public:

    typedef GUCEF::vector< CDRNNamespaceEntry > TEntryVector;

    TEntryVector entries;
    CORE::CString nextCursor;       /**< empty: this was the last page; else the cursor to ask the next page with */
    CORE::CString dirRevision;      /**< the change token of the directory, empty when the serving side has none (nothing then answers UNCHANGED) */
    bool unchanged;                 /**< the answer to ifRevisionNot: the directory is still that revision, no entries follow */

    CDRNNamespaceListPage( void );

    void Clear( void );

    bool IsLast( void ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACELISTPAGE_H ? */
