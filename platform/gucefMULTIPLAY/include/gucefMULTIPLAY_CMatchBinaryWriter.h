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

#ifndef GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H
#define GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H

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
 *  Appends values to a buffer in one fixed layout: whole numbers big endian, text and byte strings behind a
 *  32 bit length. Used for the bytes that are signed and for the container, where every machine must produce
 *  the same bytes for the same values.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchBinaryWriter
{
    public:

    explicit CMatchBinaryWriter( CORE::CDynamicBuffer& target );

    void WriteUInt8( UInt8 value );
    void WriteUInt16( UInt16 value );
    void WriteUInt32( UInt32 value );
    void WriteUInt64( UInt64 value );
    void WriteInt64( Int64 value );
    void WriteBool( bool value );

    /** The bytes as they are, without a length */
    void WriteRaw( const void* data, UInt32 size );

    /** A 32 bit length and the bytes */
    void WriteBytes( const void* data, UInt32 size );

    /** A 32 bit length and the UTF-8 bytes of the text */
    void WriteString( const CORE::CString& text );

    private:

    CMatchBinaryWriter( const CMatchBinaryWriter& src );
    CMatchBinaryWriter& operator=( const CMatchBinaryWriter& src );

    private:

    CORE::CDynamicBuffer& m_target;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H ? */
