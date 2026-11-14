/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_MACROS_H
#include "guidriverCEGUI_macros.h" 
#define GUCEF_GUIDRIVERCEGUI_MACROS_H
#endif /* GUCEF_GUIDRIVERCEGUI_MACROS_H ? */

#include "guidriverCEGUI_VFSResourceProvider.h" 

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

VfsResourceProvider::VfsResourceProvider()
    : m_containerMap() ,
      m_groupMap() ,
      m_defaultResourceGroup()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

VfsResourceProvider::~VfsResourceProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
VfsResourceProvider::setDefaultResourceGroup( const CEGUI::String& resourceGroup )
{GUCEF_TRACE;

    m_defaultResourceGroup = resourceGroup;
}
/*-------------------------------------------------------------------------*/

void
VfsResourceProvider::setResourceGroupDirectory( const CEGUI::String& resourceGroup , 
                                                const CEGUI::String& directory     )
{GUCEF_TRACE;

    m_groupMap[ resourceGroup ] = directory;
}

/*-------------------------------------------------------------------------*/

const CEGUI::String&
VfsResourceProvider::getResourceGroupDirectory( const CEGUI::String& resourceGroup )
{GUCEF_TRACE;
    
    return m_groupMap[ resourceGroup ];
}

/*-------------------------------------------------------------------------*/

void 
VfsResourceProvider::clearResourceGroupDirectory( const CEGUI::String& resourceGroup )
{GUCEF_TRACE;
    
    m_groupMap.erase( resourceGroup );
}

/*-------------------------------------------------------------------------*/

void
VfsResourceProvider::loadRawDataContainer( const CEGUI::String& filename      , 
                                           CEGUI::RawDataContainer& output    , 
                                           const CEGUI::String& resourceGroup )
{GUCEF_TRACE;
    
    // everything in CEGUI works based on resource groups
    // make sure we dont use an empty one
    const CEGUI::String* rscGroup = &resourceGroup;
    if ( resourceGroup.empty() )
    {
        rscGroup = &m_defaultResourceGroup;
    }

    CORE::CString filePath;
    CORE::CString fileName = filename;
    TCEStringMap::iterator i = m_groupMap.find( *rscGroup );
    if ( i != m_groupMap.end() )
    {
        filePath = (*i).second;
    }

    filePath = CORE::CombinePath( filePath, fileName );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    VFS::TBasicVfsResourcePtr file = vfs.GetFile( filePath, "rb", false );
    if ( !file.IsNULL() )
    {
        CORE::IOAccessPtr fileAccess = file->GetAccess();
        if ( fileAccess.IsNULL() )
        {
            CORE::CDynamicBuffer memBuffer( *fileAccess );
             
            void* buffer = GUCEF_NULL;
            UInt32 dataSize = 0; 
            memBuffer.RelinquishDataOwnership( buffer, dataSize );

            output.setData( (CEGUI::uint8*) buffer );
            output.setSize( dataSize );

            DataContainerInfoPtr info = DataContainerInfoPtr( new TDataContainerInfo );
            info->fileHandle = file;
            info->requestFilename = fileName;
            info->requestresourceGroup = resourceGroup;
            info->ceContainer = output;

            m_containerMap[ buffer ] = info;
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VfsResourceProvider:loadRawDataContainer: Unable to load resource from path: " + filePath );
        } 
    }
}

/*-------------------------------------------------------------------------*/

void
VfsResourceProvider::unloadRawDataContainer( CEGUI::RawDataContainer& data )
{GUCEF_TRACE;
    
    TContainerMap::iterator i = m_containerMap.find( (void*) data.getDataPtr() );
    if ( i != m_containerMap.end() )
    {
        (*i).second->ceContainer.setData( (CEGUI::uint8*) GUCEF_NULL );
        (*i).second->ceContainer.setSize( 0 );
        m_containerMap.erase( i );
    }
    
    delete[] data.getDataPtr();
    data.setData( (CEGUI::uint8*) GUCEF_NULL );
    data.setSize( 0 );
}

/*-------------------------------------------------------------------------*/

VfsResourceProvider::DataContainerInfoPtr 
VfsResourceProvider::GetInfoOnLoadedContainer( const CEGUI::RawDataContainer& container )
{GUCEF_TRACE;

    TContainerMap::iterator i = m_containerMap.find( (void*) container.getDataPtr() );
    if ( i != m_containerMap.end() )
    {
        return (*i).second;
    }
    return DataContainerInfoPtr();
}

/*-------------------------------------------------------------------------*/

size_t
VfsResourceProvider::getResourceGroupFileNames( std::vector<CEGUI::String>& out_vec ,
                                                const CEGUI::String& file_pattern   ,
                                                const CEGUI::String& resourceGroup  )
{GUCEF_TRACE;
    
    CORE::CString filePath;
    TCEStringMap::iterator i = m_groupMap.find( resourceGroup );
    if ( i != m_groupMap.end() )
    {
        filePath = (*i).second;
    }

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    VFS::CVFS::TStringVector fileList;
    vfs.GetFileList( fileList, filePath, false, false, file_pattern );
    VFS::CVFS::TStringVector::iterator n = fileList.begin();
    while ( n != fileList.end() )
    {   
        out_vec.push_back( (*n) );
        ++n;
    }

    return out_vec.size();
}
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
