/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHBINARYREADER_H
#define GUCEF_MULTIPLAY_CMATCHBINARYREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads what CMatchBinaryWriter wrote from bytes that are not trusted. Every read checks what is left first,
 *  a read past the end or a length above the limit given by the caller fails the reader for good: all further
 *  reads give zero values and IsOk() stays false. Nothing is allocated before the length is checked.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchBinaryReader
{
    public:

    CMatchBinaryReader( const void* data, UInt32 size );

    UInt8 ReadUInt8( void );
    UInt16 ReadUInt16( void );
    UInt32 ReadUInt32( void );
    UInt64 ReadUInt64( void );
    Int64 ReadInt64( void );

    /** Only 0 and 1 are accepted */
    bool ReadBool( void );

    /** Exactly size bytes into the destination */
    bool ReadRaw( void* destination, UInt32 size );

    /** Skips size bytes and returns where they start, GUCEF_NULL when fewer are left */
    const UInt8* ReadRawInPlace( UInt32 size );

    /** A length prefixed text of at most maxLength bytes */
    bool ReadString( CORE::CString& text, UInt32 maxLength );

    /** A length prefixed byte string of at most maxLength bytes, returned in place */
    const UInt8* ReadBytesInPlace( UInt32& size, UInt32 maxLength );

    bool IsOk( void ) const;

    UInt32 GetRemaining( void ) const;

    UInt32 GetPosition( void ) const;

    /** Whether all of the bytes were read and nothing went wrong */
    bool IsAtEndAndOk( void ) const;

    private:

    const UInt8* Take( UInt32 size );

    private:

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_position;
    bool m_ok;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHBINARYREADER_H ? */
