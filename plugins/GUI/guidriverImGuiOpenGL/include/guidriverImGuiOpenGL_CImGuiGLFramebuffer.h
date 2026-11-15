/*
 *  guidriverImGuiOpenGL: gucefGUI driver ImGui: Dear ImGui drawn with OpenGL
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

#ifndef GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H
#define GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */

#ifndef GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H
#include "guidriverImGuiOpenGL_CImGuiGLFunctions.h"
#define GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H
#endif /* GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERIMGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An offscreen RGBA8 colour target of a size: a framebuffer object with a texture attached. The driver draws the
 *  last frame into it for captures and reads it back. The GL context of the window must be current for every call.
 */
class GUIDRIVERIMGUIGL_PUBLIC_CPP CImGuiGLFramebuffer
{
    public:

    explicit CImGuiGLFramebuffer( const CImGuiGLFunctions& functions );

    /** Lets go of the GL objects: the GL context must still be there */
    ~CImGuiGLFramebuffer();

    /** Makes the target of this size, or keeps the one that has it. False, with the reason in the log, when it cannot be made */
    bool Prepare( UInt32 width, UInt32 height );

    /** Deletes the GL objects */
    void Release( void );

    bool IsReady( void ) const;

    UInt32 GetWidth( void ) const;

    UInt32 GetHeight( void ) const;

    UInt32 GetFramebuffer( void ) const;

    UInt32 GetColourTexture( void ) const;

    /** Makes it the target of drawing (GL_FRAMEBUFFER) with the viewport over all of it */
    bool Bind( void );

    /** Clears it to a colour */
    void Clear( float red, float green, float blue, float alpha );

    /**
     *  Reads the colours as they are into an RGBA picture with the rows from the top, as the image classes have them. The
     *  picture is made when it is not there or has another size or format
     */
    bool ReadPixels( IMAGE::TPixelMapPtr& pixels );

    /** Reads the framebuffer with the name given (the window's, 0 by default), 'width' by 'height' pixels from the lower left corner */
    static bool ReadFramebufferPixels( const CImGuiGLFunctions& functions ,
                                       UInt32 framebuffer                 ,
                                       UInt32 width                       ,
                                       UInt32 height                      ,
                                       IMAGE::TPixelMapPtr& pixels        );

    private:

    CImGuiGLFramebuffer( const CImGuiGLFramebuffer& src );
    CImGuiGLFramebuffer& operator=( const CImGuiGLFramebuffer& src );

    private:

    CImGuiGLFunctions m_gl;
    UInt32 m_framebuffer;
    UInt32 m_colourTexture;
    UInt32 m_width;
    UInt32 m_height;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_CIMGUIGLFRAMEBUFFER_H ? */
