/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CVariant;
class CDynamicBuffer;

/**
 *  Variant binary serializer class
 *  Supports both a proprietary binary format and a raw binary format
 */
class GUCEF_CORE_PUBLIC_CPP CVariantBinarySerializer
{
    public:
    
    /**
     *  Serializes the variant using the binary format inherent to this serializer.
     *  This entails using a prefix to indicate the type of the variant, followed by the actual data.
     *  If you want to serialize without any prefix use the SerializeRaw member function.
     */
    static bool Serialize( const CVariant& var        , 
                           UInt32 currentTargetOffset , 
                           CDynamicBuffer& target     , 
                           UInt32& bytesWritten       );

    /**
     *  Serializes the variant using the binary format inherent to this serializer.
     *  This entails using a prefix to indicate the type of the variant, followed by the actual data.
     *  If you want to serialize without any prefix use the SerializeRaw member function.
     */
    static bool Serialize( const CVariant& var  , 
                           CIOAccess& access    , 
                           UInt32& bytesWritten );

    /**
     *  Serializes the variant using only the raw bytes of the value type contained within the variant
     *  If you want to serialize with meta-data use the Serialize member function instead.
     */
    static bool SerializeRaw( const CVariant& var        , 
                              UInt32 currentTargetOffset , 
                              CDynamicBuffer& target     , 
                              UInt32& bytesWritten       );

    /**
     *  Serializes the variant using only the raw bytes of the value type contained within the variant
     *  If you want to serialize with meta-dta use the Serialize member function instead.
     */
    static bool SerializeRaw( const CVariant& var  , 
                              CIOAccess& access    , 
                              UInt32& bytesWritten );

    /**
     *  Deserializes the variant from what is presumed to be the binary format inherent to this serializer.
     *  This entails having a prefix to indicate the type of the variant, followed by the actual data.
     *  If you want to deserialize without any such prefix use the DeserializeRaw member function.
     */
    static bool Deserialize( CVariant& var                , 
                             UInt32 currentSourceOffset   , 
                             const CDynamicBuffer& source , 
                             bool linkWherePossible       , 
                             UInt32& bytesRead            ); 

    /**
     *  Deserializes the variant using only the raw bytes of the value type specified
     *  If you want to deserialize with the variant type meta-dta having been persisted alongside the data use 
     *  the Deserialize member function instead.
     *  Note that the dataSizeIfDynamic parameter is only used for dynamic data types and should be set to the size of the data if known.
     *  For fixed size data types this parameter can be left at 0 since it will be automatically determined based on the type specified
     */
    static bool DeserializeRaw( CVariant& var                , 
                                UInt8 varType                , 
                                UInt32 currentSourceOffset   , 
                                const CDynamicBuffer& source , 
                                bool linkWherePossible       , 
                                UInt32& bytesRead            ,
                                UInt32 dataSizeIfDynamic = 0 ); 

    /**
     *  Deserializes the variant using only the raw bytes of the value type specified
     *  If you want to deserialize with the variant type meta-data having been persisted alongside the data use 
     *  the Deserialize member function instead.
     *  Note that the dataSizeIfDynamic parameter is only used for dynamic data types and should be set to the size of the data if known.
     *  For fixed size data types this parameter can be left at 0 since it will be automatically determined based on the type specified
     */
    static bool DeserializeRaw( CVariant& var                , 
                                UInt8 varType                , 
                                CIOAccess& source            ,  
                                UInt32& bytesRead            ,
                                UInt32 dataSizeIfDynamic = 0 );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */
