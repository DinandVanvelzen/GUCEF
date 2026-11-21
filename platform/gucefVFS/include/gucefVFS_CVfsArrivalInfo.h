/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSARRIVALINFO_H
#define GUCEF_VFS_CVFSARRIVALINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#include "gucefCORE_EFileSystemAccessResult.h"
#define GUCEF_CORE_EFILESYSTEMACCESSRESULT_H
#endif /* GUCEF_CORE_EFILESYSTEMACCESSRESULT_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The data of the events of a remote archive (CRemoteArchiveEvents) that tell about a path: that content arrived, that
 *  a background job failed, that a revision moved. It is a copy: an event is mailed from the thread that raised it to
 *  the thread that handles it.
 *
 *  The path is relative to the archive when the archive raised the event and a VFS path (the mount path in front)
 *  when the VFS raised it.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsArrivalInfo : public CORE::CICloneable
{
    public:

    enum EKind
    {
        KIND_FILE = 0,                  /**< blocks of a file arrived (FileArrivedEvent) */
        KIND_LISTING,                   /**< the listing of a directory arrived (ListingArrivedEvent) */
        KIND_METADATA,                  /**< the metadata of a path arrived (MetadataArrivedEvent) */
        KIND_FAILURE,                   /**< a background job did not succeed (FetchFailedEvent) */
        KIND_CHANGE                     /**< the revision of a file moved (FileChangedEvent) */
    };

    EKind kind;
    CORE::CString path;                 /**< the file, or the directory for a listing */
    CORE::CString revision;             /**< the revision the event is about (the new one for a change) */
    CORE::CString oldRevision;          /**< for a change: the revision it was, when it is known */
    UInt64 firstByte;                   /**< for a file: where the bytes that arrived start */
    UInt64 byteCount;                   /**< for a file: how many bytes arrived */
    bool fileComplete;                  /**< for a file: every block of it is held now */
    CORE::TFileSystemAccessResult result;   /**< for a failure: why; a success otherwise */
    CORE::CString message;              /**< for a failure: in words */
    EVfsFetchJobKind jobKind;           /**< for a failure: the kind of the job that failed; for the others the kind of the job that brought it */

    CVfsArrivalInfo( void );

    CVfsArrivalInfo( EKind infoKind, const CORE::CString& infoPath );

    CVfsArrivalInfo( const CVfsArrivalInfo& src );

    virtual ~CVfsArrivalInfo() GUCEF_VIRTUAL_OVERRIDE;

    CVfsArrivalInfo& operator=( const CVfsArrivalInfo& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSARRIVALINFO_H ? */
