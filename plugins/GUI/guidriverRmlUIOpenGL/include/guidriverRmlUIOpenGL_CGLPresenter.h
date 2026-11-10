/*
 *  guidriverRmlUIOpenGL: GUI backend using RmlUi drawing with OpenGL 3.3, OpenGL ES 3 or WebGL 2
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

#ifndef GUCEF_GUIDRIVERRMLUIGL_CGLPRESENTER_H
#define GUCEF_GUIDRIVERRMLUIGL_CGLPRESENTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#include "guidriverRmlUIOpenGL_macros.h"
#define GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#endif /* GUCEF_GUIDRIVERRMLUIGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The present pass: draws a picture (a texture) into a rectangle of a window's framebuffer with a shader, scaled
 *  with nearest or linear sampling, optionally blended over what the window already shows.
 *
 *  Works with OpenGL 3.3, OpenGL ES 3 and WebGL 2. The GL context the presenter is used in has to be current
 *  for every call, Release() included; the GL objects are made at the first use.
 */
class GUIDRIVERRMLUIGL_PUBLIC_CPP CGLPresenter
{
    public:

    CGLPresenter( void );

    ~CGLPresenter();

    /**
     *  Draws the texture into 'area' (window coordinates, top left origin) of the target framebuffer, whose
     *  size is that of the window. With 'blend' the picture is drawn over the window with its alpha
     *  (premultiplied), else it replaces what is there
     */
    bool Present( UInt32 sourceTexture       ,
                  UInt32 targetFramebuffer   ,
                  Int32 windowWidth          ,
                  Int32 windowHeight         ,
                  const GUI::CRect& area     ,
                  bool linear                ,
                  bool blend                 );

    /** Lets go of the GL objects; the GL context has to be current */
    void Release( void );

    private:

    bool PrepareCopyProgram( void );

    CGLPresenter( const CGLPresenter& src );
    CGLPresenter& operator=( const CGLPresenter& src );

    private:

    UInt32 m_copyProgram;
    Int32 m_sourceLocation;
    UInt32 m_vertexArray;
    bool m_failed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUIGL_CGLPRESENTER_H ? */
