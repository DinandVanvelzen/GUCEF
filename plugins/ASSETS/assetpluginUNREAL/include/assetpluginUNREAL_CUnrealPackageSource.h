/*
 *  assetpluginUNREAL: Range access to the bytes of a package: a VFS file or a block of memory
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#include "assetpluginUNREAL_CUnrealArchiveReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALARCHIVEREADER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  Where the bytes of a package come from. Opening a range gives a reader with its own resource access and position,
 *  so several readers can work on one package at the same time. A VFS source opens the file in the VFS for every range;
 *  a memory source (used for the tests of damaged packages) must outlive the readers it hands out.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPackageSource
{
    public:

    CUnrealPackageSource( void );

    ~CUnrealPackageSource();

    /** Uses the file at the VFS path; false when it cannot be opened */
    bool OpenVfs( const CORE::CString& vfsPath );

    /** Uses the bytes [offset,offset+size) of the file at the VFS path as the package: a package that sits inside an archive of stored entries */
    bool OpenVfsRange( const CORE::CString& vfsPath, UInt64 offset, UInt64 size );

    /** Uses a copy of the given bytes */
    bool OpenMemory( const void* bytes, UInt32 size, const CORE::CString& name );

    bool IsOpen( void ) const;

    UInt64 GetSize( void ) const;

    /** The VFS path, or the name given to a memory source */
    const CORE::CString& GetName( void ) const;

    /** Starts a reader on the bytes [offset,offset+size) of the package; the window size is 0 for the default */
    bool OpenRange( UInt64 offset, UInt64 size, CUnrealArchiveReader& reader, UInt32 windowBytes = 0 ) const;

    private:

    CUnrealPackageSource( const CUnrealPackageSource& src );            /**< not implemented */
    CUnrealPackageSource& operator=( const CUnrealPackageSource& src ); /**< not implemented */

    private:

    CORE::CString m_name;
    bool m_isMemory;
    bool m_open;
    UInt64 m_size;
    UInt64 m_base;
    mutable CORE::CDynamicBuffer m_memory;
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

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H ? */
