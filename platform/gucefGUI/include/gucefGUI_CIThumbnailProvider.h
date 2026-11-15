/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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
#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A maker of small pictures of what a CContentBrowserForm lists. The form asks for a thumbnail of a path at a size
 *  with RequestThumbnail() and does not wait for it: the provider answers whenever the picture is ready (a slow render
 *  on a thread of its own, a cache read) by notifying ThumbnailReadyEvent, with the path and size it was asked for and
 *  the picture, or no picture when there is none to make. The GUI thread is never blocked by the making of a thumbnail.
 *
 *  ThumbnailReadyEvent has to be notified on the GUI thread, the thread the form runs on (a provider that makes its
 *  pictures on other threads hands the results over and notifies from a pulse of the GUI thread, for example with a
 *  CORE::CFutureResult it checks every frame). The form shows the picture in a tile when the tile is still there; a
 *  thumbnail for a path or size the form does not need any more is ignored (kept for later).
 */
class GUCEF_GUI_PUBLIC_CPP CIThumbnailProvider : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ThumbnailReadyEvent;

    static void RegisterEvents( void );

    /** The data of ThumbnailReadyEvent */
    struct GUCEF_GUI_PUBLIC_CPP SThumbnail
    {
        CString path;
        UInt32 size;                    /**< the size asked for: the longest side of the picture is at most this many pixels */
        IMAGE::TPixelMapPtr pixels;     /**< NULL when no thumbnail can be made for the path */

        SThumbnail( void );
    };
    typedef struct SThumbnail TThumbnail;
    typedef CORE::CTCloneableObj< TThumbnail > TThumbnailReadyEventData;

    CIThumbnailProvider( void );

    virtual ~CIThumbnailProvider();

    /** Asks for a thumbnail of a path whose longest side is at most 'size' pixels; the answer comes with ThumbnailReadyEvent, later */
    virtual void RequestThumbnail( const CString& path, UInt32 size ) = 0;

    /** The form does not need the thumbnail any more (the tile was scrolled away or the folder left); the default does nothing */
    virtual void CancelThumbnail( const CString& path, UInt32 size );

    private:

    CIThumbnailProvider( const CIThumbnailProvider& src );
    CIThumbnailProvider& operator=( const CIThumbnailProvider& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */
