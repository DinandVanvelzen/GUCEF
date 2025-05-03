/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#ifndef GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H
#define GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#include "CTAbstractFactoryWithParam.h"
#define GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORYWITHPARAM_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#include "gucefCORE_CDataDrivenDStoreCodecMeta.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECMETA_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#include "gucefCORE_CDataDrivenDStoreCodec.h"
#define GUCEF_CORE_CDATADRIVENDSTORECODEC_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODEC_H ? */

#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDSTORECODEC_H
#include "gucefKAITAI_CKaitaiDStoreCodec.h"
#define GUCEF_KAITAI_CKAITAIDSTORECODEC_H
#endif /* GUCEF_KAITAI_CKAITAIDSTORECODEC_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H
#include "gucefKAITAI_CKaitaiDataDrivenDStoreCodec.h"
#define GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H
#endif /* GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace KAITAI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CORE::CTFactoryWithParam< CORE::CDataDrivenDStoreCodec, CKaitaiDataDrivenDStoreCodec, CORE::CDataDrivenDStoreCodecMetaPtr, MT::CMutex >     CKaitaiDataDrivenDStoreCodecFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAITAI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H ? */
