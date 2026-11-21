/*
 *  AssetConverterLib: the status of one resource of the pack index: its level and the state of its steps
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSTATUS_H
#define ASSETCONVERTERLIB_CPACKINDEXSTATUS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H
#include "AssetConverterLib_CPackIndexStepStatus.h"
#define ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSTEPSTATUS_H ? */

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

/** { state, steps }: state is indexed (structure only), analysed, converted, failed or refused (licence) */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexStatus
{
    public:

    typedef GUCEF::map< CORE::CString, CPackIndexStepStatus > TStepMap;

    static const CORE::CString Indexed;
    static const CORE::CString Analysed;
    static const CORE::CString Converted;
    static const CORE::CString Failed;
    static const CORE::CString Refused;

    static const CORE::CString StepAnalyse;
    static const CORE::CString StepImport;
    static const CORE::CString StepConvert;
    static const CORE::CString StepThumbnail;
    static const CORE::CString StepCook;

    CORE::CString state;
    TStepMap steps;                         /**< by step name, in the canonical order of the map */

    CPackIndexStatus( void );

    /** The step, or NULL when the resource has no record of it */
    const CPackIndexStepStatus* FindStep( const CORE::CString& step ) const;

    void SetStep( const CORE::CString& step, const CPackIndexStepStatus& status );

    bool operator==( const CPackIndexStatus& other ) const;
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

#endif /* ASSETCONVERTERLIB_CPACKINDEXSTATUS_H ? */

