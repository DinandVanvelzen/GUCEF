/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */
#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the indexing of a pack of the store (CPrimaryAssetStore::IndexPack()) is asked to do beyond the default, which is
 *  "work once": use the stored document when it is there and fresh, else build one with the converter hook, and load it into
 *  the catalogue. 'force' loads the document again also when the catalogue holds it. 'reindex' and 'reanalyse' are the redo
 *  requests of the pack index design (section 5.1): the hook builds the structure and the grouping again, or reads the files
 *  again for the statistics. 'derive' makes the document from the rows of the catalogue and reads no file of the pack (for
 *  packs ingested before the index existed, and for packs whose originals are not there any more). 'analyse' asks for the
 *  analysis level as well as the structure level.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexOptions
{
    public:

    bool force;
    bool reindex;
    bool reanalyse;
    bool derive;
    bool analyse;
    bool verify;                            /**< ask the converter whether the stored document is still fresh (its gate lists the pack and compares), instead of trusting the loaded one */
    bool allowBuild;                        /**< the hook may build a document that is missing or stale; the ingest turns it off: it takes a document that is there and leaves the building to the job index-pack */
    bool notify;                            /**< raise PackIndexUpdatedEvent when rows were written; the ingest turns it off and lets the service say it after the pack was added */
    CIAssetConversionObserver* observer;    /**< cancel and progress, not owned, may be GUCEF_NULL */

    CAssetPackIndexOptions( void );
    ~CAssetPackIndexOptions();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXOPTIONS_H ? */
