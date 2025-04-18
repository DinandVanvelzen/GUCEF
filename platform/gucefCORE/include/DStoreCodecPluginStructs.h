/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*
 *      Header for internal use.      
 */

#ifndef GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H
#define GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"         /* often used gucefCORE macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace CORE {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnTreeBegin ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnTreeEnd ) ( void* privdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeBegin ) ( void* privdata, const char* nodename, int nodeType ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeEnd ) ( void* privdata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeAtt ) ( void* privdata, const char* nodename, const TVariantData* attname, const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeValue ) ( void* privdata, const char* nodename, const TVariantData* attvalue ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeChildrenBegin ) ( void* privdata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnNodeChildrenEnd ) ( void* privdata, const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;
typedef void ( GUCEF_PLUGIN_CALLSPEC_PREFIX *TDSTOREPLUGFPTR_OnError ) ( void* privdata, Int32 errorcode, const char* description ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Structure that hold all the function pointers to the event handlers
 *      for read events. Should be called by plugins as needed.
 */
struct SReadHandlers
{
    TDSTOREPLUGFPTR_OnTreeBegin         OnTreeBegin;
    TDSTOREPLUGFPTR_OnTreeEnd           OnTreeEnd;
    TDSTOREPLUGFPTR_OnNodeBegin         OnNodeBegin;
    TDSTOREPLUGFPTR_OnNodeEnd           OnNodeEnd;
    TDSTOREPLUGFPTR_OnNodeAtt           OnNodeAtt;
    TDSTOREPLUGFPTR_OnNodeValue         OnNodeValue;
    TDSTOREPLUGFPTR_OnNodeChildrenBegin OnNodeChildrenBegin;
    TDSTOREPLUGFPTR_OnNodeChildrenEnd   OnNodeChildrenEnd;
    TDSTOREPLUGFPTR_OnError             OnError;                             
};
typedef struct SReadHandlers TReadHandlers;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace CORE */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_DSTORECODECPLUGINSTRUCTS_H ? */
