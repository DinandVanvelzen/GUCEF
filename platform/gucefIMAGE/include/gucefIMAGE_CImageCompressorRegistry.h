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

#ifndef GUCEF_IMAGE_CIMAGECOMPRESSORREGISTRY_H
#define GUCEF_IMAGE_CIMAGECOMPRESSORREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_IMAGE_CIMAGECOMPRESSOR_H
#include "gucefIMAGE_CImageCompressor.h"
#define GUCEF_IMAGE_CIMAGECOMPRESSOR_H
#endif /* GUCEF_IMAGE_CIMAGECOMPRESSOR_H ? */

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
 *  The image compressors there are, by name. A plugin registers its compressor when it loads and unregisters
 *  it before it unloads; the compressor stays the plugin's.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageCompressorRegistry
{
    public:

    typedef GUCEF::vector< CORE::CString > TNameVector;

    CImageCompressorRegistry( void );

    ~CImageCompressorRegistry();

    bool Register( CImageCompressor* compressor );

    bool Unregister( const CORE::CString& name );

    CImageCompressor* Get( const CORE::CString& name ) const;

    /** The first registered compressor that offers the block format, NULL when none does */
    CImageCompressor* FindByFormat( const CORE::CString& format ) const;

    /** The names of the compressors, in the order they were registered */
    TNameVector GetNames( void ) const;

    private:

    CImageCompressorRegistry( const CImageCompressorRegistry& src );
    CImageCompressorRegistry& operator=( const CImageCompressorRegistry& src );

    private:

    typedef GUCEF::vector< CImageCompressor* > TCompressorVector;

    TCompressorVector m_compressors;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECOMPRESSORREGISTRY_H ? */
