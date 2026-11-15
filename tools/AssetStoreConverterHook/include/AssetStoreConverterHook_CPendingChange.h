/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H
#define ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A previewed write of the authoring operations (section 7.4 of the asset tools design), waiting for its apply: the
 *  single use change token, who may apply it (the principal of the preview), what it writes (the operation, the
 *  target, the arguments it was made from), the digest of the target when it was previewed and the digest and content
 *  it will have, and when the token expires.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPendingChange
{
    public:

    CORE::CString token;
    CORE::CString principal;
    CORE::CString operation;
    CORE::CString targetVfsPath;        /**< never shown to a client */
    CORE::CString targetRooted;
    bool targetExisted;
    CORE::CString baseDigest;           /**< SHA-256 of the target at the preview; empty when it did not exist */
    CORE::CString newDigest;            /**< SHA-256 of the new content; empty for a removal */
    CORE::CString argumentsDigest;
    CORE::CString newContent;
    bool removes;                       /**< the target is removed, not written */
    UInt64 expiresAtMs;                 /**< monotonic */
    Int64 expiresUtcMs;

    CPendingChange( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPENDINGCHANGE_H ? */
