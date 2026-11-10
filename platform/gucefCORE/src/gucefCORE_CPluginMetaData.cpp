/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCORE_CPluginMetaData.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

CPluginMetaData::CPluginMetaData( void )
    : CIPluginMetaData()      
    , CIConfigurable()        
    , CTSharedObjCreator< CPluginMetaData, MT::CMutex >( this )
    , m_description()         
    , m_copyright()           
    , m_version()             
    , m_loaderLogicTypeName()
    , m_linkBackModules()
    , m_pluginType()
    , m_moduleFilename()      
    , m_altModuleFilename()   
    , m_modulePath()          
    , m_modulePtr( GUCEF_NULL )
    , m_loadImmediately( true )
    , m_loadFailAllowed( false )
    , m_params()
{GUCEF_TRACE;

    m_version.major = -1;
    m_version.minor = -1;
    m_version.patch = -1;
    m_version.release = -1;

    m_params.SetAllowMultipleValues( true );
}

/*-------------------------------------------------------------------------*/

CPluginMetaData::CPluginMetaData( const CPluginMetaData& src )
    : CIPluginMetaData()                                 
    , CIConfigurable()      
    , CTSharedObjCreator< CPluginMetaData, MT::CMutex >( this )
    , m_description( src.m_description )                 
    , m_copyright( src.m_copyright )                     
    , m_version( src.m_version )                         
    , m_loaderLogicTypeName( src.m_loaderLogicTypeName )
    , m_linkBackModules( src.m_linkBackModules )
    , m_pluginType( src.m_pluginType )
    , m_moduleFilename( src.m_moduleFilename )           
    , m_altModuleFilename( src.m_altModuleFilename )     
    , m_modulePath( src.m_modulePath )                   
    , m_modulePtr( src.m_modulePtr )
    , m_loadImmediately( src.m_loadImmediately )
    , m_loadFailAllowed( src.m_loadFailAllowed )
    , m_params( src.m_params )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginMetaData::CPluginMetaData( const CIPluginMetaData& src )
    : CIPluginMetaData()                                    
    , CIConfigurable()                 
    , CTSharedObjCreator< CPluginMetaData, MT::CMutex >( this )
    , m_description( src.GetDescription() )                 
    , m_copyright( src.GetCopyright() )                     
    , m_version( src.GetVersion() )                         
    , m_loaderLogicTypeName( src.GetLoaderLogicTypeName() )
    , m_linkBackModules( src.GetAllLinkBackModules() )
    , m_pluginType( src.GetPluginType() )
    , m_moduleFilename( src.GetModuleFilename() )           
    , m_altModuleFilename( src.GetAltModuleFilename() )     
    , m_modulePath( src.GetFullModulePath() )               
    , m_modulePtr( src.GetModulePointer() )
    , m_loadImmediately( src.GetLoadImmediately() )
    , m_loadFailAllowed( src.GetLoadFailAllowed() )
    , m_params()
{GUCEF_TRACE;

    src.GetParams( m_params );
    m_params.SetAllowMultipleValues( true );
}

/*-------------------------------------------------------------------------*/

