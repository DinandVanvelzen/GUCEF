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

#ifndef GUCEF_VIDEO_CVIDEOGLOBAL_H
#define GUCEF_VIDEO_CVIDEOGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_MACROS_H
#include "gucefVIDEO_macros.h"
#define GUCEF_VIDEO_MACROS_H
#endif /* GUCEF_VIDEO_MACROS_H ? */

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

class CGUCEFVIDEOModule;
class CVideoCodecRegistry;
class CVideoCodecPluginManager;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Video systems: the codec registry and the codec plugin manager.
 */
class GUCEF_VIDEO_PUBLIC_CPP CVideoGlobal
{
    public:

    static CVideoGlobal* Instance( void );

    /** All video codecs by type name. Codecs come from codec plugins, they are added when a plugin loads */
    CVideoCodecRegistry& GetVideoCodecRegistry( void );

    /** Loads video codec plugins with the C interface; their codec types end up in the codec registry */
    CVideoCodecPluginManager& GetVideoCodecPluginManager( void );

    private:
    friend class CGUCEFVIDEOModule;

    static void Deinstance( void );

    private:

    CVideoGlobal( void );

    ~CVideoGlobal();

    void Initialize( void );

    CVideoGlobal( const CVideoGlobal& src );            /**< not implemented */
    CVideoGlobal& operator=( const CVideoGlobal& src ); /**< not implemented */

    private:

    CVideoCodecRegistry* m_videoCodecRegistry;
    CVideoCodecPluginManager* m_videoCodecPluginManager;

    static CVideoGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VIDEO_CVIDEOGLOBAL_H ? */
