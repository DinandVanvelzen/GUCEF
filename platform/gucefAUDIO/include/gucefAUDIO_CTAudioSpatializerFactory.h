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

#ifndef GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H
#define GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H
#include "gucefAUDIO_CAudioSpatializerFactory.h"
#define GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H
#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H ? */

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

/**
 *  Factory for a spatializer class T. T provides:
 *   - static const CString& GetStaticSpatializerTypeName( void )
 *   - a default constructor
 */
template < class T >
class CTAudioSpatializerFactory : public CAudioSpatializerFactory
{
    public:

    CTAudioSpatializerFactory( void ) : CAudioSpatializerFactory() {}

    virtual ~CTAudioSpatializerFactory() GUCEF_VIRTUAL_OVERRIDE {}

    virtual const CString& GetSpatializerTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return T::GetStaticSpatializerTypeName();
    }

    protected:

    virtual IAudioSpatializer* OnCreate( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_NEW T();
    }

    private:

    CTAudioSpatializerFactory( const CTAudioSpatializerFactory& src );                /**< not implemented */
    CTAudioSpatializerFactory& operator=( const CTAudioSpatializerFactory& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CTAUDIOSPATIALIZERFACTORY_H ? */
