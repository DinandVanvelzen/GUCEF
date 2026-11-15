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

#ifndef GUCEF_ASSETSTORE_CASSETJOBENTRY_H
#define GUCEF_ASSETSTORE_CASSETJOBENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
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
 *  One job of the asset store service: a download, an ingest, a conversion, a pipeline run, a verification, an upload or a
 *  fetch, with its state, its progress and the actions that apply to it now. The id stays with the job for as long as the
 *  service lists it.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetJobEntry : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString jobId;
    CORE::CString kind;                   /**< "download", "ingest", "conversion", "pipeline", "verification", "upload", "fetch" */
    CORE::CString name;                   /**< what the person knows it by */
    CORE::CString detail;                 /**< a second line: where it comes from, what it is for */
    TAssetJobState state;
    CORE::CString stateText;              /**< the state in words of the service ("planning", an ingest step); empty for the standard word */
    CORE::CString message;                /**< why it failed, or a note; never holds a secret */
    TAssetStoreStatus status;
    CORE::CString folder;                 /**< VFS folder of the result, empty for none */
    UInt64 totalBytes;                    /**< 0 when not known */
    UInt64 doneBytes;
    Float32 fraction;                     /**< the progress from 0 to 1 of a job that is not counted in bytes; -1 to take it from the bytes */
    UInt64 bytesPerSecond;                /**< 0 when not known */
    UInt64 secondsLeft;                   /**< 0 when not known */
    UInt32 itemsDone;                     /**< assets or packs done, of a job that counts them */
    UInt32 itemsTotal;
    CORE::CString providerId;
    CORE::CString itemId;
    CORE::CString packId;
    bool forIngest;                       /**< a download for the primary store */
    bool ingestRequested;                 /**< a download for the primary store whose ingest the service started */
    CORE::CString::StringVector files;    /**< VFS paths of the files of a download */
    CORE::CString::StringSet actions;     /**< the names of the actions that apply now (AssetJobActionToString()) */
    Int64 createdAtMs;                    /**< UTC milliseconds since the Unix epoch */
    Int64 updatedAtMs;
    CORE::CString ownerDevice;            /**< the device that started it, empty in the process of the store */

    CAssetJobEntry( void );
    virtual ~CAssetJobEntry() GUCEF_VIRTUAL_OVERRIDE;

    /** Every member back to its default */
    void Clear( void );

    /** Whether the job is over: done, failed or cancelled */
    bool IsEnded( void ) const;

    /** Whether the action applies to the job now */
    bool IsActionAvailable( TAssetJobAction action ) const;

    void SetActionAvailable( TAssetJobAction action, bool available );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

typedef GUCEF::vector< CAssetJobEntry > TAssetJobEntryVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETJOBENTRY_H ? */
