/*
 *  assetpluginUNREAL: The 48 byte record of an editor bulk data payload that refers to the package trailer
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#include "assetpluginUNREAL_CUnrealPayloadReference.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#include "assetpluginUNREAL_CUnrealPackageTrailer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#include "assetpluginUNREAL_CUnrealVersionContext.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALVERSIONCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A record as it is stored inside the native data of an export: flags, the id of the bulk data, the hash of the raw
 *  payload and the raw size, 48 bytes. With the flag "stored in package trailer" the hash is the key of the payload
 *  table of the trailer.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealEditorBulkData
{
    public:

    static const UInt32 FLAG_STORED_IN_PACKAGE_TRAILER = 0x200;

    UInt32 flags;
    CUnrealGuid bulkDataId;
    UInt8 payloadId[ 20 ];
    Int64 payloadSize;

    CUnrealEditorBulkData( void );

    /** Reads the record; packages older than UE5 version 1002 (the payload table of the package trailer) are refused with the reader error state set */
    bool Read( CUnrealArchiveReader& reader, const CUnrealVersionContext& versions );

    /** Resolves the record into the place of the bytes; the trailer may be NULL for a package without one */
    void ToReference( const CUnrealPackageTrailer* trailer, CUnrealPayloadReference& reference ) const;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALEDITORBULKDATA_H ? */
