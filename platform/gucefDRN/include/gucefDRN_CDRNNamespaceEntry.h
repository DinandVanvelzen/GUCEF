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

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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
 *  One entry of a share as the namespace operations describe it: what a STAT answers per path and what a LIST answers
 *  per entry. Names are relative to the share ("a/b.bin"); the root of a share is the empty name.
 *
 *  The revision of a file is the revision OPEN will check (CDRNResourceInfo::revision); the revision of a directory is
 *  its change token (empty when the serving side has none). A revision is opaque: only compared for equality.
 */
class GUCEF_DRN_EXPORT_CPP CDRNNamespaceEntry
{
    public:

    CORE::CString name;                 /**< relative to the share, '/' separated; for a STAT answer the path asked for */
    UInt8 kind;                         /**< TDRNNamespaceEntryKind */
    UInt64 size;                        /**< bytes of a file, 0 for a directory */
    UInt64 modifiedUtcMs;               /**< milliseconds since the Unix epoch in UTC, 0 when unknown */
    UInt32 flags;                       /**< DRN_NS_ENTRYFLAG_* */
    CORE::CString revision;
    bool hasSha256;                     /**< only when the provider knew it already: it is never computed for a STAT or LIST */
    UInt8 sha256[ 32 ];
    TDRNResourceStatus status;          /**< STAT answers: how the path fared (OK, NOT_FOUND, FORBIDDEN, UNCHANGED, ...); the entry is filled when OK */

    CDRNNamespaceEntry( void );

    void Clear( void );

    bool IsDirectory( void ) const;
    bool IsFile( void ) const;

    /** The largest revision a serving side sends, in bytes */
    static const UInt32 MaxRevisionBytes = 256;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */
