/*
 *  assetpluginUNREAL: Little endian primitive reader over a range of a resource, with a read window and a sticky error state
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#include "assetpluginUNREAL_CUnrealGuid.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALGUID_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the primitives of the package format from a range [start,end) of a resource. The resource is read through a
 *  window (default 1 MiB) that is refilled on demand, so a package is never read as a whole. The first problem (a read
 *  past the end, a bad count, a bad string) puts the reader in a sticky error state: every later read returns zero and
 *  consumes nothing, and the offset and reason of the first problem are kept. There are no exceptions.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealArchiveReader
{
    public:

    CUnrealArchiveReader( void );

    ~CUnrealArchiveReader();

    /**
     *  Starts reading the range [start,end) of the access; the position is at start afterwards. The bias is added to every
     *  offset that is used on the access, for a package that is embedded in a larger file: the positions of the reader stay
     *  relative to the start of the package.
     */
    bool Open( const CORE::IOAccessPtr& access, UInt64 start, UInt64 end, UInt32 windowBytes = 1024 * 1024, UInt64 bias = 0 );

    /** Keeps the VFS resource alive for as long as the reader reads from its access */
    void SetKeepAlive( const VFS::TBasicVfsResourcePtr& resource );

    void Close( void );

    bool IsOpen( void ) const;

    UInt64 GetStart( void ) const;

    UInt64 GetEnd( void ) const;

    UInt64 GetPosition( void ) const;

    UInt64 GetRemaining( void ) const;

    /** Bytes taken from the resource so far (window refills and direct reads) */
    UInt64 GetBytesFetched( void ) const;

    bool Seek( UInt64 position );

    bool Skip( UInt64 bytes );

    /** Reads exactly size bytes; on a shortage nothing is consumed and the reader is in the error state */
    bool Read( void* dest, UInt64 size );

    UInt8 ReadUInt8( void );
    Int8 ReadInt8( void );
    UInt16 ReadUInt16( void );
    Int16 ReadInt16( void );
    UInt32 ReadUInt32( void );
    Int32 ReadInt32( void );
    UInt64 ReadUInt64( void );
    Int64 ReadInt64( void );
    /** Big endian forms, used by the header of compressed buffers */
    UInt32 ReadUInt32BE( void );
    UInt64 ReadUInt64BE( void );

    Float32 ReadFloat32( void );
    Float64 ReadFloat64( void );

    /** A boolean of 32 bits as the engine stores it in tables; a value other than 0 and 1 is an error */
    bool ReadBool32( void );

    /** A boolean of 8 bits; a value other than 0 and 1 is an error */
    bool ReadBool8( void );

    /** A length prefixed string, ANSI or UTF-16, converted to UTF-8 */
    bool ReadString( CORE::CString& text, UInt32 maxCodeUnits = 1024 * 1024 );

    bool ReadGuid( CUnrealGuid& guid );

    /** Whether count elements of at least minElementBytes each can still be read; sets the error state when not */
    bool CheckCount( UInt64 count, UInt64 minElementBytes, const char* what );

    /**
     *  Lowers the end of what can be read to newEnd (not below the position, not above the current end) and returns the
     *  previous end for RestoreEnd(). Used to keep a reader inside the declared size of a tag or an export.
     */
    UInt64 NarrowEnd( UInt64 newEnd );

    void RestoreEnd( UInt64 previousEnd );

    bool HasError( void ) const;

    const CORE::CString& GetError( void ) const;

    UInt64 GetErrorOffset( void ) const;

    /** Leaves the error state; for readers that recover at a declared boundary (the end of a property tag) */
    void ClearError( void );

    /** Puts the reader in the error state, the first reason is kept; returns false for convenience */
    bool SetError( const CORE::CString& reason );

    private:

    CUnrealArchiveReader( const CUnrealArchiveReader& src );            /**< not implemented */
    CUnrealArchiveReader& operator=( const CUnrealArchiveReader& src ); /**< not implemented */

    bool Fill( UInt64 position );

    bool ReadRaw( void* dest, UInt32 size );

    private:

    CORE::IOAccessPtr m_access;
    VFS::TBasicVfsResourcePtr m_keepAlive;
    UInt64 m_bias;
    UInt64 m_start;
    UInt64 m_end;
    UInt64 m_hardEnd;
    UInt64 m_position;
    GUCEF::vector< UInt8 > m_window;
    UInt64 m_windowStart;
    UInt32 m_windowLength;
    UInt64 m_bytesFetched;
    bool m_error;
    CORE::CString m_errorText;
    UInt64 m_errorOffset;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */
