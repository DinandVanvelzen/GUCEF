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

#include <string.h>

#include "OgreException.h"
#include "OgreImage.h"
#include "OgreDataStream.h"
#include "OgreAny.h"

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H
#include "pluginglueOGRECOMMON_COgreDataStreamToIOAccess.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H ? */

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H
#include "OgreCodec_GucefIMAGE_CGucefImageConverter.h"
#define GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H
#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGECONVERTER_H ? */

#include "OgreCodec_GucefIMAGE_CGucefImageOgreCodec.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefImageOgreCodec::CGucefImageOgreCodec( const Ogre::String& extension )
    : Ogre::ImageCodec()
    , m_extension( extension )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefImageOgreCodec::~CGucefImageOgreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Ogre::String
CGucefImageOgreCodec::getType( void ) const
{GUCEF_TRACE;

    return m_extension;
}

/*-------------------------------------------------------------------------*/

Ogre::String
CGucefImageOgreCodec::magicNumberToFileExt( const char* magicNumberPtr, size_t maxbytes ) const
{GUCEF_TRACE;

    return Ogre::String();
}

/*-------------------------------------------------------------------------*/

void
CGucefImageOgreCodec::decode( const Ogre::DataStreamPtr& input, const Ogre::Any& output ) const
{GUCEF_TRACE;

    Ogre::Image* image = Ogre::any_cast< Ogre::Image* >( output );
    if ( !input || GUCEF_NULL == image )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "No stream or no image to decode " + m_extension + " into", "CGucefImageOgreCodec::decode" );
    }

    // the codecs of gucefIMAGE need to be able to move around in the data, and read it from the start
    OGRECOMMON::COgreDataStreamToIOAccess access( input );
    if ( 0 != access.Setpos( 0 ) )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "The stream " + input->getName() + " cannot be positioned at its start, it cannot be decoded as " + m_extension,
                     "CGucefImageOgreCodec::decode" );
    }

    IMAGE::CImage decoded;
    if ( !decoded.Load( access, m_extension.c_str(), true, false ) )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "gucefIMAGE could not decode the stream " + input->getName() + " as " + m_extension,
                     "CGucefImageOgreCodec::decode" );
    }

    Ogre::String problem;
    if ( !CGucefImageConverter::ToOgreImage( decoded, *image, problem ) )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "The " + m_extension + " image " + input->getName() + " cannot be used by Ogre: " + problem,
                     "CGucefImageOgreCodec::decode" );
    }
}

/*-------------------------------------------------------------------------*/

Ogre::DataStreamPtr
CGucefImageOgreCodec::encode( const Ogre::Any& input ) const
{GUCEF_TRACE;

    const Ogre::Image* image = GUCEF_NULL;
    try
    {
        image = Ogre::any_cast< Ogre::Image* >( input );
    }
    catch ( const Ogre::Exception& )
    {
        image = GUCEF_NULL;
    }
    if ( GUCEF_NULL == image )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "Encoding as " + m_extension + " needs an Ogre image", "CGucefImageOgreCodec::encode" );
    }

    IMAGE::CImage toEncode;
    Ogre::String problem;
    if ( !CGucefImageConverter::FromOgreImage( *image, toEncode, problem ) )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_INVALIDPARAMS, "The Ogre image cannot be encoded as " + m_extension + ": " + problem, "CGucefImageOgreCodec::encode" );
    }

    CORE::CDynamicBuffer encoded( true );
    CORE::CDynamicBufferAccess encodedAccess( encoded );
    if ( !toEncode.Save( encodedAccess, m_extension.c_str() ) || 0 == encoded.GetDataSize() )
    {
        OGRE_EXCEPT( Ogre::Exception::ERR_NOT_IMPLEMENTED, "gucefIMAGE has no encoder for " + m_extension + " that can write this image", "CGucefImageOgreCodec::encode" );
    }

    Ogre::MemoryDataStreamPtr result = std::make_shared< Ogre::MemoryDataStream >( (size_t) encoded.GetDataSize() );
    memcpy( result->getPtr(), encoded.GetConstBufferPtr(), (size_t) encoded.GetDataSize() );
    return result;
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
