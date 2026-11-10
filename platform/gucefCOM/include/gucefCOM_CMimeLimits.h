/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMIMELIMITS_H
#define GUCEF_COM_CMIMELIMITS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMIMEPART_H
#include "gucefCOM_CMimePart.h"
#define GUCEF_COM_CMIMEPART_H
#endif /* GUCEF_COM_CMIMEPART_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a message may be made of before the parser gives up: the guard against hostile input */
class GUCEF_COM_PUBLIC_CPP CMimeLimits
{
    public:

    UInt32 maxMessageSize;          /**< bytes of the message, 64 MiB */
    UInt32 maxHeaderBytes;          /**< bytes of one header section, 256 KiB */
    UInt32 maxHeaderCount;          /**< fields in one header section, 1000 */
    UInt32 maxParts;                /**< entities in the tree, 512 */
    UInt32 maxDepth;                /**< nesting of multiparts, 16 */
    UInt32 maxDecodedPartSize;      /**< bytes of one decoded body, 64 MiB */

    CMimeLimits( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMIMELIMITS_H ? */
