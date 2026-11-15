/*
 *  vfspluginUNITYPACKAGE: GUCEF VFS plugin presenting a Unity package as the project tree it contains
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

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H
#include "vfspluginUNITYPACKAGE_macros.h"
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_MACROS_H ? */

// Forward declarations to avoid a header file dependency on libarchive
struct archive;
struct archive_entry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace UNITYPACKAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A forward only reader over the entries of the gzip compressed tar that makes up a Unity package.
 *  The package is read either from a file on disk or from a IO access object, for example the entry of
 *  another archive. In the latter case the reader works on its own clone of the given access object so
 *  that any number of readers can be used at the same time, also from different threads.
 */
class CUnityPackageReader
{
    public:

    CUnityPackageReader( void );

    ~CUnityPackageReader();

    /**
     *  Opens the package for reading from the start.
     *  If a source access object is given it is used as the data source and the path is not used
     *  @param path          path of a file on the file system
     *  @param sourceAccess  optional access to the package data, cloned by this call
     */
    bool Open( const CORE::CString& path                   ,
               const CORE::IOAccessPtr& sourceAccess       );

    void Close( void );

    bool IsOpen( void ) const;

    /**
     *  Advances to the next entry
     *  @return true if there is a next entry, false at the end of the package or on error, see HasFailed()
     */
    bool NextEntry( struct archive_entry** entry );

    /**
     *  Advances to the entry at the given zero based position in the package, skipping the entries before it
     *  Only valid directly after opening the package
     */
    bool SeekToEntry( UInt32 entryIndex, struct archive_entry** entry );

    /** Number of entries that have been advanced to so far */
    UInt32 GetEntryCount( void ) const;

    /** Skips the remainder of the data of the current entry */
    void SkipEntryData( void );

    /**
     *  Reads data of the current entry
     *  @return the number of bytes read, 0 at the end of the entry or on error
     */
    UInt64 ReadEntryData( void* dest, UInt64 bytes );

    /**
     *  Opens the package, advances to the given entry and reads up to the given number of bytes of its data
     *  into the buffer, which is resized to what could be read. Returns whether the entire size was read.
     */
    bool ReadWholeEntry( const CORE::CString& path             ,
                         const CORE::IOAccessPtr& sourceAccess ,
                         UInt32 entryIndex                     ,
                         UInt32 entrySize                      ,
                         CORE::CDynamicBuffer& buffer          );

    bool HasFailed( void ) const;

    CORE::CString GetErrorString( void ) const;

    /** Called by the libarchive callbacks */
    UInt32 ReadSource( const void** data );

    private:

    CUnityPackageReader( const CUnityPackageReader& src );            /**< not supported */
    CUnityPackageReader& operator=( const CUnityPackageReader& src ); /**< not supported */

    struct archive* m_archive;
    CORE::IOAccessPtr m_access;
    CORE::CDynamicBuffer m_sourceBuffer;
    UInt32 m_entryCount;
    bool m_failed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITYPACKAGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H ? */
