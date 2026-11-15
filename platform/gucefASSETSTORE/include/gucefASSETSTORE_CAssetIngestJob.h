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

#ifndef GUCEF_ASSETSTORE_CASSETINGESTJOB_H
#define GUCEF_ASSETSTORE_CASSETINGESTJOB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#include "gucefASSETSTORE_CAssetIngestRequest.h"
#define GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTREQUEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#include "gucefASSETSTORE_CAssetRelationRecord.h"
#define GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRELATIONRECORD_H ? */
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

typedef GUCEF::vector< CAssetRelationRecord > TAssetRelationRecordVector;

/**
 *  The state of one ingest of the primary store, kept as its manifest ("ingest.json" in
 *  "<location>/staging/<ingest id>/") after every step, so that an ingest that failed resumes from the step that
 *  failed with everything the earlier steps found (section 8): the request, the last step that completed, the failure,
 *  where the files are now (contentRoot, with contentPrefix the part of the asset paths that is not below it), the
 *  pack record, the asset records with their digests and properties, the relations and the problems.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetIngestJob : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString ManifestFileName;    /**< "ingest.json" */

    CORE::CString ingestId;
    CAssetIngestRequest request;
    TAssetIngestStep lastStep;                      /**< the last step that completed */
    TAssetIngestStep failedStep;
    TAssetStoreStatus failureStatus;
    CORE::CString failureMessage;
    CORE::CString locationId;
    CORE::CString stagingFolder;                    /**< VFS folder of the manifest and of copies */
    CORE::CString contentRoot;                      /**< VFS folder where the files of the pack are now */
    CORE::CString contentPrefix;                    /**< the start of the asset paths that contentRoot stands for: "original/" once placed */
    bool inPlace;                                   /**< the files stay where they were found */
    bool contentInStaging;                          /**< the files are in the staging area and move when placed */
    CORE::CString::StringVector archives;           /**< archive files of the pack (relative), whose content was taken in */
    CAssetPackRecord pack;
    TAssetRecordVector assets;
    TAssetRelationRecordVector relations;
    CORE::CString::StringVector problems;
    Int64 startedAtMs;

    CAssetIngestJob( void );
    virtual ~CAssetIngestJob() GUCEF_VIRTUAL_OVERRIDE;

    /** The VFS path of the manifest */
    CORE::CString GetManifestPath( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETINGESTJOB_H ? */
