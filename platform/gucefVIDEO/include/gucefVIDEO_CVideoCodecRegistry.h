/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_VIDEO_CIVIDEOCODEC_H
#include "gucefVIDEO_CIVideoCodec.h"
#define GUCEF_VIDEO_CIVIDEOCODEC_H
#endif /* GUCEF_VIDEO_CIVIDEOCODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CVideoGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  All video codecs that can be used, by the name of their type ("smk", ...). Built in codecs and the codecs
 *  of loaded codec plugins end up here. Reach it through CVideoGlobal.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoCodecRegistry : public CORE::CTGlobalONRegistry< CIVideoCodec, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CIVideoCodec, MT::CMutex >::TRegisteredObjPtr   TVideoCodecPtr;

    /** Finds a codec by type, without regard for upper or lower case. A leading dot (as in an extension) is ignored */
    bool TryFindCodec( const CString& codecType, TVideoCodecPtr& codec ) const;

    /** Convenience: the codec for the extension of a file name or path */
    bool TryFindCodecForFilename( const CString& filename, TVideoCodecPtr& codec ) const;

    private:
    friend class CVideoGlobal;

    CVideoCodecRegistry( void );

    virtual ~CVideoCodecRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CVideoCodecRegistry( const CVideoCodecRegistry& src );              /**< not implemented, don't use */
    CVideoCodecRegistry& operator=( const CVideoCodecRegistry& src );   /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */
