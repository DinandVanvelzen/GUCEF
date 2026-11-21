/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#include "gucefASSETSTORE_CAssetSettingsOverrideStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetSettingsOverrideStore::OverridesFileName = "settings-overrides.json";
const CORE::CString CAssetSettingsOverrideStore::AuditFileName = "settings-audit.json";
const UInt32 CAssetSettingsOverrideStore::MaxAuditEntries;

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString
JoinPath( const CORE::CString& folder, const CORE::CString& name )
{GUCEF_TRACE;

    if ( folder.IsNULLOrEmpty() )
        return name;
    return folder.EndsWith( "/" ) ? folder + name : folder + "/" + name;
}

}

/*-------------------------------------------------------------------------*/

CAssetSettingsOverrideStore::CAssetSettingsOverrideStore( void )
    : m_folder()
    , m_overrides()
    , m_audit()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingsOverrideStore::~CAssetSettingsOverrideStore()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsOverrideStore::SetFolder( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_folder = vfsFolder;
    m_overrides.clear();
    m_audit.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingsOverrideStore::GetFolder( void ) const
{GUCEF_TRACE;

    return m_folder;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::IsPersistent( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_folder.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::Load( CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_overrides.clear();
    m_audit.clear();
    if ( m_folder.IsNULLOrEmpty() )
        return true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool success = true;
    const CORE::CString overridesPath = JoinPath( m_folder, OverridesFileName );
    if ( vfs.FileExists( overridesPath ) )
    {
        CORE::CDataNode root;
        if ( vfs.LoadFile( root, overridesPath, "json" ) )
        {
            const CORE::CDataNode* list = root.FindChild( "overrides" );
            if ( GUCEF_NULL != list )
            {
                for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
                {
                    const CORE::CString key = (*i)->GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true );
                    if ( !key.IsNULLOrEmpty() )
                        m_overrides[ key ] = (*i)->GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true );
                }
            }
        }
        else
        {
            problem = "the file " + overridesPath + " cannot be read";
            success = false;
        }
    }
    const CORE::CString auditPath = JoinPath( m_folder, AuditFileName );
    if ( vfs.FileExists( auditPath ) )
    {
        CORE::CDataNode root;
        if ( vfs.LoadFile( root, auditPath, "json" ) )
        {
            const CORE::CDataNode* list = root.FindChild( "entries" );
            if ( GUCEF_NULL != list )
            {
                for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
                    m_audit.push_back( *(*i) );
                while ( m_audit.size() > MaxAuditEntries )
                    m_audit.erase( m_audit.begin() );
            }
        }
        else
        {
            problem += ( problem.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "; " ) ) + "the file " + auditPath + " cannot be read";
            success = false;
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CAssetSettingsOverrideStore::GetOverrides( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_overrides;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::GetOverride( const CORE::CString& key, CORE::CString& value ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CORE::CString::StringMap::const_iterator i = m_overrides.find( key );
    if ( i == m_overrides.end() )
        return false;
    value = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::SetOverrides( const CORE::CString::StringMap& overrides, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_overrides = overrides;
    if ( m_folder.IsNULLOrEmpty() )
        return true;
    CORE::CDataNode root( "settingsOverrides", GUCEF_DATATYPE_OBJECT );
    root.SetAttribute( "version", (UInt32) 1 );
    CORE::CDataNode* list = root.AddChild( "overrides", GUCEF_DATATYPE_ARRAY );
    for ( CORE::CString::StringMap::const_iterator i = m_overrides.begin(); i != m_overrides.end(); ++i )
    {
        CORE::CDataNode* entry = list->AddChild( "override", GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "key", (*i).first );
        entry->SetAttribute( "value", (*i).second );
    }
    return StoreAtomically( OverridesFileName, root, problem );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::AddAuditEntry( const CORE::CDataNode& entry )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_audit.push_back( entry );
    while ( m_audit.size() > MaxAuditEntries )
        m_audit.erase( m_audit.begin() );
    if ( m_folder.IsNULLOrEmpty() )
        return true;
    CORE::CDataNode root( "settingsAudit", GUCEF_DATATYPE_OBJECT );
    root.SetAttribute( "version", (UInt32) 1 );
    CORE::CDataNode* list = root.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_audit.size(); ++i )
        list->AddChild( m_audit[ i ] );
    CORE::CString problem;
    if ( !StoreAtomically( AuditFileName, root, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CAssetSettingsOverrideStore: the audit of the settings could not be saved: " << problem );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetSettingsOverrideStore::GetAuditCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_audit.size();
}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsOverrideStore::GetAuditEntries( UInt32 offset, UInt32 maxEntries, GUCEF::vector< CORE::CDataNode >& entries ) const
{GUCEF_TRACE;

    entries.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=offset; i<m_audit.size() && entries.size()<maxEntries; ++i )
        entries.push_back( m_audit[ i ] );
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsOverrideStore::StoreAtomically( const CORE::CString& fileName, const CORE::CDataNode& root, CORE::CString& problem ) const
{GUCEF_TRACE;

    // written beside the file, then renamed over it: a reader sees the old file or the new one, never half of one
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString path = JoinPath( m_folder, fileName );
    const CORE::CString temporary = path + ".tmp";
    if ( !vfs.StoreAsFile( temporary, root, "json", true ) )
    {
        problem = "the file " + temporary + " could not be written";
        return false;
    }
    if ( !vfs.MoveFileAtomically( temporary, path, true ) )
    {
        vfs.DeleteFile( temporary, true );
        problem = "the file " + path + " could not be replaced";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
