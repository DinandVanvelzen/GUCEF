/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

/*
 *  Deprecated: CWebSessionSignInSettings moved to gucefAUTH, header gucefAUTH_CWebSessionSignInSettings.h, name GUCEF::AUTH::CWebSessionSignInSettings.
 *  This header and the name in GUCEF::ASSETSTORE are kept for one release so code written against them still
 *  compiles; use the gucefAUTH header and name instead.
 */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#include "gucefAUTH_CWebSessionSignInSettings.h"
#define GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H
#endif /* GUCEF_AUTH_CWEBSESSIONSIGNINSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/* deprecated: use the names of GUCEF::AUTH */
using AUTH::CWebSessionSignInSettings;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNINSETTINGS_H ? */
