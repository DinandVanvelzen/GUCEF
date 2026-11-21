/*
 *  assetpluginUNREAL: The header of a legacy bulk data payload (packages of UE4 and early UE5)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H

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
 *  The bulk data header of the engine versions before payload trailers: flags, element count, size on disk and the
 *  offset in the file (relative to the bulk data start offset of the summary unless the flags say otherwise). A payload
 *  that is flagged inline follows the header directly; the caller skips it with the stored size of the reference.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealLegacyBulkData
{
    public:

    static const UInt32 FLAG_PAYLOAD_AT_END_OF_FILE     = 0x00001;
    static const UInt32 FLAG_COMPRESSED_ZLIB            = 0x00002;
    static const UInt32 FLAG_UNUSED                     = 0x00020;
    static const UInt32 FLAG_FORCE_INLINE_PAYLOAD       = 0x00040;
    static const UInt32 FLAG_PAYLOAD_IN_SEPERATE_FILE   = 0x00100;
    static const UInt32 FLAG_SIZE_64BIT                 = 0x02000;
    static const UInt32 FLAG_BAD_DATA_VERSION           = 0x08000;
    static const UInt32 FLAG_NO_OFFSET_FIXUP            = 0x10000;

    UInt32 flags;
    Int64 elementCount;
    Int64 sizeOnDisk;
    Int64 offsetInFile;

    CUnrealLegacyBulkData( void );

    bool Read( CUnrealArchiveReader& reader, const CUnrealVersionContext& versions );

    /** The place of the bytes; bulkDataStartOffset is the one of the summary, packageSize bounds the range */
    void ToReference( Int64 bulkDataStartOffset, UInt64 headerEnd, UInt64 packageSize, CUnrealPayloadReference& reference ) const;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */
