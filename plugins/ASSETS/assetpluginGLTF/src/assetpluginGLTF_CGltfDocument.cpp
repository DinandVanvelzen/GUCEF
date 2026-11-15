/*
 *  assetpluginGLTF: a glTF or GLB file read through the VFS with cgltf
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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "assetpluginGLTF_CGltfDocument.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** cgltf asks for the files it names (the buffers): they come from the VFS */
cgltf_result
ReadFromVfs( const cgltf_memory_options* memory, const cgltf_file_options* /* file */, const char* path, cgltf_size* size, void** data )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( bytes, path, "rb" ) )
        return cgltf_result_file_not_found;

    const UInt32 byteSize = bytes.GetDataSize();
    void* copy = GUCEF_NULL != memory->alloc_func ? memory->alloc_func( memory->user_data, byteSize ) : malloc( byteSize > 0 ? byteSize : 1 );
    if ( GUCEF_NULL == copy )
        return cgltf_result_out_of_memory;
    if ( byteSize > 0 )
        memcpy( copy, bytes.GetConstBufferPtr(), byteSize );
    *size = byteSize;
    *data = copy;
    return cgltf_result_success;
}

void
ReleaseFromVfs( const cgltf_memory_options* memory, const cgltf_file_options* /* file */, void* data )
{GUCEF_TRACE;

    if ( GUCEF_NULL != memory->free_func )
        memory->free_func( memory->user_data, data );
    else
        free( data );
}

void
SetFileOptions( cgltf_options& options )
{
    memset( &options, 0, sizeof( options ) );
    options.file.read = &ReadFromVfs;
    options.file.release = &ReleaseFromVfs;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGltfDocument::CGltfDocument( void )
    : m_file()
    , m_data( GUCEF_NULL )
    , m_path()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGltfDocument::~CGltfDocument()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CGltfDocument::Clear( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_data )
    {
        cgltf_free( m_data );
        m_data = GUCEF_NULL;
    }
    m_file.Clear();
    m_path.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CGltfDocument::IsLoaded( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_data;
}

/*-------------------------------------------------------------------------*/

const cgltf_data*
CGltfDocument::GetData( void ) const
{GUCEF_TRACE;

    return m_data;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGltfDocument::GetPath( void ) const
{GUCEF_TRACE;

    return m_path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfDocument::GetFolder( void ) const
{GUCEF_TRACE;

    return ASSETCONVERTER::CVfsPaths::FolderOf( m_path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfDocument::GetJson( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    if ( GUCEF_NULL != m_data && GUCEF_NULL != m_data->json )
        text.Set( m_data->json, (UInt32) m_data->json_size );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfDocument::GetResultText( cgltf_result result )
{GUCEF_TRACE;

    switch ( result )
    {
        case cgltf_result_success : return "success";
        case cgltf_result_data_too_short : return "the data is too short";
        case cgltf_result_unknown_format : return "the format is not glTF or GLB";
        case cgltf_result_invalid_json : return "the JSON is not valid";
        case cgltf_result_invalid_gltf : return "the glTF is not valid";
        case cgltf_result_invalid_options : return "invalid options";
        case cgltf_result_file_not_found : return "a file is not found";
        case cgltf_result_io_error : return "a file cannot be read";
        case cgltf_result_out_of_memory : return "out of memory";
        case cgltf_result_legacy_gltf : return "the file is glTF 1.0, which is not supported";
        default : return "an unknown error";
    }
}

/*-------------------------------------------------------------------------*/

bool
CGltfDocument::Load( const CORE::CString& vfsPath, CORE::CString& problem )
{GUCEF_TRACE;

    Clear();
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( m_file, vfsPath, "rb" ) )
    {
        problem = "the file " + vfsPath + " cannot be read";
        return false;
    }
    m_path = vfsPath;

    cgltf_options options;
    SetFileOptions( options );
    const cgltf_result result = cgltf_parse( &options, m_file.GetConstBufferPtr(), m_file.GetDataSize(), &m_data );
    if ( cgltf_result_success != result )
    {
        problem = vfsPath + ": " + GetResultText( result );
        m_data = GUCEF_NULL;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGltfDocument::LoadBuffers( CORE::CString& problem )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return false;

    cgltf_options options;
    SetFileOptions( options );
    const cgltf_result result = cgltf_load_buffers( &options, m_data, m_path.C_String() );
    if ( cgltf_result_success != result )
    {
        problem = m_path + ": the buffers cannot be loaded: " + GetResultText( result );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGltfDocument::Validate( CORE::CString& problem )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_data )
        return false;

    const cgltf_result result = cgltf_validate( m_data );
    if ( cgltf_result_success != result )
    {
        problem = m_path + ": validation failed: " + GetResultText( result );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
