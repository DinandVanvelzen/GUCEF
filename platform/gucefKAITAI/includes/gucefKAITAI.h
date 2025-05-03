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

/*
 *      This is the main include header for the gucefKAITAI library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with utilizing the Kaitai descriptive format to interpret binary data
 */

#ifndef GUCEF_KAITAI_H
#define GUCEF_KAITAI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEF_KAITAI_CONFIG_H
#include "gucefKAITAI_config.h"
#define GUCEF_KAITAI_CONFIG_H
#endif /* GUCEF_KAITAI_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_KAITAI_MACROS_H
#include "gucefKAITAI_macros.h"
#define GUCEF_KAITAI_MACROS_H
#endif /* GUCEF_KAITAI_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_KAITAI_CKAITAISCHEMA_H
#include "gucefKAITAI_CKaitaiSchema.h"
#define GUCEF_KAITAI_CKAITAISCHEMA_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMA_H ? */

#ifndef GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CKaitaiSchemaRegistry.h"
#define GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CKAITAISCHEMAREGISTRY_H ? */

#ifndef GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H
#include "gucefKAITAI_CGlobalKaitaiSchemaRegistry.h"    
#define GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H
#endif /* GUCEF_KAITAI_CGLOBALKAITAISCHEMAREGISTRY_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H
#include "gucefKAITAI_CKaitaiDataDrivenDStoreCodecFactory.h"
#define GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODECFACTORY_H ? */

#ifndef GUCEF_KAITAI_CKAITAIGLOBAL_H
#include "gucefKAITAI_CKaitaiGlobal.h"
#define GUCEF_KAITAI_CKAITAIGLOBAL_H
#endif /* GUCEF_KAITAI_CKAITAIGLOBAL_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDSTORECODEC_H
#include "gucefKAITAI_CKaitaiDStoreCodec.h"
#define GUCEF_KAITAI_CKAITAIDSTORECODEC_H
#endif /* GUCEF_KAITAI_CKAITAIDSTORECODEC_H ? */

#ifndef GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H
#include "gucefKAITAI_CKaitaiDataDrivenDStoreCodec.h"
#define GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H
#endif /* GUCEF_KAITAI_CKAITAIDATADRIVENDSTORECODEC_H ? */


#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_KAITAI_H ? */
