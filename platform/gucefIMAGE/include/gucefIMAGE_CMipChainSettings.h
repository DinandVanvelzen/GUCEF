/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_IMAGE_CMIPCHAINSETTINGS_H
#define GUCEF_IMAGE_CMIPCHAINSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_IMAGE_MACROS_H
#include "gucefIMAGE_macros.h"
#define GUCEF_IMAGE_MACROS_H
#endif /* GUCEF_IMAGE_MACROS_H ? */

#ifndef GUCEF_IMAGE_ETYPES_H
#include "gucefIMAGE_ETypes.h"
#define GUCEF_IMAGE_ETYPES_H
#endif /* GUCEF_IMAGE_ETYPES_H ? */

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
 *  How the CMipChainGenerator makes a mip chain. The settings round trip through a CDataNode as the section
 *  ConfigSectionName, with the values by name (see the ToString() functions of gucefIMAGE_ETypes.h).
 */
class GUCEF_IMAGE_EXPORT_CPP CMipChainSettings : public CORE::CIConfigurable
{
    public:

    /** The name of the config section: SaveConfig() writes it in the node it gets, LoadConfig() reads it from there */
    static const CORE::CString ConfigSectionName;

    TResampleFilter filter;          /**< default RESAMPLEFILTER_KAISER */
    TColourSpace colourSpace;        /**< the colour space of colour content, default COLOURSPACE_SRGB */
    TImageContent content;           /**< default IMAGECONTENT_COLOUR */
    TWrapMode wrapMode;              /**< default WRAPMODE_REPEAT */
    Float32 alphaCoverageCutoff;     /**< below 0: off (default); else the alpha test reference, for example 0.5 */
    UInt32 minimumSize;              /**< the smallest level's larger side, default 1 */
    bool normalMapSigned;            /**< false: normal map components are stored as 0..1 (default); true: as signed values */

    CMipChainSettings( void );

    CMipChainSettings( const CMipChainSettings& src );

    virtual ~CMipChainSettings();

    CMipChainSettings& operator=( const CMipChainSettings& src );

    /** Writes the settings as the section ConfigSectionName in 'tree' */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section ConfigSectionName of 'treeroot', or 'treeroot' itself when it is that section; what it lacks stays as it is */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CMIPCHAINSETTINGS_H ? */
