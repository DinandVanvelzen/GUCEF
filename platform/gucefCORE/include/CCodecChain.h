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

#ifndef GUCEF_CORE_CCODECCHAIN_H
#define GUCEF_CORE_CCODECCHAIN_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"        /* dynamic buffer implementation */
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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

class CICodec;
class CStringList;

/*-------------------------------------------------------------------------*/

/**
 *      Utility class for sequential encoding/decoding steps
 *      Allows you to create a chain of codecs that will operate
 *      on the given data in order.
 *
class GUCEF_CORE_PUBLIC_CPP CCodecChain
{
        public:

        CCodecChain( void );

        CCodecChain( const CCodecChain& src );

        ~CCodecChain();

        CCodecChain& operator=( const CCodecChain& src );

        bool AddCodec( const CICodec& codec   ,
                       const UInt32 codecSlot );

        void AppendCodec( const CICodec& codec );

        void RemoveCodec( const CString& typeName );

        bool RemoveCodec( const UInt32 codecSlot );

        bool HasCodec( const CString& typeName ) const;

        void GetList( CStringList& list ) const;

        void SetCodecOrder( const CStringList& codecOrder );

        bool Encode( const void* sourceData         ,
                     const UInt32 sourceBuffersSize ,
                     CIOAccess& dest                );

        bool Decode( const void* sourceData         ,
                     const UInt32 sourceBuffersSize ,
                     CIOAccess& dest                );

        void Clear( void );

        private:

        bool EncodeBuffers( const TDynamicBufferList& src   ,
                            const UInt32 sourceBuffersUsed  ,
                            TDynamicBufferList& dest        ,
                            UInt32& destBuffsUsed           ,
                            TDynamicBufferList& swapBuffers ,
                            CICodec* codec                  ) const;

        bool DecodeBuffers( const TDynamicBufferList& src   ,
                            const UInt32 sourceBuffersUsed  ,
                            TDynamicBufferList& dest        ,
                            UInt32& destBuffsUsed           ,
                            TDynamicBufferList& swapBuffers ,
                            CICodec* codec                  ) const;

        private:
        typedef GUCEF::vector< CICodec* > TCodecList;

        TDynamicBufferList m_bufferA;
        TDynamicBufferList m_bufferB;
        TDynamicBufferList m_bufferC;
        TCodecList m_codecList;
};   */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCODECCHAIN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 20-07-2005 :
        - Added this class

-----------------------------------------------------------------------------*/
