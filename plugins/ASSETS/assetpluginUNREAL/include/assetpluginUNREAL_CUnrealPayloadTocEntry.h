/*
 *  assetpluginUNREAL: One entry of the payload table of a package trailer
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H

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
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPayloadTocEntry
{
    public:

    UInt8 identifier[ 20 ];     /**< hash of the raw payload; the key the bulk data records refer to */
    Int64 offsetInFile;         /**< relative to the end of the trailer header */
    UInt64 compressedSize;
    UInt64 rawSize;
    UInt32 flags;
    UInt8 accessMode;           /**< 0: stored in this package; other values are payloads that live elsewhere */

    CUnrealPayloadTocEntry( void );

    bool HasIdentifier( const UInt8* hash ) const;

    CORE::CString IdentifierToString( void ) const;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADTOCENTRY_H ? */
