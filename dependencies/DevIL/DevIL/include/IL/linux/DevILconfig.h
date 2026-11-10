#ifndef __CONFIG_H__
#define __CONFIG_H__

// DV edit: this is the configuration for Linux, the Windows specifics that were here do not apply
// #define IL_USE_PRAGMA_LIBS  // Links to only the libraries that are requested, MSVC only.
// #define IL_INLINE_ASM 1  // Define if you can support at least some ASM, the asm of DevIL is written for MSVC

// Supported images formats (IL)

// #define IL_NO_BLP
// #define IL_NO_BMP
// #define IL_NO_CUT
// #define IL_NO_CHEAD
// #define IL_NO_DCX
// #define IL_NO_DDS
// #define IL_NO_DICOM
// #define IL_NO_DOOM
// #define IL_NO_EXR
// #define IL_NO_FITS
// #define IL_NO_FTX
// #define IL_NO_GIF
// #define IL_NO_HDR
// #define IL_NO_ICO
// #define IL_NO_ICNS
// #define IL_NO_IWI
// #define IL_NO_JP2
// #define IL_NO_JPG
// #define IL_NO_LCMS
// #define IL_NO_LIF
// #define IL_NO_MDL
// #define IL_NO_MNG
// #define IL_NO_PCD
// #define IL_NO_PCX
// #define IL_NO_PIC
// #define IL_NO_PIX
// #define IL_NO_PNG
// #define IL_NO_PNM
// #define IL_NO_PSD
// #define IL_NO_PSP
// #define IL_NO_PXR
// #define IL_NO_RAW
// #define IL_NO_ROT
// #define IL_NO_SGI
// #define IL_NO_SUN
// #define IL_NO_TGA
// #define IL_NO_TIF
// #define IL_NO_TPL
// #define IL_NO_WAL
#define IL_NO_WDP   // DV edit: needs the JPEG XR library which is not part of this build
// #define IL_NO_XPM

#define IL_USE_JPEGLIB_UNMODIFIED 1
// #define IL_USE_DXTC_NVIDIA   // DV edit: needs the nvtt library, squish is used instead
#define IL_USE_DXTC_SQUISH

//#define IL_NO_GAMES

/* Supported api (ilut) */


//
// sorry just
// cant get this one to work under windows
// have disabled for the now
//
// will look at it some more later
//
// Kriss
//
#undef ILUT_USE_ALLEGRO

#undef ILUT_USE_DIRECTX8
#undef ILUT_USE_DIRECTX9
#undef ILUT_USE_DIRECTX10
#define ILUT_USE_OPENGL
#undef ILUT_USE_SDL
#undef ILUT_USE_WIN32


#endif /* __CONFIG_H__ */
