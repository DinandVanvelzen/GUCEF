/*
 * Hand maintained equivalent of the MyGUI_Config.h which the upstream CMake build generates from
 * MyGUI_Config.h.in. The module definition decides what is on, the preprocessor selects the rest.
 */

#ifndef MYGUI_CONFIG_H_
#define MYGUI_CONFIG_H_

// MYGUI_STATIC:
//   Define when building or using MyGUIEngine as a static library.
//   Disables Windows dllimport/dllexport attributes. WebAssembly builds are static.
#if defined(__EMSCRIPTEN__)
	#ifndef MYGUI_STATIC
		#define MYGUI_STATIC
	#endif
#endif

// MYGUI_DONT_USE_OBSOLETE:
//   When defined, deprecated APIs are excluded from the library: not defined, they are [[deprecated]]

// MYGUI_USE_FREETYPE:
//   TrueType font support via FreeType (the freetype module of this code base)
#ifndef MYGUI_USE_FREETYPE
	#define MYGUI_USE_FREETYPE
#endif

// MYGUI_MSDF_FONTS:
//   Multi-channel signed distance field fonts, generated at run time from a TrueType font (font property
//   MsdfMode). Needs msdfgen (the msdfgen-core and msdfgen-ext modules of this code base), implies MYGUI_USE_FREETYPE.
//   The fonts also need a render platform with the MSDF shader registered: OpenGL 3, OpenGL ES and Ogre have one.
#ifndef MYGUI_MSDF_FONTS
    #define MYGUI_MSDF_FONTS
#endif

#endif // MYGUI_CONFIG_H_
