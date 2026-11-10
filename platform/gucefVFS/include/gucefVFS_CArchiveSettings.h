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

#ifndef GUCEF_VFS_CARCHIVESETTINGS_H
#define GUCEF_VFS_CARCHIVESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <set>

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"          /* abstract base class interface for configurable objects */
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"          
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

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

class GUCEF_VFS_PUBLIC_CPP CArchiveSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ArchiveSpecificSettingsPrefix;

    CArchiveSettings( void );
    
    CArchiveSettings( const CArchiveSettings& src );
    
    virtual ~CArchiveSettings();
    
    CArchiveSettings& operator=( const CArchiveSettings& src );
    
    /**
     *  Attempts to save data to the given node
     *
     *  @param cfg relevant data node in the tree
     *  @return whether storing the data was successfull.
     */ 
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const;
                                
    /**
     *  Attempts to load data from the given node
     *
     *  @param cfg relevant data node in the tree
     *  @return whether locating relevant information was successfull.
     */                                    
    virtual bool LoadConfig( const CORE::CDataNode& cfg );

    virtual const CORE::CString& GetClassTypeName( void ) const;

    void SetArchiveName( const CORE::CString& name );
    
    const CORE::CString& GetArchiveName( void ) const;

    void SetArchiveType( const CORE::CString& type );
    
    const CORE::CString& GetArchiveType( void ) const;

    void SetArchivePath( const CORE::CString& path );
    
    const CORE::CString& GetArchivePath( void ) const;

    void SetActualArchivePath( const CORE::CString& path );
    
    const CORE::CString& GetActualArchivePath( void ) const;

    void SetMountPath( const CORE::CString& path );
    
    const CORE::CString& GetMountPath( void ) const;

    void SetAutoMountSubArchives( bool autoMount );
    
    bool GetAutoMountSubArchives( void ) const;

    void SetAutoMountSubArchivesIsRecursive( bool recursive );
    
    bool GetAutoMountSubArchivesIsRecursive( void ) const;

    void SetWriteableRequested( bool writeable );
    
    bool GetWriteableRequested( void ) const;

    void SetReadableRequested( bool readable );
    
    bool GetReadableRequested( void ) const;

    void SetDirectoryWatchingAbilityRequested( bool canWatch );
    
    bool GetDirectoryWatchingAbilityRequested( void ) const;

    /**
     *  The archive that mounts this one as one of its sub archives (auto mounting), empty for none. Unmounting that archive
     *  unmounts this one too. Set at run time only, it is not part of the configuration
     */
    void SetParentArchiveName( const CORE::CString& parentArchiveName );

    const CORE::CString& GetParentArchiveName( void ) const;
    
    const CORE::CValueList& GetArchiveSpecificSettings( void ) const;

    private:
    
    CORE::CString m_archiveName;
    CORE::CString m_archivePath;
    CORE::CString m_actualArchivePath;
    CORE::CString m_archiveType;
    CORE::CString m_mountPath;
    bool m_autoMountSubArchives;
    bool m_autoMountSubArchivesIsRecursive;
    bool m_writeableRequested;
    bool m_readableRequested;
    bool m_directoryWatchingAbilityRequested;
    CORE::CString m_parentArchiveName;
    CORE::CValueList m_archiveSpecificSettings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_VFS_CARCHIVESETTINGS_H ? */
