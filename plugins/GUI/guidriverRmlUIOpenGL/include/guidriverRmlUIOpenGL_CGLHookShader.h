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

#ifndef GUCEF_GUIDRIVERRMLUIGL_CGLHOOKSHADER_H
#define GUCEF_GUIDRIVERRMLUIGL_CGLHOOKSHADER_H

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
 *  A filter for a picture written in the user shader format of the mpv media player, so the many filters made
 *  for it (FSR, Anime4K, pixel art scalers ...) can be used as they are. A file has passes, each a block of
 *  directives and a GLSL function 'vec4 hook()'; parameters are blocks of their own:
 *
 *      //!DESC text            what the pass does
 *      //!HOOK MAIN            the texture the pass works on; only MAIN (the picture) is supported
 *      //!BIND name            a texture the pass reads: HOOKED, MAIN or one an earlier pass saved
 *      //!SAVE name            where the result goes; without it, or with MAIN, it replaces the picture
 *      //!WIDTH expression     the size of the result, an expression in reverse polish notation over numbers and
 *      //!HEIGHT expression    name.w / name.h of textures, OUTPUT (the size the picture is shown at) included
 *      //!WHEN expression      the pass runs only when the expression is not 0
 *      //!COMPONENTS n         (results are always RGBA)
 *      //!PARAM name           a parameter, with //!TYPE float or int, //!MINIMUM, //!MAXIMUM, //!DESC and the
 *                              default value on the line after the directives
 *
 *  For every bound texture the code has NAME_tex( pos ), NAME_texOff( offset ), NAME_pos, NAME_size, NAME_pt and
 *  NAME_mul, and target_size is the size of the result of the pass. Operators in expressions: + - * / > < = !.
 *
 *  Results between passes are half float textures where the GL can render to them (filters like Anime4K need
 *  values outside 0 .. 1 there), RGBA8 otherwise.
 */
class GUIDRIVERRMLUIGL_PUBLIC_CPP CGLHookShader
{
    public:

    typedef GUCEF::map< CORE::CString, CORE::CString > TParameterMap;
    typedef GUCEF::map< CORE::CString, Float32 > TSizeMap;   /**< "NAME.w" and "NAME.h" by texture */

    CGLHookShader( void );

    ~CGLHookShader();

    /** Reads the passes and parameters of a shader file; false when it has no pass this class can run */
    bool Parse( const CORE::CString& source, const CORE::CString& name );

    const CORE::CString& GetName( void ) const;

    UInt32 GetPassCount( void ) const;

    const CORE::CString& GetPassDescription( UInt32 passIndex ) const;

    bool HasParameter( const CORE::CString& name ) const;

    /**
     *  Runs the passes on the picture in 'sourceTexture' for a picture shown at outputWidth x outputHeight.
     *  'resultTexture' is the picture after the passes (the source texture when no pass ran) and stays valid
     *  until the next Run() or Release(). The GL context of the textures has to be current.
     */
    bool Run( UInt32 sourceTexture        ,
              UInt32 sourceWidth          ,
              UInt32 sourceHeight         ,
              UInt32 outputWidth          ,
              UInt32 outputHeight         ,
              const TParameterMap& params ,
              UInt32& resultTexture       ,
              UInt32& resultWidth         ,
              UInt32& resultHeight        );

    /** Lets go of the GL objects; the GL context has to be current */
    void Release( void );

    /** Evaluates an expression in reverse polish notation; false when it cannot be evaluated */
    static bool EvaluateExpression( const CORE::CString& expression, const TSizeMap& sizes, Float32& result );

    /** Whether the current GL context renders to half float textures; the context has to be current */
    static bool SupportsFloatTargets( void );

    private:

    struct SPass
    {
        CORE::CString description;
        CORE::CString hook;
        GUCEF::vector< CORE::CString > binds;
        CORE::CString save;
        CORE::CString widthExpression;
        CORE::CString heightExpression;
        CORE::CString whenExpression;
        CORE::CString code;
        UInt32 program;
    };
    typedef struct SPass TPass;

    struct SParameter
    {
        CORE::CString name;
        bool isInteger;
        Float32 defaultValue;
        Float32 minimum;
        Float32 maximum;
    };
    typedef struct SParameter TParameter;

    struct STexture
    {
        UInt32 texture;
        UInt32 framebuffer;
        UInt32 width;
        UInt32 height;
        bool inUse;
    };
    typedef struct STexture TTexture;

    bool PreparePass( TPass& pass );

    /** A texture of the size to render to that is not in use, by index in m_targets; -1 when none can be made */
    Int32 GetTarget( UInt32 width, UInt32 height );

    CGLHookShader( const CGLHookShader& src );
    CGLHookShader& operator=( const CGLHookShader& src );

    private:

    CORE::CString m_name;
    GUCEF::vector< TPass > m_passes;
    GUCEF::vector< TParameter > m_parameters;
    GUCEF::vector< TTexture > m_targets;
    UInt32 m_vertexArray;
    bool m_floatTargets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUIGL_CGLHOOKSHADER_H ? */
