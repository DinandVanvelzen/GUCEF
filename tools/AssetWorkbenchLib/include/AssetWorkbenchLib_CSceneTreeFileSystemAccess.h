/*
 *  AssetWorkbenchLib: a scene node tree as a read only file system, for the content browser
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

#ifndef ASSETWORKBENCHLIB_CSCENETREEFILESYSTEMACCESS_H
#define ASSETWORKBENCHLIB_CSCENETREEFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The node tree of a scene as a file system that nothing can be written to, so that a GUI::CContentBrowserForm shows it
 *  as it shows folders: every scene node is a directory (the scene itself is the root "/"), and the objects attached to a node
 *  are files in that directory, named "<name>.<kind>" with the kind entity, light, camera or object. A node path is the names
 *  of the nodes from the root, "/Parent/Child".
 *
 *  The tree is a copy: SetTree() replaces it, and the form shows the new one after its Refresh().
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CSceneTreeFileSystemAccess : public CORE::IFileSystemAccess
{
    public:

    /** An object attached to a node */
    struct SObject
    {
        CORE::CString name;
        CORE::CString kind;         /**< entity, light, camera or object */
    };
    typedef GUCEF::vector< SObject > TObjectVector;

    /** A node of the tree; the parent comes before its children in the list */
    struct SNode
    {
        CORE::CString path;         /**< "/Parent/Child" */
        TObjectVector objects;
    };
    typedef GUCEF::vector< SNode > TNodeVector;

    explicit CSceneTreeFileSystemAccess( const CORE::CString& displayName = "Scene" );

    virtual ~CSceneTreeFileSystemAccess() GUCEF_VIRTUAL_OVERRIDE;

    /** Replaces the tree; the root is always there, as a node of the path "/" it can have objects too */
    void SetTree( const TNodeVector& nodes );

    const TNodeVector& GetNodes( void ) const;

    /** The path of a child: "/" and "A" give "/A", "/A" and "B" give "/A/B" */
    static CORE::CString JoinPath( const CORE::CString& parent, const CORE::CString& name );

    /** The path of the node a node or object path is in, "/" for the children of the root */
    static CORE::CString ParentOf( const CORE::CString& path );

    /** The kind in the name of an object file, empty for a name that has none of the kinds */
    static CORE::CString GetObjectKind( const CORE::CString& fileName );

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                                        const CORE::CString& destinationDir             ,
                                                        bool overwrite                                  ,
                                                        TFileSystemAccessResultVector* perFileResults   ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CSceneTreeFileSystemAccess( const CSceneTreeFileSystemAccess& src );
    CSceneTreeFileSystemAccess& operator=( const CSceneTreeFileSystemAccess& src );

    const SNode* FindNode( const CORE::CString& path ) const;
    static CORE::CString Conform( const CORE::CString& path );

    private:

    CORE::CString m_displayName;
    TNodeVector m_nodes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSCENETREEFILESYSTEMACCESS_H ? */
