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

#ifndef GUCEF_GUI_CFONTRASTERIZER_H
#define GUCEF_GUI_CFONTRASTERIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_GUI_CBITMAPFONT_H
#include "gucefGUI_CBitmapFont.h"
#define GUCEF_GUI_CBITMAPFONT_H
#endif /* GUCEF_GUI_CBITMAPFONT_H ? */

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
 *  Turns an outline font (TrueType, OpenType, ...) into a CBitmapFont at a given pixel size, so widgets
 *  draw it the same on every backend. Rasterizers come from plugins and register with the
 *  CFontRasterizerRegistry of CGuiGlobal; without one only bitmap fonts made by the application exist.
 */
class GUCEF_GUI_PUBLIC_CPP CFontRasterizer
{
    public:

    /** The code points first .. last, both included */
    struct SCodePointRange
    {
        UInt32 first;
        UInt32 last;

        SCodePointRange( void );
        SCodePointRange( UInt32 firstCodePoint, UInt32 lastCodePoint );
    };
    typedef struct SCodePointRange TCodePointRange;
    typedef GUCEF::vector< TCodePointRange > TCodePointRanges;

    /** Printable ASCII and Latin-1: what a user interface in a western language needs */
    static TCodePointRanges GetLatin1Ranges( void );

    /** What CGUIContext::RegisterFont() rasterizes: Latin-1 and the bullet (U+2022), the mask character of the secret input */
    static TCodePointRanges GetUiRanges( void );

    CFontRasterizer( void );
    virtual ~CFontRasterizer();

    /** The name it registers under, like "FreeType" */
    virtual CString GetName( void ) const = 0;

    /** Whether the font data looks like a font this rasterizer reads, from its first bytes */
    virtual bool CanRasterize( const CORE::CDynamicBuffer& fontData ) const = 0;

    /**
     *  Rasterizes the glyphs of the code point ranges the font has. The atlas holds the glyphs in white
     *  with their coverage in alpha. A '?' glyph, when there is one, is the fallback for code points the
     *  font lacks. 'pixelSize' is the height of the em square in pixels.
     *
     *  @return the font, NULL when the data is not a font this rasterizer can read
     */
    virtual TBitmapFontPtr Rasterize( const CORE::CDynamicBuffer& fontData ,
                                      UInt32 pixelSize                     ,
                                      const TCodePointRanges& ranges       ) = 0;

    private:

    CFontRasterizer( const CFontRasterizer& src );
    CFontRasterizer& operator=( const CFontRasterizer& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFONTRASTERIZER_H ? */
