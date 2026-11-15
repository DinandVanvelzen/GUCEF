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

#ifndef GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H
#define GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUIDRIVERIMGUIGL_MACROS_H
#include "guidriverImGuiOpenGL_macros.h"
#define GUIDRIVERIMGUIGL_MACROS_H
#endif /* GUIDRIVERIMGUIGL_MACROS_H ? */

/*
 *  The calling convention of the GL entry points: stdcall on Windows, the default elsewhere
 */
#if defined( _WIN32 )
  #define GUIDRIVERIMGUIGL_APIENTRY __stdcall
#else
  #define GUIDRIVERIMGUIGL_APIENTRY
#endif

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
 *  The entry points of OpenGL (OpenGL ES) that the driver calls itself, apart from the ones of the backend of Dear ImGui
 *  (the framebuffer of a capture, the clearing of the window, reading pixels back). Plain function pointers over the
 *  types of GL, which are the same for OpenGL and OpenGL ES, so that no GL header is needed.
 *
 *  Where the plugin is linked to OpenGL ES (Android, the web) the entry points are the linked functions; elsewhere
 *  they come from the GL_GETPROCADDRESS property of a window.
 *
 *  Also feeds the loader of the backend of Dear ImGui with the same function (desktop OpenGL), so that the backend
 *  never opens libGL or libEGL itself.
 */
class GUIDRIVERIMGUIGL_PUBLIC_CPP CImGuiGLFunctions
{
    public:

    /** What GL_GETPROCADDRESS of a window points to */
    typedef void* ( *TGetProcAddress )( const char* name );

    /** The constants of GL that the driver uses */
    enum EGLConstant
    {
        GLC_FALSE                       = 0         ,
        GLC_TRUE                        = 1         ,
        GLC_COLOR_BUFFER_BIT            = 0x4000    ,
        GLC_SCISSOR_TEST                = 0x0C11    ,
        GLC_TEXTURE_2D                  = 0x0DE1    ,
        GLC_UNSIGNED_BYTE               = 0x1401    ,
        GLC_RGBA                        = 0x1908    ,
        GLC_RGBA8                       = 0x8058    ,
        GLC_NEAREST                     = 0x2600    ,
        GLC_TEXTURE_MAG_FILTER          = 0x2800    ,
        GLC_TEXTURE_MIN_FILTER          = 0x2801    ,
        GLC_TEXTURE_WRAP_S              = 0x2802    ,
        GLC_TEXTURE_WRAP_T              = 0x2803    ,
        GLC_CLAMP_TO_EDGE               = 0x812F    ,
        GLC_PACK_ALIGNMENT              = 0x0D05    ,
        GLC_VENDOR                      = 0x1F00    ,
        GLC_RENDERER                    = 0x1F01    ,
        GLC_VERSION                     = 0x1F02    ,
        GLC_FRAMEBUFFER                 = 0x8D40    ,
        GLC_READ_FRAMEBUFFER            = 0x8CA8    ,
        GLC_FRAMEBUFFER_BINDING         = 0x8CA6    ,
        GLC_READ_FRAMEBUFFER_BINDING    = 0x8CAA    ,
        GLC_COLOR_ATTACHMENT0           = 0x8CE0    ,
        GLC_FRAMEBUFFER_COMPLETE        = 0x8CD5    ,
        GLC_VIEWPORT                    = 0x0BA2
    };
    typedef enum EGLConstant TGLConstant;

    typedef const unsigned char* ( GUIDRIVERIMGUIGL_APIENTRY *TGetString )( unsigned int name );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TGetIntegerv )( unsigned int name, int* data );
    typedef unsigned int ( GUIDRIVERIMGUIGL_APIENTRY *TGetError )( void );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TGenObjects )( int count, unsigned int* objects );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TDeleteObjects )( int count, const unsigned int* objects );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TBindObject )( unsigned int target, unsigned int object );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TFramebufferTexture2D )( unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level );
    typedef unsigned int ( GUIDRIVERIMGUIGL_APIENTRY *TCheckFramebufferStatus )( unsigned int target );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TTexImage2D )( unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TTexParameteri )( unsigned int target, unsigned int name, int value );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TReadPixels )( int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TPixelStorei )( unsigned int name, int value );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TViewport )( int x, int y, int width, int height );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TClearColor )( float red, float green, float blue, float alpha );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TClear )( unsigned int mask );
    typedef void ( GUIDRIVERIMGUIGL_APIENTRY *TCapability )( unsigned int capability );

    TGetString GetString;
    TGetIntegerv GetIntegerv;
    TGetError GetError;
    TGenObjects GenFramebuffers;
    TDeleteObjects DeleteFramebuffers;
    TBindObject BindFramebuffer;
    TFramebufferTexture2D FramebufferTexture2D;
    TCheckFramebufferStatus CheckFramebufferStatus;
    TGenObjects GenTextures;
    TDeleteObjects DeleteTextures;
    TBindObject BindTexture;
    TTexImage2D TexImage2D;
    TTexParameteri TexParameteri;
    TReadPixels ReadPixels;
    TPixelStorei PixelStorei;
    TViewport Viewport;
    TClearColor ClearColor;
    TClear Clear;
    TCapability Disable;

    CImGuiGLFunctions( void );

    /** True when every entry point is there */
    bool IsComplete( void ) const;

    /**
     *  Takes the entry points: the linked ones with OpenGL ES, else the ones that the function of a window gives. Also
     *  hands the function to the loader of the backend. False, with the first missing entry point in the log, when
     *  one is missing
     */
    bool Load( TGetProcAddress getProcAddress );

    /**
     *  Makes sure the loader of the backend of Dear ImGui has entry points (the backend lets go of them when one of its
     *  contexts shuts down, and would then open libGL itself): call before every use of the backend. Nothing to do
     *  with OpenGL ES, where the backend calls the linked functions
     */
    static bool EnsureBackendLoader( TGetProcAddress getProcAddress );

    /** True when this build is compiled for OpenGL ES (Android, the web): GLSL "#version 300 es" and no desktop OpenGL */
    static bool IsOpenGlEsBuild( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERIMGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERIMGUIGL_CIMGUIGLFUNCTIONS_H ? */
