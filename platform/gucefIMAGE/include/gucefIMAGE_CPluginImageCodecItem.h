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

#ifndef GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H
#define GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CIIMAGECODEC_H
#include "gucefIMAGE_CIImageCodec.h"
#define GUCEF_IMAGE_CIIMAGECODEC_H
#endif /* GUCEF_IMAGE_CIIMAGECODEC_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#include "gucefIMAGE_CImageCodecPlugin.h"
#define GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#endif /* GUCEF_IMAGE_CIMAGECODECPLUGIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Implements the IImageCodec interface by redirecting the calls to
 *  a GucefImageCodecPlugin plugin module
 */
class CPluginImageCodecItem : public CIImageCodec
{
    public:

    /**
     *
     */
    CPluginImageCodecItem( TImageCodecPluginPtr pluginPtr ,
                           const CString& codecTypeName   );

    CPluginImageCodecItem( const CPluginImageCodecItem& src );

    virtual ~CPluginImageCodecItem() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( const CImage& inputImage       ,
                         CORE::CIOAccess& encodedOutput ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Decode( CORE::CIOAccess& encodedInput ,
                         CImage& outputImage           ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Encode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Decode( CORE::CIOAccess& source ,
                         CORE::CIOAccess& dest   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DecodeMetaData( CORE::CIOAccess& source    ,
                                 CORE::CValueList& metaData ) GUCEF_VIRTUAL_OVERRIDE;

    /** What the plugin states for the codec: encode and/or decode */
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** IMAGECAPABILITY_MULTIPLE_FRAMES when the plugin states GUCEF_CODEC_CAPABILITY_MULTIPLE_FRAMES for the codec; the plugin interface has nothing about mipmap levels */
    virtual UInt32 GetImageCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPluginImageCodecItem( void );
    CPluginImageCodecItem& operator=( const CPluginImageCodecItem& src );

    private:

    TImageCodecPluginPtr m_pluginPtr;
    CString m_codecTypeName;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPluginImageCodecItem, MT::CMutex > TPluginImageCodecItemPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
