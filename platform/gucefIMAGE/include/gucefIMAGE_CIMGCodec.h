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

#ifndef GUCEF_IMAGE_CIMGCODEC_H
#define GUCEF_IMAGE_CIMGCODEC_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF { namespace CORE { class CIOAccess; } }

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

class CImage;

/*-------------------------------------------------------------------------*/

/**
 *  Utility conversion class for image codec's
 *  It adds some utility member functions that make it easier to use 
 *  the interface when dealing with images
 *
 *  Note that decoded image data is expected to be a blob of the header
 *  info of type TImageInfo followed by the pixel data. The pixel data is
 *  defined as ordered in the following hierarchy:
 *      - frame
 *          - mipmap levels
 *  Each pixel segment is ordered with the bottom left pixel as the first 
 *  pixel in the segment stored as a series of horizontal scan lines.
 */
class GUCEF_IMAGE_EXPORT_CPP CIMGCodec
{
    public:
    
    typedef CORE::CCodecRegistry::TICodecPtr   CCodecPtr;
    
    /**
     *  Constructs and links the conversion object
     *
     *  @throw EInvalidCodec thrown if the given codec is not of the "ImageCodec" family
     */
    CIMGCodec( const CCodecPtr& codecPtr );
    
    CIMGCodec( const CIMGCodec& src );
    
    ~CIMGCodec();
    
    CIMGCodec& operator=( const CIMGCodec& src );

    bool Encode( const CImage& inputImage       ,
                 CORE::CIOAccess& encodedOutput );
                     
    bool Decode( CORE::CIOAccess& encodedInput ,
                 CImage& outputImage           );
                 
    bool Encode( CORE::CIOAccess& source ,
                 CORE::CIOAccess& dest   );

    bool Decode( CORE::CIOAccess& source ,
                 CORE::CIOAccess& dest   );
                 
    CORE::CString GetFamilyName( void ) const;
    
    CORE::CString GetType( void ) const;
    
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_IMAGE_EXPORT_CPP, EInvalidCodec );
    
    private:
    
    CIMGCodec( void );
    
    private:
    
    CCodecPtr m_codecPtr;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMGCODEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Dinand: Added this class

-----------------------------------------------------------------------------*/