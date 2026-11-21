/*
 *  assetpluginUNREAL: The interface of the provider that decompresses Oodle blocks
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {
class CConversionReport;
}; /* namespace ASSETCONVERTER */
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the readers need of an Oodle decoder: decompress one block (one Kraken family stream, which is a block of a compressed
 *  buffer) whose compressed and decompressed sizes are known. The only implementation is COodleToolProvider, which asks an
 *  external tool in a child process; a native decoder could replace it without a change to the readers.
 *
 *  A provider belongs to one load session and one thread at a time. Reading is synchronous on the calling thread.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CIUnrealOodleProvider
{
    public:

    virtual ~CIUnrealOodleProvider() {}

    /** Whether blocks can be decompressed; false when the tool is missing (see GetUnavailableReason()) */
    virtual bool IsAvailable( void ) = 0;

    /** Why IsAvailable() is false: for the reports of the assets that need the provider, "tool not found at ..." */
    virtual CORE::CString GetUnavailableReason( void ) = 0;

    /**
     *  Decompresses one block into dest, which has room for exactly decompressedSize bytes.
     *  @param toolCode the exit code or answer code of the tool for a failed block (0 when the failure was not the tool's)
     *  @param error the reason of a failure, with the code and the message of the tool
     */
    virtual bool DecodeBlock( const void* compressed     ,
                              UInt32 compressedSize      ,
                              UInt32 decompressedSize    ,
                              void* dest                 ,
                              CORE::CString& error       ,
                              Int32& toolCode            ) = 0;

    /** Where the request to cancel the load of the asset is polled; the report of the asset that is converted. NULL for none */
    virtual void SetCancellationSource( const ASSETCONVERTER::CConversionReport* report ) = 0;

    /** Ends the decompression that is in progress and any child process; later blocks start it again */
    virtual void Stop( void ) = 0;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CIUNREALOODLEPROVIDER_H ? */
