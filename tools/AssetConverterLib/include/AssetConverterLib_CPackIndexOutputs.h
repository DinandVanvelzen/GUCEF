/*
 *  AssetConverterLib: the converted outputs of a resource of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H
#define ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXOUTPUTFILE_H
#include "AssetConverterLib_CPackIndexOutputFile.h"
#define ASSETCONVERTERLIB_CPACKINDEXOUTPUTFILE_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXOUTPUTFILE_H ? */

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
 *  { intermediate: { path, sha256, size, cacheKey, sidecar: { path, sha256 } }, cooked: [ { profile, kind, path, sha256, size } ], lossy }.
 *  The index copies the results for search; it never decides whether a conversion is fresh (CPackWorkGate asks the cache entry).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexOutputs
{
    public:

    typedef GUCEF::vector< CPackIndexOutputFile > TFileVector;

    CPackIndexOutputFile intermediate;
    CORE::CString cacheKey;
    CPackIndexOutputFile sidecar;
    TFileVector cooked;
    UInt32 lossy;

    CPackIndexOutputs( void );

    bool IsEmpty( void ) const;
    bool operator==( const CPackIndexOutputs& other ) const;
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXOUTPUTS_H ? */

