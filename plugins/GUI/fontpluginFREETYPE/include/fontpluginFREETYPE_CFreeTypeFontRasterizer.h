/*
 *  fontpluginFREETYPE: GUCEF GUI plugin that rasterizes TrueType and OpenType fonts with FreeType
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

#ifndef GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H
#define GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFONTRASTERIZER_H
#include "gucefGUI_CFontRasterizer.h"
#define GUCEF_GUI_CFONTRASTERIZER_H
#endif /* GUCEF_GUI_CFONTRASTERIZER_H ? */

#ifndef GUCEF_FONTPLUGINFREETYPE_MACROS_H
#include "fontpluginFREETYPE_macros.h"
#define GUCEF_FONTPLUGINFREETYPE_MACROS_H
#endif /* GUCEF_FONTPLUGINFREETYPE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace FONTPLUGINFREETYPE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Font rasterizer "FreeType": TrueType, OpenType and font collections (the first face) into a bitmap font
 *  with anti-aliased glyphs, hinted the way FreeType does by default. Glyphs are packed in rows into one
 *  atlas with a pixel of space around each.
 */
class FONTPLUGINFREETYPE_PUBLIC_CPP CFreeTypeFontRasterizer : public GUI::CFontRasterizer
{
    public:

    CFreeTypeFontRasterizer( void );
    virtual ~CFreeTypeFontRasterizer();

    /** Whether FreeType started, a rasterizer that did not start rasterizes nothing */
    bool IsInitialized( void ) const;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CanRasterize( const CORE::CDynamicBuffer& fontData ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TBitmapFontPtr Rasterize( const CORE::CDynamicBuffer& fontData ,
                                           UInt32 pixelSize                     ,
                                           const TCodePointRanges& ranges       ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void* m_library;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FONTPLUGINFREETYPE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H ? */
