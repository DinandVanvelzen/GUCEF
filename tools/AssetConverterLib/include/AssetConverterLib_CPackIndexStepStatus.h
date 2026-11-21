/*
 *  AssetConverterLib: the state of one step (analyse, import, convert, thumbnail, cook) for one resource of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H
#define ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/** What is true of one step for one resource: { s: "fresh|stale|missing|failed|refused|skipped", key?, reasons? } */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexStepStatus
{
    public:

    static const CORE::CString Fresh;
    static const CORE::CString Stale;
    static const CORE::CString Missing;
    static const CORE::CString Failed;
    static const CORE::CString Refused;
    static const CORE::CString Skipped;

    CORE::CString state;                    /**< one of the names above */
    CORE::CString key;                      /**< the key the step ran with (hex); empty when the step has none */
    CORE::CString::StringVector reasons;    /**< in words, with the names of TActivityReason */

    CPackIndexStepStatus( void );
    explicit CPackIndexStepStatus( const CORE::CString& stepState );

    bool operator==( const CPackIndexStepStatus& other ) const;
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

#endif /* ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H ? */

