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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#include "gucefIMAGE_CImageCodecCapabilities.h"

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

namespace {

const char* const BasicImageCodecFamily = "ImageCodec";

/*-------------------------------------------------------------------------*/

void
FillFromCodec( const CORE::CICodec& codec, CImageCodecCapabilities::SImageCodecInfo& info )
{GUCEF_TRACE;

    const UInt32 capabilities = codec.GetCapabilities();
    info.canEncode = 0 != ( capabilities & GUCEF_CODEC_CAPABILITY_ENCODE );
    info.canDecode = 0 != ( capabilities & GUCEF_CODEC_CAPABILITY_DECODE );
}

/*-------------------------------------------------------------------------*/

bool
IsLessByType( const CImageCodecCapabilities::SImageCodecInfo& left, const CImageCodecCapabilities::SImageCodecInfo& right )
{GUCEF_TRACE;

    return left.type < right.type;
}

}

/*-------------------------------------------------------------------------*/

bool
CImageCodecCapabilities::TryGetCodec( const CORE::CString& type ,
                                      SImageCodecInfo& info     )
{GUCEF_TRACE;

    info = SImageCodecInfo();
    info.type = type.Lowercase();

    // the same order as CImage::Load() and CImage::Save()
    CImageCodecRegistry::TImageCodecPtr imageCodec;
    if ( CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( type, imageCodec, false ) && !imageCodec.IsNULL() )
    {
        FillFromCodec( *imageCodec, info );
        const UInt32 imageCapabilities = imageCodec->GetImageCapabilities();
        info.multipleFrames = 0 != ( imageCapabilities & CIImageCodec::IMAGECAPABILITY_MULTIPLE_FRAMES );
        info.mipLevels = 0 != ( imageCapabilities & CIImageCodec::IMAGECAPABILITY_MIP_LEVELS );
        return true;
    }

    CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( BasicImageCodecFamily, family, true ) )
    {
        CORE::CCodecRegistry::TICodecPtr codec;
        if ( family->TryLookup( type, codec, false ) && !codec.IsNULL() )
        {
            FillFromCodec( *codec, info );
            info.multipleFrames = 0 != ( codec->GetCapabilities() & GUCEF_CODEC_CAPABILITY_MULTIPLE_FRAMES );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CImageCodecCapabilities::GetCodecs( TImageCodecInfoVector& codecs )
{GUCEF_TRACE;

    codecs.clear();

    CORE::CString::StringVector types;
    CImageGlobal::Instance()->GetImageCodecRegistry().GetRegisteredObjNames( types );

    CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( BasicImageCodecFamily, family, true ) )
    {
        CORE::CString::StringVector basicTypes;
        family->GetRegisteredObjNames( basicTypes );
        types.insert( types.end(), basicTypes.begin(), basicTypes.end() );
    }

    for ( size_t i=0; i<types.size(); ++i )
    {
        SImageCodecInfo info;
        if ( !TryGetCodec( types[ i ], info ) )
            continue;

        bool known = false;
        for ( size_t n=0; n<codecs.size(); ++n )
            known = known || codecs[ n ].type == info.type;
        if ( !known )
            codecs.push_back( info );
    }
    std::sort( codecs.begin(), codecs.end(), IsLessByType );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
