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

#ifndef GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H
#define GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_IAUDIOSPATIALIZER_H
#include "gucefAUDIO_IAudioSpatializer.h"
#define GUCEF_AUDIO_IAUDIOSPATIALIZER_H
#endif /* GUCEF_AUDIO_IAUDIOSPATIALIZER_H ? */

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
 *  Makes spatializers of one type. Registered in CAudioSpatializerRegistry under its type name, by CAudioGlobal for the
 *  built-in panning spatializer and by spatializer plugins (C++ link-back plugins, as effect plugins are) for theirs.
 *
 *  A factory counts the spatializers it made that are still alive. A plugin must not be unloaded while any of them live,
 *  because their code is in the plugin: CAudioSpatializerRegistry::TryUnregisterFactory() refuses in that case, and the
 *  factory must outlive every spatializer it made.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioSpatializerFactory
{
    public:

    typedef CORE::CTSharedPtr< CAudioSpatializerFactory, MT::CMutex > TAudioSpatializerFactoryPtr;

    CAudioSpatializerFactory( void );

    virtual ~CAudioSpatializerFactory();

    /** Type name of the spatializers this factory makes, the key in the registry */
    virtual const CString& GetSpatializerTypeName( void ) const = 0;

    /** Makes a spatializer, not yet prepared. Null when it cannot be made */
    TAudioSpatializerPtr Create( void );

    /** Spatializers made by this factory that are still alive */
    UInt32 GetLiveInstanceCount( void ) const;

    protected:

    /** Makes the spatializer object. The factory links it to itself for counting */
    virtual IAudioSpatializer* OnCreate( void ) = 0;

    private:
    friend class IAudioSpatializer;

    CAudioSpatializerFactory( const CAudioSpatializerFactory& src );              /**< not implemented */
    CAudioSpatializerFactory& operator=( const CAudioSpatializerFactory& src );   /**< not implemented */

    void OnInstanceDestroyed( void );

    private:

    MT::CTAtomic< UInt32 > m_liveInstances;
};

/*-------------------------------------------------------------------------*/

typedef CAudioSpatializerFactory::TAudioSpatializerFactoryPtr TAudioSpatializerFactoryPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOSPATIALIZERFACTORY_H ? */
