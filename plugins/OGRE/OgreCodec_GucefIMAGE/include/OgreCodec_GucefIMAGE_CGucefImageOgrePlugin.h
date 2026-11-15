/*
 *  OgreCodec_GucefIMAGE: Ogre plugin that decodes and encodes the image formats of gucefIMAGE for Ogre
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

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGREPLUGIN_H
#define GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGREPLUGIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgrePlugin.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H
#include "OgreCodec_GucefIMAGE_macros.h"
#define GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H
#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGucefImageOgreCodec;

/*-------------------------------------------------------------------------*/

/**
 *  Ogre plugin that gives Ogre an image codec for every file extension that gucefIMAGE has a decoder or an
 *  encoder for, so Ogre decodes the common image formats (PNG, JPEG, ...) through gucefIMAGE and not itself.
 *
 *  The extensions are those of the image codec registry of gucefIMAGE and of the codecs of the family "ImageCodec" in
 *  the codec registry of gucefCORE. The formats that are uploaded to the GPU as they are (dds, ktx, pkm, astc, pvr)
 *  stay with the codecs of Ogre, as does every extension that has an Ogre codec already, whoever registered it.
 *
 *  The extensions are taken when the plugin is installed and again whenever gucefIMAGE gets a codec, for example
 *  because an image plugin is loaded later.
 */
class GUCEF_OGRECODEC_GUCEFIMAGE_EXPORT_CPP CGucefImageOgrePlugin : public Ogre::Plugin ,
                                                                     public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CGucefImageOgrePlugin >    TEventCallback;

    CGucefImageOgrePlugin( void );

    virtual ~CGucefImageOgrePlugin() GUCEF_VIRTUAL_OVERRIDE;

    virtual const Ogre::String& getName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void install( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void initialise( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void shutdown( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void uninstall( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the Ogre codecs match the codecs of gucefIMAGE now. Returns the number of Ogre codecs this plugin has */
    UInt32 RefreshCodecs( void );

    /** The extensions this plugin has Ogre codecs for */
    Ogre::StringVector GetExtensions( void ) const;

    private:

    CGucefImageOgrePlugin( const CGucefImageOgrePlugin& src );              /**< not implemented */
    CGucefImageOgrePlugin& operator=( const CGucefImageOgrePlugin& src );   /**< not implemented */

    void OnCodecRegistryChanged( CORE::CNotifier* notifier   ,
                                 const CORE::CEvent& eventId ,
                                 CORE::CICloneable* evenData );

    void SubscribeToCodecRegistries( void );

    private:

    typedef GUCEF::map< Ogre::String, CGucefImageOgreCodec* > TCodecMap;

    mutable MT::CMutex m_lock;
    TCodecMap m_codecs;
    bool m_installed;
    bool m_subscribedToRegistries;
    bool m_subscribedToFamily;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGREPLUGIN_H ? */
