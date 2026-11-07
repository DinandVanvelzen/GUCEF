/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHCONTAINER_H
#define GUCEF_MULTIPLAY_CMATCHCONTAINER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#include "gucefMULTIPLAY_CMatchPreview.h"
#define GUCEF_MULTIPLAY_CMATCHPREVIEW_H
#endif /* GUCEF_MULTIPLAY_CMATCHPREVIEW_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#include "gucefMULTIPLAY_CMatchPackage.h"
#define GUCEF_MULTIPLAY_CMATCHPACKAGE_H
#endif /* GUCEF_MULTIPLAY_CMATCHPACKAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EMatchContainerResult
{
    MATCHCONTAINER_OK                = 0 ,
    MATCHCONTAINER_TOO_LARGE         = 1 ,  /**< above the cap, refused before anything is read */
    MATCHCONTAINER_TRUNCATED         = 2 ,  /**< shorter than its header says */
    MATCHCONTAINER_BAD_MAGIC         = 3 ,
    MATCHCONTAINER_BAD_VERSION       = 4 ,
    MATCHCONTAINER_BAD_HEADER        = 5 ,  /**< a field outside its limits, or bytes behind the end */
    MATCHCONTAINER_BAD_PREVIEW       = 6 ,
    MATCHCONTAINER_BAD_PAYLOAD       = 7 ,  /**< the payload does not have the SHA-256 or the size the header says */
    MATCHCONTAINER_BAD_COMPRESSION   = 8 ,  /**< the gzip data is damaged or larger than the header says */
    MATCHCONTAINER_CODEC_UNAVAILABLE = 9 ,  /**< the payload is gzip and there is no gzip codec */
    MATCHCONTAINER_INCONSISTENT      = 10   /**< the preview, the header and the payload do not describe the same package */
};
typedef enum EMatchContainerResult TMatchContainerResult;

GUCEF_MULTIPLAY_EXPORT_CPP const char* MatchContainerResultToString( TMatchContainerResult result );

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The one binary form in which a turn goes from machine to machine: a file, a mail attachment or the text of a
 *  mail body (CMatchTextArmor carries these same bytes). Layout, numbers big endian:
 *
 *      magic "GMTC" (4), version (1) = 1, flags (1: bit 0 payload is gzip), preview size (2), stored payload size (4),
 *      raw payload size (4), revision (4), base revision (4), base SHA-256 (32, zero when there is none),
 *      SHA-256 of the raw payload (32), the signed preview (binary form), the stored payload.
 *
 *  The payload is gzip when the gzip codec is available and it makes the payload smaller, stored as it is when
 *  there is no codec or compression does not help; a reader accepts both and refuses gzip only when it has no codec.
 *
 *  The reader is for bytes from outside. Sizes are checked against their caps before anything is copied or
 *  allocated, every field is bounded, every truncation and every extra byte is refused, and gzip data is
 *  expanded in small steps that stop as soon as the output is larger than the raw size in the header (a
 *  small file cannot make a large allocation). The signature of the preview is not checked here (the
 *  pinned keys are not known to the codec): see CMatchPreview::VerifyWithKey() and CMatchChainVerifier.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchContainer
{
    public:

    static const UInt32 MaxContainerSize = 2 * 1024 * 1024;
    static const UInt32 HeaderSize = 88;

    CMatchPreview preview;
    CMatchPackage package;

    CMatchContainer( void );

    CMatchContainer( const CMatchContainer& src );

    ~CMatchContainer();

    CMatchContainer& operator=( const CMatchContainer& src );

    /** Whether the gzip codec is there (a plugin of the application provides it) */
    static bool IsCompressionAvailable( void );

    /**
     *  Makes the bytes of a container
     *
     *  @param compress gzip the payload when the codec is there and it helps
     */
    static TMatchContainerResult Encode( const CMatchPreview& preview   ,
                                         const CMatchPackage& package   ,
                                         bool compress                  ,
                                         CORE::CDynamicBuffer& bytes    );

    /** Reads the bytes of a container, nothing of this object is changed unless the result is MATCHCONTAINER_OK */
    TMatchContainerResult Decode( const void* data, UInt32 size );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHCONTAINER_H ? */
