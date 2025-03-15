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

#ifndef GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H
#define GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_C_VARIANTMAP_H
#include "gucefCORE_c_variantmap.h"        
#define GUCEF_CORE_C_VARIANTMAP_H
#endif /* GUCEF_CORE_C_VARIANTMAP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Meta-data structure referencing the information that allows constructing a 
 *  data driven data storage codec by combining the base codec type name with resources
 *  and a data driven codec type name under which this logical codec will be registered.
 * 
 *  Some examples of data driven data storage codecs are:
 *      - protobuf schema driven data storage codecs for specific data types
 *      - kaitai schema driven data storage codecs for specific data types
 */
struct SDataDrivenDStoreCodecMeta
{
    const char* base_codec_type_name;
    const char* data_driven_codec_typename;
    TVariantMapApi resources;
    TVariantMapApi params;
    UInt8 is_shareable;
};

typedef struct SDataDrivenDStoreCodecMeta TDataDrivenDStoreCodecMeta;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_DATADRIVENDSTORECODECMETA_H ? */
