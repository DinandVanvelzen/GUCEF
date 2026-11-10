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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H
#include "fontpluginFREETYPE_CFreeTypeFontRasterizer.h"
#define GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H
#endif /* GUCEF_FONTPLUGINFREETYPE_CFREETYPEFONTRASTERIZER_H ? */

#include "fontpluginFREETYPE_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace FONTPLUGINFREETYPE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CFreeTypeFontRasterizer* g_rasterizer = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
FONTPLUGINFREETYPE_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    g_rasterizer = GUCEF_NEW CFreeTypeFontRasterizer();
    if ( !g_rasterizer->IsInitialized() || !GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().Register( g_rasterizer ) )
    {
        GUCEF_DELETE g_rasterizer;
        g_rasterizer = GUCEF_NULL;
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
FONTPLUGINFREETYPE_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != g_rasterizer )
        GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().Unregister( g_rasterizer->GetName() );
    GUCEF_DELETE g_rasterizer;
    g_rasterizer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
FONTPLUGINFREETYPE_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
FONTPLUGINFREETYPE_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
FONTPLUGINFREETYPE_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Font rasterizer for TrueType and OpenType fonts using FreeType";
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FONTPLUGINFREETYPE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
