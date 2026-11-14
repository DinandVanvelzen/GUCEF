/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef GUCEF_VFS_ASYNCVFSTASKDATA_H
#define GUCEF_VFS_ASYNCVFSTASKDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEEXPANSION_H
#include "CTCloneableExpansion.h"
#define GUCEF_CORE_CTCLONEABLEEXPANSION_H
#endif /* GUCEF_CORE_CTCLONEABLEEXPANSION_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#include "gucefVFS_CArchiveSettings.h"
#define GUCEF_VFS_CARCHIVESETTINGS_H
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

enum EAsyncVfsOperationType GUCEF_ENUM_BASE( int )
{
    ASYNCVFSOPERATIONTYPE_UNKNOWN           = 0,
    ASYNCVFSOPERATIONTYPE_MOUNTARCHIVE         ,
    ASYNCVFSOPERATIONTYPE_STOREDATAASFILE      ,
    ASYNCVFSOPERATIONTYPE_COPYFILE             ,
    ASYNCVFSOPERATIONTYPE_COPYFILECONTENT      ,
    ASYNCVFSOPERATIONTYPE_MOVEFILE             ,
    ASYNCVFSOPERATIONTYPE_ENCODEFILE           ,
    ASYNCVFSOPERATIONTYPE_DECODEFILE           ,
    ASYNCVFSOPERATIONTYPE_ENCODEDATAASFILE
};
typedef enum EAsyncVfsOperationType TAsyncVfsOperationType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CAsyncVfsTaskData : public CORE::CICloneable
{
    public:

    TAsyncVfsOperationType operationType;
    CORE::CString asyncRequestId;
    
    virtual void SetRequestorData( CORE::CICloneable* data );

    virtual CORE::CICloneable* GetRequestorData( void ) const;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CAsyncVfsTaskData& operator=( const CAsyncVfsTaskData& src );

    CAsyncVfsTaskData( void );
    CAsyncVfsTaskData( const CAsyncVfsTaskData& src );
    virtual ~CAsyncVfsTaskData();

    protected:
    CORE::CICloneable* m_requestorData;
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CAsyncVfsTaskResultData : public CAsyncVfsTaskData
{
    public:

    bool successState;
    UInt32 durationInMilliSecs;

    virtual void SetTaskData( CAsyncVfsTaskData* data );

    virtual CAsyncVfsTaskData* GetTaskData( void ) const;

    virtual void SetRequestorData( CORE::CICloneable* data ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* GetRequestorData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CAsyncVfsTaskResultData( void );
    CAsyncVfsTaskResultData( const CAsyncVfsTaskResultData& src );
    virtual ~CAsyncVfsTaskResultData();

    protected:
    CAsyncVfsTaskData* m_taskData;
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CStoreAsFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString filepath;
    CORE::CDynamicBuffer data;
    CORE::UInt64 offset;
    bool overwrite;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CStoreAsFileTaskData( void );
    CStoreAsFileTaskData( const CStoreAsFileTaskData& src );
    virtual ~CStoreAsFileTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CMountArchiveTaskData : public CAsyncVfsTaskData
{
    public:
    CArchiveSettings settings;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CMountArchiveTaskData( void );
    CMountArchiveTaskData( const CMountArchiveTaskData& src );
    virtual ~CMountArchiveTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CEncodeFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString originalFilepath;
    CORE::CString encodedFilepath;
    bool overwrite;
    CString codecFamily;
    CORE::CString encodeCodec;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CEncodeFileTaskData( void );
    CEncodeFileTaskData( const CEncodeFileTaskData& src );
    virtual ~CEncodeFileTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CEncodeBufferAsFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CDynamicBuffer data;
    CORE::UInt64 bufferOffset;
    CORE::CString encodedFilepath;
    bool overwrite;
    CString codecFamily;
    CORE::CString encodeCodec;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CEncodeBufferAsFileTaskData( void );
    CEncodeBufferAsFileTaskData( const CEncodeBufferAsFileTaskData& src );
    virtual ~CEncodeBufferAsFileTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CDecodeFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString originalFilepath;
    CORE::CString decodedFilepath;
    bool overwrite;
    CString codecFamily;
    CORE::CString decodeCodec;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CDecodeFileTaskData( void );
    CDecodeFileTaskData( const CDecodeFileTaskData& src );
    virtual ~CDecodeFileTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CCopyFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString originalFilepath;
    CORE::CString copyFilepath;
    bool overwrite;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CCopyFileTaskData( void );
    CCopyFileTaskData( const CCopyFileTaskData& src );
    virtual ~CCopyFileTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CCopyFileContentTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString originalFilepath;
    CORE::CString copyFilepath;
    bool overwrite;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CCopyFileContentTaskData( void );
    CCopyFileContentTaskData( const CCopyFileContentTaskData& src );
    virtual ~CCopyFileContentTaskData();
};

/*-------------------------------------------------------------------------*/

class GUCEF_VFS_PUBLIC_CPP CMoveFileTaskData : public CAsyncVfsTaskData
{
    public:

    CORE::CString originalFilepath;
    CORE::CString newFilepath;
    bool overwrite;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    CMoveFileTaskData( void );
    CMoveFileTaskData( const CMoveFileTaskData& src );
    virtual ~CMoveFileTaskData();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_VFS_ASYNCVFSTASKDATA_H ? */
