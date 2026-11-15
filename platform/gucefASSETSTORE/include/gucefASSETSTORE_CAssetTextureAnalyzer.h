/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H
#define GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#include "gucefASSETSTORE_CIAssetContentAnalyzer.h"
#define GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONTENTANALYZER_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The built in content analyser of images (section 7.3): textures, decals, HDRIs, icons and sprites. The size,
 *  channels, bit depth and compression come from the header of the file where CAssetImageHeaderProbe knows the format
 *  (PNG, JPEG, TGA, BMP, GIF, DDS), so the image is not decoded; other formats are decoded through the image codecs of
 *  gucefIMAGE (codec plugins). The role comes from the name hints; an 8 bit colour image that the analyser decoded
 *  and whose name does not tell is taken for a normal map when its colours average to the blue of a tangent space
 *  normal map. Albedo and emissive textures are sRGB, the rest linear.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetTextureAnalyzer : public CIAssetContentAnalyzer
{
    public:

    CAssetTextureAnalyzer( void );
    virtual ~CAssetTextureAnalyzer() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanAnalyze( const CAssetRecord& asset ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TAssetStoreStatus Analyze( const CORE::CString& vfsPath, CAssetRecord& asset ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H ? */
