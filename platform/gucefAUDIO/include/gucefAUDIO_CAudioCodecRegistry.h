/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#define GUCEF_AUDIO_CAUDIOCODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#include "gucefCORE_CTGlobalONRegistry.h"
#define GUCEF_CORE_CTGLOBALONREGISTRY_H
#endif /* GUCEF_CORE_CTGLOBALONREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CIAUDIOCODEC_H
#include "gucefAUDIO_CIAudioCodec.h"
#define GUCEF_AUDIO_CIAUDIOCODEC_H
#endif /* GUCEF_AUDIO_CIAUDIOCODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAudioGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  All audio codecs that can be used, by the name of their type ("wav", "ogg", ...). Built in codecs and the codecs
 *  of loaded codec plugins end up here. Reach it through CAudioGlobal.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioCodecRegistry : public CORE::CTGlobalONRegistry< CIAudioCodec, MT::CMutex >
{
    public:

    typedef CORE::CTONRegistry< CIAudioCodec, MT::CMutex >::TRegisteredObjPtr   TAudioCodecPtr;

    /** Finds a codec by type, without regard for upper or lower case. A leading dot (as in an extension) is ignored */
    bool TryFindCodec( const CString& codecType, TAudioCodecPtr& codec ) const;

    /** Convenience: the codec for the extension of a file name or path */
    bool TryFindCodecForFilename( const CString& filename, TAudioCodecPtr& codec ) const;

    /**
     *  Content sniffing for the case where the name does not settle it (no extension, a wrong one, one format
     *  under several): asks the codecs that can probe (CIAudioCodec::Probe, for plugin codecs the optional
     *  AUDIOCODECPLUG_Probe) about the data at the current position of the input and picks the most confident
     *  answer above 0. The first one found wins a tie; the order of codecs is by type name. The input position is
     *  the same afterwards. Codecs that cannot probe are never chosen this way. 'confidence' may be NULL.
     */
    bool TryFindCodecForData( CORE::CIOAccess& encodedInput, TAudioCodecPtr& codec, UInt32* confidence = GUCEF_NULL ) const;

    private:
    friend class CAudioGlobal;

    CAudioCodecRegistry( void );

    virtual ~CAudioCodecRegistry() GUCEF_VIRTUAL_OVERRIDE;

    CAudioCodecRegistry( const CAudioCodecRegistry& src );              /**< not implemented, don't use */
    CAudioCodecRegistry& operator=( const CAudioCodecRegistry& src );   /**< not implemented, don't use */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOCODECREGISTRY_H ? */
