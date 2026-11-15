/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_WEB_CWEBVIEWFRAMEINFO_H
#define GUCEF_WEB_CWEBVIEWFRAMEINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CWEBVIEWRECT_H
#include "gucefWEB_CWebViewRect.h"
#define GUCEF_WEB_CWEBVIEWRECT_H
#endif /* GUCEF_WEB_CWEBVIEWRECT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the writer of a frame exchange publishes for a frame: which of the three buffers holds it, its size and
 *  scale, its number, and the part that changed since the frame the reader took before (a union of the dirty
 *  rectangles of the frames it did not take). Small and plain: it is copied in and out of the exchange.
 */
class GUCEF_WEB_PUBLIC_CPP CWebViewFrameInfo
{
    public:

    UInt32 bufferIndex;         /**< 0, 1 or 2 */
    UInt32 width;               /**< device pixels */
    UInt32 height;
    Float32 deviceScale;        /**< device pixels per logical pixel the frame was rendered at */
    UInt64 frameNumber;         /**< 1 for the first frame of the exchange */
    CWebViewRect dirty;         /**< what changed since the frame the reader took before; all of it after a change of size */

    CWebViewFrameInfo( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBVIEWFRAMEINFO_H ? */
