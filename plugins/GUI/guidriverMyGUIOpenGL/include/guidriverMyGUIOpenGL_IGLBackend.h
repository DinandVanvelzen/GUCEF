/*
 *  guidriverMyGUIOpenGL: GUI backend using MyGUI with OpenGL, OpenGL 3 or OpenGL ES 3 / WebGL 2
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

#ifndef GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H
#define GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_guidriverMyGUIOpenGL_MACROS_H
#include "guidriverMyGUIOpenGL_macros.h"
#define GUCEF_guidriverMyGUIOpenGL_MACROS_H
#endif /* GUCEF_guidriverMyGUIOpenGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUIOpenGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One of the OpenGL platforms of MyGUI as the context uses it: the legacy OpenGL one, the OpenGL 3 one
 *  (core profile, GLSL 330) or the OpenGL ES 3 one (GLSL ES 300, also WebGL 2). Each is a class of its own
 *  in MyGUI with the same use, this interface is what they have in common. The images of MyGUI are loaded
 *  with gucefIMAGE from the VFS.
 */
class GUCEF_guidriverMyGUIOpenGL_PUBLIC_CPP IGLBackend
{
    public:

    virtual ~IGLBackend() {}

    /** Starts the render manager of the platform; the GL context must be current and the data manager there */
    virtual bool Initialise( void ) = 0;

    /** Stops the render manager; MyGUI itself must have been shut down before */
    virtual void Shutdown( void ) = 0;

    /** Lets MyGUI draw its widgets into the framebuffer that is bound */
    virtual void DrawOneFrame( void ) = 0;

    /** Whether the platform reads the bytes of "R8G8B8A8" textures as B, G, R, A (else R, G, B, A) */
    virtual bool IsBgraMemoryOrder( void ) const = 0;

    /** Which platform of MyGUI it is: "OpenGL3", "OpenGL" or "OpenGLES" */
    virtual const char* GetName( void ) const = 0;

    /**
     *  Whether the platform has a shader for MSDF fonts: the OpenGL 3 and OpenGL ES platforms do, the legacy
     *  OpenGL platform has no shaders at all
     */
    virtual bool SupportsMsdfFonts( void ) const = 0;

    /** Registers the MSDF font shader with the render manager (the GL context must be current); false when it fails or there is none */
    virtual bool RegisterMsdfShader( const char* shaderName ) = 0;
};

/*-------------------------------------------------------------------------*/

/** The backends; NULL when the platform was not built for this target */
IGLBackend* CreateGL3Backend( void );
IGLBackend* CreateGLBackend( void );
IGLBackend* CreateGLESBackend( void );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUIOpenGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUIOpenGL_IGLBACKEND_H ? */
