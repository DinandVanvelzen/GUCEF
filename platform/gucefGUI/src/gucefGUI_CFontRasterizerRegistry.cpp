/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefGUI_CFontRasterizerRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CFontRasterizerRegistry::CFontRasterizerRegistry( void )
    : m_rasterizers()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFontRasterizerRegistry::~CFontRasterizerRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CFontRasterizerRegistry::Register( CFontRasterizer* rasterizer )
{GUCEF_TRACE;

    if ( GUCEF_NULL == rasterizer )
        return false;

    CString name = rasterizer->GetName();
    MT::CScopeMutex lock( m_lock );
    if ( m_rasterizers.find( name ) != m_rasterizers.end() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FontRasterizerRegistry: A font rasterizer named \"" << name << "\" is already registered" );
        return false;
    }
    m_rasterizers[ name ] = rasterizer;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "FontRasterizerRegistry: Registered font rasterizer \"" << name << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFontRasterizerRegistry::Unregister( const CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return 0 < m_rasterizers.erase( name );
}

/*-------------------------------------------------------------------------*/

CFontRasterizer*
CFontRasterizerRegistry::Get( const CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TRasterizerMap::const_iterator i = m_rasterizers.find( name );
    return i == m_rasterizers.end() ? GUCEF_NULL : (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CFontRasterizerRegistry::HasRasterizers( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_rasterizers.empty();
}

/*-------------------------------------------------------------------------*/

CFontRasterizerRegistry::TStringSet
CFontRasterizerRegistry::GetNames( void ) const
{GUCEF_TRACE;

    TStringSet names;
    MT::CScopeMutex lock( m_lock );
    TRasterizerMap::const_iterator i = m_rasterizers.begin();
    while ( i != m_rasterizers.end() )
    {
        names.insert( (*i).first );
        ++i;
    }
    return names;
}

/*-------------------------------------------------------------------------*/

TBitmapFontPtr
CFontRasterizerRegistry::RasterizeFont( const CORE::CDynamicBuffer& fontData ,
                                        UInt32 pixelSize                     ,
                                        const TCodePointRanges& ranges       ) const
{GUCEF_TRACE;

    // the lock is held while rasterizing: a plugin cannot unregister and unload its rasterizer meanwhile
    MT::CScopeMutex lock( m_lock );
    TRasterizerMap::const_iterator i = m_rasterizers.begin();
    while ( i != m_rasterizers.end() )
    {
        CFontRasterizer* rasterizer = (*i).second;
        if ( rasterizer->CanRasterize( fontData ) )
        {
            TBitmapFontPtr font = rasterizer->Rasterize( fontData, pixelSize, ranges );
            if ( !font.IsNULL() )
                return font;
        }
        ++i;
    }
    return TBitmapFontPtr();
}

/*-------------------------------------------------------------------------*/

TBitmapFontPtr
CFontRasterizerRegistry::RasterizeFont( const CString& vfsPath         ,
                                        UInt32 pixelSize               ,
                                        const TCodePointRanges& ranges ) const
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FontRasterizerRegistry: Cannot open font " << vfsPath );
        return TBitmapFontPtr();
    }

    CORE::CDynamicBuffer fontData;
    CORE::CIOAccess& access = *file->GetAccess();
    UInt64 size = access.GetSize();
    if ( 0 == size || (UInt32) size != access.Read( fontData, 1, (UInt32) size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FontRasterizerRegistry: Cannot read font " << vfsPath );
        return TBitmapFontPtr();
    }

    TBitmapFontPtr font = RasterizeFont( fontData, pixelSize, ranges );
    if ( font.IsNULL() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "FontRasterizerRegistry: No loaded font rasterizer reads " << vfsPath );
    return font;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
