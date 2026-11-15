/*
 *  assetpluginFBX: an FBX file read through the VFS with ufbx
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#include "assetpluginFBX_CFbxDocument.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace FBX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFbxDocument::CFbxDocument( void )
    : m_scene( GUCEF_NULL )
    , m_path()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFbxDocument::~CFbxDocument()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CFbxDocument::Clear( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_scene )
    {
        ufbx_free_scene( m_scene );
        m_scene = GUCEF_NULL;
    }
    m_path.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CFbxDocument::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_scene;
}

/*-------------------------------------------------------------------------*/

const ufbx_scene*
CFbxDocument::GetScene( void ) const
{GUCEF_TRACE;

    return m_scene;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFbxDocument::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

bool
CFbxDocument::Load( const CORE::CString& vfsPath, CORE::CString& problem )
{GUCEF_TRACE;

    Clear();
    CORE::CDynamicBuffer file;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( file, vfsPath, "rb" ) )
    {
        problem = "the file " + vfsPath + " cannot be read";
        return false;
    }
    m_path = vfsPath;

    ufbx_load_opts options;
    memset( &options, 0, sizeof( options ) );
    options.filename.data = vfsPath.C_String();
    options.filename.length = vfsPath.ByteSize() > 0 ? vfsPath.ByteSize() - 1 : 0;
    options.generate_missing_normals = true;
    options.clean_skin_weights = true;
    options.ignore_missing_external_files = true;
    options.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_MODIFY_GEOMETRY;
    options.inherit_mode_handling = UFBX_INHERIT_MODE_HANDLING_COMPENSATE;
    options.pivot_handling = UFBX_PIVOT_HANDLING_ADJUST_TO_PIVOT;

    ufbx_error error;
    memset( &error, 0, sizeof( error ) );
    m_scene = ufbx_load_memory( file.GetConstBufferPtr(), file.GetDataSize(), &options, &error );
    if ( GUCEF_NULL == m_scene )
    {
        char text[ 512 ];
        ufbx_format_error( text, sizeof( text ), &error );
        problem = vfsPath + ": " + CORE::CString( text );
        m_path.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FBX */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
