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

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H
#define GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

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
 *  The revisions, counts and scalars of a published table, the value of the reserved grid key "#meta". A replica
 *  counts the rows of the newest page revision against changedRowCount to know that an update arrived whole.
 */
class GUCEF_DRN_EXPORT_CPP CDRNPublishedTableMeta
{
    public:

    static const UInt8 EncodingVersion = 1;

    UInt64 pageRevision;            /**< increases with every published version of the row set */
    UInt64 sourceVersion;           /**< the version of the data the rows were built from, the application's meaning */
    UInt32 rowCount;                /**< rows of this revision */
    UInt32 changedRowCount;         /**< rows written or removed in this revision */
    CORE::CValueList scalars;       /**< header scalars of the page */

    CDRNPublishedTableMeta( void );

    void Encode( CORE::CVariantStream& stream ) const;
    bool Decode( CORE::CVariantStream& stream, bool& unsupported );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLEMETA_H ? */
