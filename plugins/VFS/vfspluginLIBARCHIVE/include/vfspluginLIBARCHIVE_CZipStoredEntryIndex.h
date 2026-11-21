/*
 *  vfspluginLIBARCHIVE: Generic GUCEF VFS plugin for archives handled by libarchive
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

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_CZIPSTOREDENTRYINDEX_H
#define GUCEF_VFSPLUGIN_LIBARCHIVE_CZIPSTOREDENTRYINDEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#include "vfspluginLIBARCHIVE_macros.h"
#define GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H
#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace LIBARCHIVE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where the bytes of the stored (not compressed, not encrypted) entries of a zip archive are in the archive file.
 *  libarchive reads the data of an entry as a stream, so reading from the middle of a large stored entry would read
 *  everything in front of it. With the offsets of this index a stored entry is read with direct seeks instead.
 *  The directory is read from the end of the file, ordinary and 64 bit (zip64) directories are understood.
 *  The index only names entries that are safe to read directly: any doubt leaves the entry out, and libarchive then reads it.
 */
class CZipStoredEntryIndex
{
    public:

    struct SStoredEntry
    {
        UInt64 dataOffset;      /**< offset in the archive file of the first data byte */
        UInt64 size;            /**< size of the data in bytes */
    };
    typedef GUCEF::map< CORE::CString, SStoredEntry > TEntryMap;

    /**
     *  Reads the directory of the zip archive at the given path
     *  @param entries receives the stored entries by the name in the directory, not normalized
     *  @return false if the file is not a zip archive or its directory cannot be read, entries is empty then
     */
    static bool Load( const CORE::CString& archivePath ,
                      TEntryMap& entries               );

    private:

    CZipStoredEntryIndex( void );   /**< only static members */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace LIBARCHIVE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_LIBARCHIVE_CZIPSTOREDENTRYINDEX_H ? */
