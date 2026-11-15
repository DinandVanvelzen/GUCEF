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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreCodec.h"
#include "OgreStringConverter.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H
#include "OgreCodec_GucefIMAGE_CGucefImageOgreCodec.h"
#define GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H
#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H ? */

#include "OgreCodec_GucefIMAGE_CGucefImageOgrePlugin.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Formats that are uploaded to the GPU as they are, which the built in codecs of Ogre handle without decoding */
static bool
IsGpuReadyExtension( const Ogre::String& extension )
{GUCEF_TRACE;

    static const char* const gpuReady[] = { "dds", "ktx", "ktx2", "pkm", "astc", "pvr", "ete", GUCEF_NULL };
    for ( int i=0; GUCEF_NULL != gpuReady[ i ]; ++i )
    {
        if ( extension == gpuReady[ i ] )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static Ogre::String
ToExtension( const CORE::CString& codecType )
{GUCEF_TRACE;

    Ogre::String extension = codecType.C_String();
    Ogre::StringUtil::toLowerCase( extension );
    return extension;
}

/*-------------------------------------------------------------------------*/

/** Every extension that gucefIMAGE has an image codec for, from the registry of gucefIMAGE and from the codecs of gucefCORE */
static void
GatherGucefImageExtensions( GUCEF::set< Ogre::String >& extensions )
{GUCEF_TRACE;

    // the codecs of the image plugins that have the extended interface
    IMAGE::CImageCodecRegistry& imageRegistry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
    CORE::CString::StringVector names;
    imageRegistry.GetRegisteredObjNames( names );
    CORE::CString::StringVector::const_iterator i = names.begin();
    while ( i != names.end() )
    {
        extensions.insert( ToExtension( (*i) ) );
        ++i;
    }

    // the codecs of the family ImageCodec of the codec registry, the built in PNG encoder is one
    CORE::CCodecRegistry::TCodecProviderVector providers;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().FindCodecs( "ImageCodec", 0, providers ) )
    {
        CORE::CCodecRegistry::TCodecProviderVector::const_iterator n = providers.begin();
        while ( n != providers.end() )
        {
            extensions.insert( ToExtension( (*n).codecType ) );
            ++n;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefImageOgrePlugin::CGucefImageOgrePlugin( void )
    : Ogre::Plugin()
    , CORE::CObserver()
    , m_lock()
    , m_codecs()
    , m_installed( false )
    , m_subscribedToRegistries( false )
    , m_subscribedToFamily( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefImageOgrePlugin::~CGucefImageOgrePlugin()
{GUCEF_TRACE;

    // the registries do not call us while we are being taken apart
    SignalUpcomingObserverDestruction();
}

/*-------------------------------------------------------------------------*/

const Ogre::String&
CGucefImageOgrePlugin::getName( void ) const
{GUCEF_TRACE;

    static const Ogre::String name = "GucefIMAGE Codecs";
    return name;
}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::SubscribeToCodecRegistries( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CGucefImageOgrePlugin::OnCodecRegistryChanged );

    if ( !m_subscribedToRegistries )
    {
        m_subscribedToRegistries = true;

        // codecs that image plugins register for their formats
        IMAGE::CImageCodecRegistry& imageRegistry = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry();
        SubscribeTo( &imageRegistry.AsNotifier(), IMAGE::CImageCodecRegistry::ItemRegisteredEvent, callback );
        SubscribeTo( &imageRegistry.AsNotifier(), IMAGE::CImageCodecRegistry::ItemUnregisteredEvent, callback );

        // the families of the codec registry of gucefCORE: the family ImageCodec may be made later
        CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
        SubscribeTo( &codecRegistry.AsNotifier(), CORE::CCodecRegistry::ItemRegisteredEvent, callback );
    }

    // the family ImageCodec, which the codecs are added to
    if ( !m_subscribedToFamily )
    {
        CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
        if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( "ImageCodec", family, true ) && !family.IsNULL() )
        {
            m_subscribedToFamily = true;
            SubscribeTo( &family.GetPointerAlways()->AsNotifier(), CORE::CCodecRegistry::TCodecFamilyRegistry::ItemRegisteredEvent, callback );
            SubscribeTo( &family.GetPointerAlways()->AsNotifier(), CORE::CCodecRegistry::TCodecFamilyRegistry::ItemUnregisteredEvent, callback );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::install( void )
{GUCEF_TRACE;

    m_installed = true;
    SubscribeToCodecRegistries();
    const UInt32 count = RefreshCodecs();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OgreCodec_GucefIMAGE: " << count << " image codecs of gucefIMAGE are registered in Ogre" );
}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::initialise( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::shutdown( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::uninstall( void )
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    MT::CScopeMutex lock( m_lock );
    m_installed = false;
    m_subscribedToRegistries = false;
    m_subscribedToFamily = false;
    TCodecMap::iterator i = m_codecs.begin();
    while ( i != m_codecs.end() )
    {
        Ogre::Codec::unregisterCodec( (*i).second );
        delete (*i).second;
        ++i;
    }
    m_codecs.clear();
}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgrePlugin::OnCodecRegistryChanged( CORE::CNotifier* notifier   ,
                                               const CORE::CEvent& eventId ,
                                               CORE::CICloneable* evenData )
{GUCEF_TRACE;

    // a new family of the codec registry of gucefCORE may be the one we have to watch
    if ( eventId == CORE::CCodecRegistry::ItemRegisteredEvent )
    {
        SubscribeToCodecRegistries();
    }
    RefreshCodecs();
}

/*-------------------------------------------------------------------------*/

UInt32
CGucefImageOgrePlugin::RefreshCodecs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_installed )
        return 0;

    GUCEF::set< Ogre::String > wanted;
    GatherGucefImageExtensions( wanted );

    // out with the codecs for extensions that gucefIMAGE no longer has
    TCodecMap::iterator i = m_codecs.begin();
    while ( i != m_codecs.end() )
    {
        if ( wanted.find( (*i).first ) == wanted.end() )
        {
            Ogre::Codec::unregisterCodec( (*i).second );
            delete (*i).second;
            m_codecs.erase( i++ );
        }
        else
        {
            ++i;
        }
    }

    // in with the codecs for the new ones, unless Ogre has a codec for the extension already
    GUCEF::set< Ogre::String >::const_iterator n = wanted.begin();
    while ( n != wanted.end() )
    {
        const Ogre::String& extension = (*n);
        if ( !extension.empty() && m_codecs.find( extension ) == m_codecs.end() )
        {
            if ( IsGpuReadyExtension( extension ) || Ogre::Codec::isCodecRegistered( extension ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "OgreCodec_GucefIMAGE: Ogre has a codec for " << extension << " already, it is kept" );
            }
            else
            {
                CGucefImageOgreCodec* codec = new CGucefImageOgreCodec( extension );
                Ogre::Codec::registerCodec( codec );
                m_codecs[ extension ] = codec;
            }
        }
        ++n;
    }
    return (UInt32) m_codecs.size();
}

/*-------------------------------------------------------------------------*/

Ogre::StringVector
CGucefImageOgrePlugin::GetExtensions( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Ogre::StringVector extensions;
    TCodecMap::const_iterator i = m_codecs.begin();
    while ( i != m_codecs.end() )
    {
        extensions.push_back( (*i).first );
        ++i;
    }
    return extensions;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
