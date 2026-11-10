/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H
#define GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

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

/** The operations the default async variants of IFileSystemAccess run on a pool thread */
enum EFileSystemAccessOperation
{
    FILESYSTEMACCESSOPERATION_UNKNOWN = 0,
    FILESYSTEMACCESSOPERATION_CREATE_DIRECTORY,
    FILESYSTEMACCESSOPERATION_DELETE_DIRECTORY,
    FILESYSTEMACCESSOPERATION_MOVE_DIRECTORY,
    FILESYSTEMACCESSOPERATION_MOVE_FILE,
    FILESYSTEMACCESSOPERATION_MOVE_FILES,
    FILESYSTEMACCESSOPERATION_RENAME_FILE,
    FILESYSTEMACCESSOPERATION_DELETE_FILE,
    FILESYSTEMACCESSOPERATION_GET_METADATA,
    FILESYSTEMACCESSOPERATION_GET_ENTRY_TYPE,
    FILESYSTEMACCESSOPERATION_GET_ITERATOR
};

/*-------------------------------------------------------------------------*/

/** What an async operation of IFileSystemAccess asks a pool thread to do, and what came of it */
class GUCEF_HIDDEN CFileSystemAccessTaskData : public CICloneable
{
    public:

    TFileSystemAccessPtr access;                                        /**< the provider, kept alive by this until the task ended */
    EFileSystemAccessOperation operation;
    CString path;                                                       /**< the path, the old path of a move, the destination of MoveFiles */
    CString secondPath;                                                 /**< the new path of a move, the new name of a rename */
    bool flag;                                                          /**< createParents, deleteContents or overwrite */
    CString::StringVector paths;                                        /**< the files of MoveFiles */

    TFileSystemAccessResult result;
    IFileSystemAccess::TFileSystemAccessResultVector perFileResults;
    CResourceMetaData metaData;
    TFileSystemEntryType entryType;
    TFileSystemIteratorPtr iterator;

    CFileSystemAccessTaskData( void );

    CFileSystemAccessTaskData( const CFileSystemAccessTaskData& src );

    virtual ~CFileSystemAccessTaskData();

    CFileSystemAccessTaskData& operator=( const CFileSystemAccessTaskData& src );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Type check of future data without RTTI, see CICloneable::CloneableAs() */
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The task that runs a synchronous operation of IFileSystemAccess for its default async variants */
class GUCEF_HIDDEN CAsyncFileSystemAccessOperation : public CTaskConsumer
{
    public:

    static const CString TaskType;

    CAsyncFileSystemAccessOperation( void );

    virtual ~CAsyncFileSystemAccessOperation() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    /** The task data stays: it has the result for IFileSystemAccess::TryGetAsync...() */
    virtual void OnTaskEnded( CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAsyncFileSystemAccessOperation( const CAsyncFileSystemAccessOperation& src );
    CAsyncFileSystemAccessOperation& operator=( const CAsyncFileSystemAccessOperation& src );
};

/*-------------------------------------------------------------------------*/

typedef CTFactory< CTaskConsumer, CAsyncFileSystemAccessOperation, MT::CMutex > TAsyncFileSystemAccessOperationTaskFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CASYNCFILESYSTEMACCESSOPERATION_H ? */
