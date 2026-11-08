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

#ifndef GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H
#define GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#include "gucefAUDIO_CAudioEffectFactory.h"
#define GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H
#endif /* GUCEF_AUDIO_CAUDIOEFFECTFACTORY_H ? */

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
 *  Factory for an effect class T. T provides:
 *   - static const CString& GetStaticEffectTypeName( void )
 *   - static UInt32 GetStaticParameterCount( void )
 *   - static const CAudioEffectParameterInfo* GetStaticParameterInfos( void )
 *   - a constructor T( const CORE::CValueList& options )
 */
template < class T >
class CTAudioEffectFactory : public CAudioEffectFactory
{
    public:

    CTAudioEffectFactory( void ) : CAudioEffectFactory() {}

    virtual ~CTAudioEffectFactory() GUCEF_VIRTUAL_OVERRIDE {}

    virtual const CString& GetEffectTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return T::GetStaticEffectTypeName();
    }

    virtual UInt32 GetParameterCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return T::GetStaticParameterCount();
    }

    virtual const CAudioEffectParameterInfo* GetParameterInfo( UInt32 parameterId ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return parameterId < T::GetStaticParameterCount() ? &T::GetStaticParameterInfos()[ parameterId ] : GUCEF_NULL;
    }

    protected:

    virtual CAudioEffect* OnCreate( const CORE::CValueList& options ) GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_NEW T( options );
    }

    private:

    CTAudioEffectFactory( const CTAudioEffectFactory& src );                /**< not implemented */
    CTAudioEffectFactory& operator=( const CTAudioEffectFactory& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CTAUDIOEFFECTFACTORY_H ? */