CPluginMetaData::~CPluginMetaData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPluginMetaData&
CPluginMetaData::operator=( const CPluginMetaData& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        m_description = src.m_description;
        m_copyright = src.m_copyright;
        m_version = src.m_version;
        m_loaderLogicTypeName = src.m_loaderLogicTypeName;
        m_linkBackModules = src.m_linkBackModules;
        m_pluginType = src.m_pluginType;
        m_moduleFilename = src.m_moduleFilename;
        m_modulePath = src.m_modulePath;
        m_modulePtr = src.m_modulePtr;
        m_loadImmediately = src.m_loadImmediately;
        m_loadFailAllowed = src.m_loadFailAllowed;
        m_params = src.m_params;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetDescription( const CString& description )
{GUCEF_TRACE;

    m_description = description;
}

/*-------------------------------------------------------------------------*/

CString
CPluginMetaData::GetDescription( void ) const
{GUCEF_TRACE;

    return m_description;
}

/*-------------------------------------------------------------------------*/
    
void
CPluginMetaData::SetCopyright( const CString& copyright )
{GUCEF_TRACE;

    m_copyright = copyright;
}

/*-------------------------------------------------------------------------*/
    
CString
CPluginMetaData::GetCopyright( void ) const
{GUCEF_TRACE;

    return m_copyright;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetVersion( const TVersion& version )
{GUCEF_TRACE;

    m_version = version;
}

/*-------------------------------------------------------------------------*/

TVersion
CPluginMetaData::GetVersion( void ) const
{GUCEF_TRACE;

    return m_version;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetLoaderLogicTypeName( const CString& loaderLogicTypeName )
{GUCEF_TRACE;

    m_loaderLogicTypeName = loaderLogicTypeName;
}

/*-------------------------------------------------------------------------*/
    
CString
CPluginMetaData::GetLoaderLogicTypeName( void ) const
{GUCEF_TRACE;

    return m_loaderLogicTypeName;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetAllLinkBackModules( const CString& linkBackModules )
{GUCEF_TRACE;

    m_linkBackModules = linkBackModules;
}

/*-------------------------------------------------------------------------*/

CString
CPluginMetaData::GetAllLinkBackModules( void ) const
{GUCEF_TRACE;

    return m_linkBackModules;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetPluginType( const CString& pluginType )
{GUCEF_TRACE;

    m_pluginType = pluginType;
}

/*-------------------------------------------------------------------------*/
    
CString
CPluginMetaData::GetPluginType( void ) const
{GUCEF_TRACE;

    return m_pluginType;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetModuleFilename( const CString& moduleFilename )
{GUCEF_TRACE;

    m_moduleFilename = moduleFilename;
}
    
/*-------------------------------------------------------------------------*/

CString
CPluginMetaData::GetModuleFilename( void ) const
{GUCEF_TRACE;

    return m_moduleFilename;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetAltModuleFilename( const CString& altModuleFilename )
{GUCEF_TRACE;

    m_altModuleFilename = altModuleFilename; 
}

/*-------------------------------------------------------------------------*/

CString
CPluginMetaData::GetAltModuleFilename( void ) const
{GUCEF_TRACE;

    return m_altModuleFilename;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetFullModulePath( const CString& modulePath )
{GUCEF_TRACE;

    m_modulePath = modulePath;
}

/*-------------------------------------------------------------------------*/
    
CString
CPluginMetaData::GetFullModulePath( void ) const
{GUCEF_TRACE;

    return m_modulePath;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetModulePointer( void* modulePointer )
{GUCEF_TRACE;

    m_modulePtr = modulePointer;
}

/*-------------------------------------------------------------------------*/
    
void*
CPluginMetaData::GetModulePointer( void ) const
{GUCEF_TRACE;

    return m_modulePtr;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetLoadImmediately( bool loadImmediately )
{GUCEF_TRACE;

    m_loadImmediately = loadImmediately;
}

/*-------------------------------------------------------------------------*/
    
bool
CPluginMetaData::GetLoadImmediately( void ) const
{GUCEF_TRACE;

    return m_loadImmediately;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetLoadFailAllowed( bool loadFailAllowed )
{GUCEF_TRACE;

    m_loadFailAllowed = loadFailAllowed;
}

/*-------------------------------------------------------------------------*/
    
bool
CPluginMetaData::GetLoadFailAllowed( void ) const
{GUCEF_TRACE;

    return m_loadFailAllowed;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::GetParams( CValueList& params ) const
{GUCEF_TRACE;

    params = m_params;
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::SetParams( const CValueList& params )
{GUCEF_TRACE;

    m_params = params;
}

/*-------------------------------------------------------------------------*/

bool
CPluginMetaData::SaveConfig( CDataNode& tree ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPluginMetaData::LoadConfig( const CDataNode& treeroot )
{GUCEF_TRACE;

    CDataNode* node = treeroot.Search( "PluginMetaData", '/', true );
    if ( 0 != node )
    {
        CString value = node->GetAttributeValueOrChildValueByName( "Description" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_description = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "Copyright" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_copyright = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "Path" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_modulePath = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "Filename" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_moduleFilename = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "AltFilename" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_altModuleFilename = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "Version" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_version = StringToVersion( value );
        }
        value = node->GetAttributeValueOrChildValueByName( "Type" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_pluginType = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "LoaderLogicType" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_loaderLogicTypeName = value;
        }
        value = node->GetAttributeValueOrChildValueByName( "LinkBackModules" );
        if ( !value.IsNULLOrEmpty() )
        {
            m_linkBackModules = value;
        }
        CDataNode::TVariantVector paramList = node->GetChildrenValuesByName( "Param" );
        CDataNode::TVariantVector::iterator i = paramList.begin();
        while ( i != paramList.end() )
        {
            m_params.SetUsingKvCombo( (*i), '=', GUCEF_NULL );
            ++i;
        }
        return true;
    }                                       
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CPluginMetaData::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CPluginMetaData";
    return classTypeName; 
}

/*-------------------------------------------------------------------------*/

void
CPluginMetaData::Clear( void )
{GUCEF_TRACE;

    m_description.Clear();
    m_copyright.Clear();
    m_loaderLogicTypeName.Clear();
    m_linkBackModules.Clear();
    m_pluginType.Clear();
    m_moduleFilename.Clear();
    m_modulePath.Clear();
    m_params.Clear();
    m_version.major = 0;
    m_version.minor = 0;
    m_version.patch = 0;
    m_version.release = 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
