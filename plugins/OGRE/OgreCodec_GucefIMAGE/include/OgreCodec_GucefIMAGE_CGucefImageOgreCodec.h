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

#ifndef GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H
#define GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreImageCodec.h"

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

/**
 *  The Ogre image codec of one file extension, which decodes (and encodes, where there is an encoder) through the
 *  image codecs that gucefIMAGE has for that extension.
 *
 *  Decoding takes the data stream as is, the stream must be able to seek. The result is an Ogre image of one face
 *  or six, with the mipmap levels the file had (see CGucefImageConverter). A decoder that cannot read the data makes
 *  an Ogre exception.
 *
 *  Encoding gives the data in memory: Image::encode( "png" ) works where gucefIMAGE has an encoder for it. Ogre
 *  does not write files by itself in this code base, so encodeToFile() is not supported.
 */
class GUCEF_OGRECODEC_GUCEFIMAGE_EXPORT_CPP CGucefImageOgreCodec : public Ogre::ImageCodec
{
    public:

    /** The extension is what Ogre names the codec by, lower case without a dot */
    explicit CGucefImageOgreCodec( const Ogre::String& extension );

    virtual ~CGucefImageOgreCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual void decode( const Ogre::DataStreamPtr& input, const Ogre::Any& output ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Ogre::DataStreamPtr encode( const Ogre::Any& input ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Ogre::String getType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** gucefIMAGE has no way to tell a format from its first bytes: only the extension selects the codec */
    virtual Ogre::String magicNumberToFileExt( const char* magicNumberPtr, size_t maxbytes ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGucefImageOgreCodec( const CGucefImageOgreCodec& src );              /**< not implemented */
    CGucefImageOgreCodec& operator=( const CGucefImageOgreCodec& src );   /**< not implemented */

    private:

    Ogre::String m_extension;
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

#endif /* GUCEF_OGRECODEC_GUCEFIMAGE_CGUCEFIMAGEOGRECODEC_H ? */
