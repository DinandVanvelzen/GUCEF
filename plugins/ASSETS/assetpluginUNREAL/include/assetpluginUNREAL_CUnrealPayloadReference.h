/*
 *  assetpluginUNREAL: Where the bytes of a payload are
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H

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

/**
 *  The answer to "where is this payload": inline in the package (a range), in the package trailer (a range found through
 *  the payload table), or not available (it lives in another file or outside the package; the reason says which).
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPayloadReference
{
    public:

    enum EKind
    {
        KIND_NONE = 0,          /**< no payload */
        KIND_EMPTY,             /**< a payload of no bytes */
        KIND_INLINE,            /**< inside the package, at offset */
        KIND_TRAILER,           /**< inside the package trailer, at offset (the trailer entry was found) */
        KIND_UNAVAILABLE        /**< not in this package: see reason */
    };

    EKind kind;
    UInt64 offset;              /**< absolute offset in the package of the stored bytes, for KIND_INLINE and KIND_TRAILER */
    UInt64 storedSize;          /**< bytes in the package */
    UInt64 rawSize;             /**< bytes after decompression */
    UInt8 identifier[ 20 ];     /**< hash of the raw payload, when the form has one */
    bool hasIdentifier;
    UInt32 flags;               /**< the flags of the record the reference came from */
    CORE::CString reason;       /**< for KIND_UNAVAILABLE */

    CUnrealPayloadReference( void );

    bool IsReadable( void ) const;

    static const char* KindToString( EKind kind );
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */
