/*
 *  AssetConverterLib: reads the size of a picture from the first bytes of its file
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H
#define ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The header probe of the analysis level: width, height, channels and bit depth of a picture from its header, without decoding it */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexImageProbe
{
    public:

    /** Whether the probe knows the format of the extension (png, jpg, jpeg, bmp, gif, tga, dds) */
    static bool IsSupported( const CORE::CString& lowerExtension );

    /** Reads at most the first 64 KiB of the file; false when the file cannot be read or the header is not that of the format */
    static bool Probe( const CORE::CString& vfsPath, const CORE::CString& lowerExtension, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitDepth );

    /** The same for bytes already read: the testable part */
    static bool ProbeBytes( const UInt8* bytes, UInt32 size, const CORE::CString& lowerExtension, UInt32& width, UInt32& height, UInt32& channels, UInt32& bitDepth );

    private:

    CPackIndexImageProbe( void );
    ~CPackIndexImageProbe();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXIMAGEPROBE_H ? */

