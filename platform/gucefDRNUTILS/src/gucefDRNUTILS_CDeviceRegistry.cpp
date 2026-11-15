/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDeviceRegistry.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CDeviceRegistry::FileName = "devices.json";
const char* const CDeviceRegistry::Format = "gucef.drnutils.DeviceRegistry";

/*-------------------------------------------------------------------------*/

CDeviceRegistry::CDeviceRegistry( void )
    : m_lock()
    , m_devices()
    , m_filePath()
    , m_inMemory( false )
    , m_dirty( false )
    , m_revision( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDeviceRegistry::~CDeviceRegistry()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CDeviceRegistry::SetInMemory( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_inMemory = true;
    m_filePath.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::LoadFile( const CORE::CString& path, TDeviceMap& devices, CORE::CString& error ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    CORE::CDynamicBuffer buffer;
    if ( !buffer.LoadContentFromFile( path ) )
    {
        error = "the file " + path + " cannot be read";
        return false;
    }
    CORE::CDataNode document;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->BuildDataTree( &document, &access ) )
        {
            error = "the file " + path + " is not valid JSON";
            return false;
        }
    }
    if ( Format != document.GetAttributeValueOrChildValueByName( "format" ).AsString() )
    {
        error = "the file " + path + " is not a device registry";
        return false;
    }
    UInt32 version = document.GetAttributeValueOrChildValueByName( "version" ).AsUInt32( 0 );
    if ( version != FormatVersion )
    {
        error = "the device registry " + path + " has the unsupported version " + CORE::ToString( version );
        return false;
    }
    devices.clear();
    const CORE::CDataNode* list = document.FindChild( "devices" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        for ( ; i != list->ConstEnd(); ++i )
        {
            CDeviceRecord record;
            CORE::CString recordError;
            if ( !record.FromDataNode( *(*i), recordError ) )
            {
                error = "the device registry " + path + " holds a malformed device: " + recordError;
                return false;
            }
            devices[ record.deviceId ] = record;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::Load( const CORE::CString& stateDirectory, CORE::CString& error )
{GUCEF_TRACE;

    if ( stateDirectory.IsNULLOrEmpty() )
    {
        error = "the device registry has no state directory";
        return false;
    }
    if ( !CORE::DirExists( stateDirectory ) && !CORE::CreateDirs( stateDirectory ) )
    {
        error = "the state directory " + stateDirectory + " cannot be made";
        return false;
    }
    CORE::CString path = CORE::CombinePath( stateDirectory, FileName );
    CORE::CString previousPath = path + ".prev";

    TDeviceMap devices;
    bool loaded = false;
    if ( CORE::FileExists( path ) )
    {
        CORE::CString fileError;
        loaded = LoadFile( path, devices, fileError );
        if ( !loaded )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DeviceRegistry: " << fileError << "; trying the previous copy" );
            if ( CORE::FileExists( previousPath ) )
            {
                CORE::CString previousError;
                loaded = LoadFile( previousPath, devices, previousError );
                if ( !loaded )
                {
                    error = fileError + "; the previous copy: " + previousError;
                    return false;
                }
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "DeviceRegistry: using the previous copy " << previousPath << " with " << (UInt32) devices.size() << " devices" );
            }
            else
            {
                error = fileError;
                return false;
            }
        }
    }
    else if ( CORE::FileExists( previousPath ) )
    {
        CORE::CString previousError;
        loaded = LoadFile( previousPath, devices, previousError );
        if ( !loaded )
        {
            error = previousError;
            return false;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "DeviceRegistry: " << path << " is missing, using the previous copy" );
    }

    MT::CScopeMutex lock( m_lock );
    m_devices = devices;
    m_filePath = path;
    m_inMemory = false;
    m_dirty = false;
    ++m_revision;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeviceRegistry: " << (UInt32) m_devices.size() << " devices in " << path );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::Save( CORE::CString& error )
{GUCEF_TRACE;

    CORE::CDataNode document( "registry", GUCEF_DATATYPE_OBJECT );
    CORE::CString path;
    UInt64 revision = 0;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_inMemory )
        {
            m_dirty = false;
            return true;
        }
        if ( m_filePath.IsNULLOrEmpty() )
        {
            error = "the device registry was not loaded from a state directory";
            return false;
        }
        path = m_filePath;
        revision = m_revision;
        document.SetAttribute( "format", CORE::CString( Format ) );
        document.SetAttribute( "version", FormatVersion );
        CORE::CDataNode* list = document.AddChild( "devices", GUCEF_DATATYPE_OBJECT );
        TDeviceMap::const_iterator i = m_devices.begin();
        for ( ; i != m_devices.end(); ++i )
        {
            CORE::CDataNode* device = list->AddChild( (*i).first, GUCEF_DATATYPE_OBJECT );
            (*i).second.ToDataNode( *device );
        }
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
    {
        error = "the JSON data tree codec is not available (is dstorepluginJSONPARSER loaded?)";
        return false;
    }
    CORE::CDynamicBuffer buffer;
    {
        CORE::CDynamicBufferAccess access( buffer );
        if ( !codec->StoreDataTree( &document, &access ) )
        {
            error = "the device registry cannot be encoded as JSON";
            return false;
        }
    }

    // the file it replaces stays as the previous copy, so a torn or damaged file is never the only one
    if ( CORE::FileExists( path ) )
    {
        CORE::CString previousPath = path + ".prev";
        if ( CORE::CopyFile( path, previousPath, true ) )
        {
            CORE::SetOwnerOnlyAccess( previousPath );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DeviceRegistry: the previous copy " << previousPath << " cannot be written" );
        }
    }
    else
    {
        // a new file is made owner only before anything is written to it, so it is never more open than that
        CORE::CString problem;
        if ( !CORE::CreateOwnerOnlyFile( path, "", 0, problem ) )
        {
            error = "the device registry " + path + " cannot be made: " + problem;
            return false;
        }
    }
    if ( !CORE::WriteFileAtomically( path, buffer ) )
    {
        error = "the device registry " + path + " cannot be written";
        return false;
    }
    CORE::SetOwnerOnlyAccess( path );

    MT::CScopeMutex lock( m_lock );
    if ( revision == m_revision )
        m_dirty = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::Find( const CORE::CString& deviceId, CDeviceRecord& record ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDeviceMap::const_iterator i = m_devices.find( deviceId );
    if ( i == m_devices.end() )
        return false;
    record = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::FindByPublicKey( const CORE::CString& publicKeyHex, CDeviceRecord& record ) const
{GUCEF_TRACE;

    CORE::CString wanted = publicKeyHex.Lowercase();
    MT::CScopeMutex lock( m_lock );
    TDeviceMap::const_iterator i = m_devices.begin();
    for ( ; i != m_devices.end(); ++i )
    {
        if ( (*i).second.publicKey == wanted )
        {
            record = (*i).second;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDeviceRegistry::Put( const CDeviceRecord& record )
{GUCEF_TRACE;

    CDeviceRecord stored = record;
    stored.publicKey = stored.publicKey.Lowercase();
    MT::CScopeMutex lock( m_lock );
    m_devices[ stored.deviceId ] = stored;
    m_dirty = true;
    ++m_revision;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::Remove( const CORE::CString& deviceId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDeviceMap::iterator i = m_devices.find( deviceId );
    if ( i == m_devices.end() )
        return false;
    m_devices.erase( i );
    m_dirty = true;
    ++m_revision;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDeviceRegistry::GetAll( TDeviceRecordVector& records ) const
{GUCEF_TRACE;

    records.clear();
    MT::CScopeMutex lock( m_lock );
    TDeviceMap::const_iterator i = m_devices.begin();
    for ( ; i != m_devices.end(); ++i )
        records.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

UInt32
CDeviceRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_devices.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CDeviceRegistry::GetActiveCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    UInt32 count = 0;
    TDeviceMap::const_iterator i = m_devices.begin();
    for ( ; i != m_devices.end(); ++i )
    {
        if ( (*i).second.IsActive() )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

void
CDeviceRegistry::NoteSeen( const CORE::CString& deviceId, UInt64 nowUtcMs, const CORE::CString& address )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDeviceMap::iterator i = m_devices.find( deviceId );
    if ( i == m_devices.end() )
        return;
    (*i).second.lastSeenUtcMs = nowUtcMs;
    (*i).second.lastAddress = address;
    m_dirty = true;
    ++m_revision;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceRegistry::IsDirty( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_dirty;
}

/*-------------------------------------------------------------------------*/

UInt64
CDeviceRegistry::GetRevision( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_revision;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDeviceRegistry::GetFilePath( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_filePath;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
