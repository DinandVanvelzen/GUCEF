/*===========================================================================
  xmlsbuf.h
    see parsifal.h for copyright info
===========================================================================*/
#ifndef XMLSBUF__H
#define XMLSBUF__H

#include <stddef.h>
#include "pns.h"
#include "xmlpool.h"

/******************************************************************************/

/*
 *  Dinand Vanvelzen edit:
 *  Added this to build easily with CMake
 */
 
#ifdef PARSIFAL_CUSTOM_GUCEF_BUILD 

#ifndef XMLAPI
#define XMLAPI
#endif

#define PARSIFAL_XMLAPI XMLAPI
 
#if !( defined(PARSIFAL_LINUX_BUILD) || defined(PARSIFAL_MSWIN_BUILD) )
  #if defined( WIN32 ) || defined( _WIN32 )
    #define PARSIFAL_MSWIN_BUILD
  #elif defined( linux ) || defined( __linux__ ) || defined( __EMSCRIPTEN__ )
    #define PARSIFAL_LINUX_BUILD
  #else
    #error Cannot automaticly detect your operating system, please define
  #endif
#endif /* MANUAL_OS_DEFINE ? */ 
 
#ifdef PARSIFAL_MSWIN_BUILD
    #define PARSIFAL_EXPORT __declspec( dllexport )
    #define PARSIFAL_IMPORT __declspec( dllimport )
#elif defined( __GNUC__ ) && ( __GNUC__ >= 4 )
    /* GUCEF builds use -fvisibility=hidden with GCC/Clang, as such exports must be explicitly marked */
    #define PARSIFAL_EXPORT __attribute__ ((visibility("default")))
    #define PARSIFAL_IMPORT
#else
    #define PARSIFAL_EXPORT
    #define PARSIFAL_IMPORT
#endif /* GUCEF_MSWIN_BUILD ? */

#undef XMLAPI
#ifdef PARSIFAL_BUILD_MODULE
  #define XMLAPI PARSIFAL_EXPORT
#else
  #define XMLAPI PARSIFAL_IMPORT
#endif

#endif /* PARSIFAL_CUSTOM_GUCEF_BUILD ? */

/******************************************************************************/

#ifndef XMLAPI
#define XMLAPI
#endif

#ifndef BYTE
 #define BYTE unsigned char 
#endif

#ifndef XMLCH_DEFINED
	#define XMLCH_DEFINED
	typedef unsigned char XMLCH;
#endif

#ifndef COUNTBUFSIZE
#define COUNTBUFSIZE(cBytes, blocksize) \
((!(cBytes)) ? (blocksize) : (!( (cBytes) % (blocksize) ) ? (int)(cBytes) : (int)( (((cBytes) / (blocksize)) + 1) * (blocksize) )) )
#endif

typedef struct tagXMLSTRINGBUF
{
	int capacity;
	int blocksize;
	int len;
	int usePool;
	XMLCH *str;
	LPXMLPOOL pool;
} XMLSTRINGBUF, *LPXMLSTRINGBUF;

#ifdef __cplusplus
   extern "C" {
#endif

XMLCH XMLAPI *XMLStringbuf_Append(LPXMLSTRINGBUF sbuf, XMLCH *str, int len);
XMLCH XMLAPI *XMLStringbuf_AppendCh(LPXMLSTRINGBUF sbuf, XMLCH c);
XMLCH XMLAPI *XMLStringbuf_Init(LPXMLSTRINGBUF sbuf, int blockSize, int initSize);
XMLCH XMLAPI *XMLStringbuf_InitUsePool(LPXMLSTRINGBUF sbuf, int blockSize, int initSize, LPXMLPOOL pool);
XMLCH XMLAPI *XMLStringbuf_ToString(LPXMLSTRINGBUF sbuf);
int XMLAPI XMLStringbuf_SetLength(LPXMLSTRINGBUF sbuf, int len);
void XMLAPI XMLStringbuf_Free(LPXMLSTRINGBUF sbuf);

/******************************************************************************/

/*
 *  Dinand Vanvelzen edit:
 *  Added this to build easily with CMake
 */

#ifdef PARSIFAL_CUSTOM_GUCEF_BUILD 

#undef XMLAPI
#define XMLAPI PARSIFAL_XMLAPI

#endif /* PARSIFAL_CUSTOM_GUCEF_BUILD ? */
 
/******************************************************************************/

#ifdef __cplusplus
   }
#endif /* __cplusplus */
#endif /* XMLSBUF__H */



