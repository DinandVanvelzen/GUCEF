/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include "gucefIMAGE_CPluginImageCodecItem.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPluginImageCodecItem::CPluginImageCodecItem( TImageCodecPluginPtr pluginPtr ,
                                              const CString& codecTypeName   )
    : m_pluginPtr( pluginPtr )         ,
      m_codecTypeName( codecTypeName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginImageCodecItem::CPluginImageCodecItem( const CPluginImageCodecItem& src )
    : m_pluginPtr( src.m_pluginPtr )         ,
      m_codecTypeName( src.m_codecTypeName )
{

}

/*-------------------------------------------------------------------------*/

CPluginImageCodecItem::~CPluginImageCodecItem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPluginImageCodecItem::Encode( CORE::CIOAccess& source ,
                               CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    return m_pluginPtr->Encode( source          ,
                                dest            ,
                                m_codecTypeName );
}

/*-------------------------------------------------------------------------*/

bool
CPluginImageCodecItem::Decode( CORE::CIOAccess& source ,
                               CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    return m_pluginPtr->Decode( source          ,
                                dest            ,
                                m_codecTypeName );
}

/*-------------------------------------------------------------------------*/

bool
CPluginImageCodecItem::Encode( const CImage& inputImage       ,
                               CORE::CIOAccess& encodedOutput )
{GUCEF_TRACE;


    return m_pluginPtr->Encode( inputImage      ,
                                encodedOutput   ,
                                m_codecTypeName );
}

/*-------------------------------------------------------------------------*/

bool
CPluginImageCodecItem::Decode( CORE::CIOAccess& encodedInput ,
                               CImage& outputImage           )
{GUCEF_TRACE;


    return m_pluginPtr->Decode( encodedInput    ,
                                outputImage     ,
                                m_codecTypeName );
}

/*-------------------------------------------------------------------------*/

bool
CPluginImageCodecItem::DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                       CORE::CValueList& metaData    )
{GUCEF_TRACE;


    return m_pluginPtr->DecodeMetaData( encodedInput    ,
                                        metaData        ,
                                        m_codecTypeName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPluginImageCodecItem::GetFamilyName( void ) const
{GUCEF_TRACE;

    return CImageCodecPlugin::ImageCodecFamilyName;
}

/*-------------------------------------------------------------------------*/

UInt32
CPluginImageCodecItem::GetCapabilities( void ) const
{GUCEF_TRACE;

    // the capabilities are those the plugin states for the codec in its codec link
    CORE::CICodecPlugin::CCodecSet codecs;
    TImageCodecPluginPtr plugin( m_pluginPtr );
    if ( plugin->GetCodecs( codecs ) )
    {
        CORE::CICodecPlugin::CCodecSet::const_iterator family = codecs.find( CImageCodecPlugin::ImageCodecFamilyName );
        if ( codecs.end() != family )
        {
            CORE::CICodecPlugin::CCodecFamilySet::const_iterator codec = (*family).second.find( m_codecTypeName );
            if ( (*family).second.end() != codec && !(*codec).second.IsNULL() )
                return (*codec).second->GetCapabilities();
        }
    }
    return CIImageCodec::GetCapabilities();
}

/*-------------------------------------------------------------------------*/

UInt32
CPluginImageCodecItem::GetImageCapabilities( void ) const
{GUCEF_TRACE;

    return 0 != ( GetCapabilities() & GUCEF_CODEC_CAPABILITY_MULTIPLE_FRAMES ) ? IMAGECAPABILITY_MULTIPLE_FRAMES : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPluginImageCodecItem::GetType( void ) const
{GUCEF_TRACE;

    return m_codecTypeName;
}

/*-------------------------------------------------------------------------*/

const CString&
CPluginImageCodecItem::GetClassTypeName( void ) const
{
    static CString classTypeName = "GUCEF::IMAGE::CPluginImageCodecItem";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPluginImageCodecItem::Clone( void ) const
{
    return new CPluginImageCodecItem( *this );
}
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
