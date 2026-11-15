/*
 *  AssetConverterLib: SHA-256 content hashes of files and buffers
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

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#define ASSETCONVERTERLIB_CCONTENTHASH_H

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

/**
 *  The SHA-256 (gucefCORE_CSha256) of files of the VFS, of buffers and of texts, as 64 lower case hex digits: the
 *  parts of the cache keys and the names of baked textures. Files are read in pieces, so a large file is not held in
 *  memory for its hash.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CContentHash
{
    public:

    /** The hash of a file of the VFS; empty when it cannot be read */
    static CORE::CString HashFile( const CORE::CString& vfsPath );

    static CORE::CString HashBuffer( const void* data, UInt32 size );

    /** The hash of the bytes of the text (UTF-8), without a terminating zero */
    static CORE::CString HashText( const CORE::CString& text );

    private:

    CContentHash( void );
    ~CContentHash();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */
