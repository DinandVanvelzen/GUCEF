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

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEENTRYIOACCESS_H
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEENTRYIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H
#include "vfspluginUNITYPACKAGE_CUnityPackageReader.h"
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H
#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEREADER_H ? */

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
 *  Read only access to the data of a single entry of a Unity package, streamed from the package.
 *  Seeking forward skips data, seeking backward restarts from the beginning of the package,
 *  which is why the archive only uses this for entries too large to be loaded into memory.
 */
class CUnityPackageEntryIOAccess : public CORE::CIOAccess
{
    public:

    /**
     *  Positions a new reader of the package at the entry with the given index. Check IsValid() afterwards
     *  @param packagePath   path of the package file, used when there is no source access
     *  @param sourceAccess  optional access to the package data, as the package is not a file on disk
     */
    CUnityPackageEntryIOAccess( const CORE::CString& packagePath       ,
                                const CORE::IOAccessPtr& sourceAccess  ,
                                UInt32 entryIndex                      ,
                                UInt64 entrySize                       );

    virtual ~CUnityPackageEntryIOAccess() GUCEF_VIRTUAL_OVERRIDE;

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE;
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CUnityPackageEntryIOAccess( const CUnityPackageEntryIOAccess& src );            /**< not supported */
    CUnityPackageEntryIOAccess& operator=( const CUnityPackageEntryIOAccess& src ); /**< not supported */

    bool PositionAtEntry( void );
    bool SkipForward( UInt64 bytes );

    CORE::CString m_packagePath;
    CORE::IOAccessPtr m_sourceAccess;
    UInt32 m_entryIndex;
    UInt64 m_size;
    UInt64 m_pos;
    CUnityPackageReader m_reader;
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

#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_CUNITYPACKAGEENTRYIOACCESS_H ? */
