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

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H
#define GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#include "gucefASSETSTORE_CAssetConversionRecord.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONRECORD_H ? */

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
 *  The outcome of the conversion of a pack: what a hook gives back from CIAssetConverterHook::ConvertPack(), and the
 *  result data of CPrimaryAssetStore::ConvertPackAsync() and ConvertPendingPacksAsync().
 *
 *  'status' is OK when the conversion ran and its report has no errors, REFUSED when the converter would not convert
 *  the pack (the message says why: this is no failure), FAILED when the conversion ran and the report has errors,
 *  CANCELLED when the job was asked to stop, and another status (IO_ERROR, UNAVAILABLE, ...) when the conversion could
 *  not run at all. 'record' holds the profile, the versions of the converter parts, the counts and the report path;
 *  'state' is the state the pack has afterwards. For a run over several packs the counts of packs say how it went.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetConversionOutcome : public CORE::CICloneable
{
    public:

    CORE::CString jobId;
    CORE::CString packId;
    TAssetStoreStatus status;
    TAssetPackState state;
    CORE::CString message;
    CORE::CString::StringVector problems;
    CAssetConversionRecord record;
    UInt32 packsConverted;                          /**< of a run over several packs */
    UInt32 packsRefused;
    UInt32 packsFailed;

    CAssetConversionOutcome( void );
    virtual ~CAssetConversionOutcome() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONOUTCOME_H ? */
