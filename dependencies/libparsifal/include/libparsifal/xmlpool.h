/*===========================================================================
  xmlpool.h
    Based heavily on alloc-pool.c in gcc (some old code)
  TODO
    - String interning (wraps xmlpool object?)
    - Automatically free blocks when all items are unused (in XMLPool_Free)
      Add itemsAllocatedThisBlock tracking var to headers?
===========================================================================*/
#ifndef XMLPOOL__H
#define XMLPOOL__H

#include <stddef.h>
#include "pns.h"

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

typedef struct tagLPXMLPOOLLIST 
{
	struct tagLPXMLPOOLLIST *next;
} *LPXMLPOOLLIST;

typedef struct tagLPXMLPOOL 
{
	int itemSize;
	int itemsPerBlock; 
	int itemsAllocated;
	int itemsFree;
	int blocksAllocated; 
	int blockSize; 
	LPXMLPOOLLIST freeList;
	LPXMLPOOLLIST blockList;
} *LPXMLPOOL;

#ifdef __cplusplus
   extern "C" {
#endif

LPXMLPOOL XMLAPI XMLPool_Create(int itemSize, int itemsPerBlock);
void XMLAPI XMLPool_FreePool(LPXMLPOOL pool);
void XMLAPI *XMLPool_Alloc(LPXMLPOOL pool);
void XMLAPI XMLPool_Free(LPXMLPOOL pool, void *ptr);

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
#endif /* XMLPOOL__H */


