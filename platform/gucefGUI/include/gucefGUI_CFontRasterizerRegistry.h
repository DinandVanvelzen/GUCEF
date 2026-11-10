/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZER_H
#include "gucefGUI_CFontRasterizer.h"
#define GUCEF_GUI_CFONTRASTERIZER_H
#endif /* GUCEF_GUI_CFONTRASTERIZER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The font rasterizers the loaded plugins provide, by name. A plugin registers its rasterizer when it
 *  loads and unregisters it before it unloads; the rasterizer stays the plugin's.
 */
class GUCEF_GUI_PUBLIC_CPP CFontRasterizerRegistry
{
    public:

    typedef CFontRasterizer::TCodePointRanges TCodePointRanges;
    typedef GUCEF::set< CString > TStringSet;

    CFontRasterizerRegistry( void );
    ~CFontRasterizerRegistry();

    bool Register( CFontRasterizer* rasterizer );
    bool Unregister( const CString& name );

    CFontRasterizer* Get( const CString& name ) const;
    bool HasRasterizers( void ) const;
    TStringSet GetNames( void ) const;

    /**
     *  Rasterizes the font data with the first rasterizer that can read it
     *
     *  @return the font, NULL when no loaded rasterizer reads this data
     */
    TBitmapFontPtr RasterizeFont( const CORE::CDynamicBuffer& fontData ,
                                  UInt32 pixelSize                     ,
                                  const TCodePointRanges& ranges       ) const;

    /**
     *  Reads the font file through the VFS and rasterizes it
     *
     *  @return the font, NULL when the file cannot be read or no loaded rasterizer reads it
     */
    TBitmapFontPtr RasterizeFont( const CString& vfsPath           ,
                                  UInt32 pixelSize                 ,
                                  const TCodePointRanges& ranges   ) const;

    private:

    CFontRasterizerRegistry( const CFontRasterizerRegistry& src );
    CFontRasterizerRegistry& operator=( const CFontRasterizerRegistry& src );

    private:

    typedef GUCEF::map< CString, CFontRasterizer* > TRasterizerMap;

    TRasterizerMap m_rasterizers;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */
