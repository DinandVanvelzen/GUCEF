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


#ifndef GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H
#define GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#include "gucefASSETSTORE_CAssetRelationRecord.h"
#define GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */

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
 *  What a converter hook is asked to index (CIAssetConverterHook::BuildPackIndex()): the pack with its licence, the VFS
 *  folder that holds its original files and the VFS folder of its intermediate area, where the document
 *  "pack-index.json" (and its shards) of the pack is, or is to be written (CPackStorageLayout::GetPackIndexPath()).
 *
 *  The hook builds the document (work once: when the stored document is fresh nothing is built, see the pack index design
 *  section 4) and answers where it is. 'derive' asks for a document made from what the catalogue knows of the pack ('assets',
 *  with their typed properties, relations and conversion) instead of from the files, for packs ingested before the index
 *  existed: no file of the pack is read then. 'reindex' and 'reanalyse' are the redo requests of the design (section 5).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackIndexRequest
{
    public:

    CAssetPackRecord pack;
    CORE::CString sourceVfsPath;                    /**< VFS folder with the original files */
    CORE::CString sourcePrefix;                     /**< the start of the original paths of the assets that sourceVfsPath stands for ("original/") */
    CORE::CString intermediateVfsFolder;            /**< VFS folder of the intermediate area of the pack: where pack-index.json is */
    CORE::CString cookedVfsFolder;                  /**< VFS folder of the cooked area, empty when there is none */
    TAssetRecordVector assets;                      /**< the assets of the catalogue: given for 'derive' only */
    GUCEF::vector< CAssetRelationRecord > relations;    /**< the relations of the catalogue from the assets of the pack: given for 'derive' only */
    bool derive;                                    /**< make the document from the assets of the catalogue, read no file of the pack */
    bool reindex;                                   /**< build the structure and the grouping again */
    bool reanalyse;                                 /**< read the files again for statistics, bounds and references */
    bool analyse;                                   /**< also the analysis level (headers of the files); the structure level alone when false */
    CIAssetConversionObserver* observer;            /**< cancel and progress, not owned, may be GUCEF_NULL */

    CAssetPackIndexRequest( void );
    ~CAssetPackIndexRequest();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKINDEXREQUEST_H ? */
