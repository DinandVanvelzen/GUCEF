/*
 *  guceGUI: GUCE module providing GUI functionality
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "dvcppstringutils.h"
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefGUI_CFormEx.h"

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

CFormEx::CFormEx( void )
    : CForm()                 ,
      m_isReloadable( false ) ,
      m_layoutFile()          ,
      m_useVfsDirect( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CFormEx::CFormEx( CIGUIContext* context )
    : CForm( context )        ,
      m_isReloadable( false ) ,
      m_layoutFile()          ,
      m_useVfsDirect( false )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/
    
CFormEx::~CFormEx()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CFormEx::LoadLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    m_isReloadable = false;
    m_layoutFile.Clear();
    return CForm::LoadLayout( layoutStorage );
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::SaveLayout( CORE::CIOAccess& layoutStorage )
{GUCEF_TRACE;

    return CForm::SaveLayout( layoutStorage );
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::LoadLayout( const CString& filename )
{GUCEF_TRACE;

    m_isReloadable = true;
    m_layoutFile = filename;
    m_useVfsDirect = false;
    return CForm::LoadLayout( filename );
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::SaveLayout( const CString& filename )
{GUCEF_TRACE;

    return CForm::SaveLayout( filename );
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::LoadLayoutUsingVfs( const CString& filename )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( filename );
    if ( !file.IsNULL() )
    {
        // the name of the file picks the codec
        if ( CForm::LoadLayout( *file->GetAccess(), CORE::ExtractFileExtention( filename ).Lowercase() ) )
        {
            m_layoutFile = filename;
            m_isReloadable = true;
            m_useVfsDirect = true;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::SaveLayoutUsingVfs( const CString& filename )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr file = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( filename, "wb", true );
    if ( !file.IsNULL() )
    {
        return CForm::SaveLayout( *file->GetAccess(), CORE::ExtractFileExtention( filename ).Lowercase() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::Reload( void )
{GUCEF_TRACE;
    
    if ( m_isReloadable )
    {
        if ( m_useVfsDirect )
        {
            return LoadLayoutUsingVfs( m_layoutFile );
        }
        else
        {
            return LoadLayout( m_layoutFile );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
bool
CFormEx::IsReloadable( void ) const
{GUCEF_TRACE;

    return m_isReloadable;
}

/*-------------------------------------------------------------------------*/

const CString&
CFormEx::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::GUI::CFormEx";
    return typeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
