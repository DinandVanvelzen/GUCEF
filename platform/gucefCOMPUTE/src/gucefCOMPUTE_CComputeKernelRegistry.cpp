/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeKernelRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CComputeKernelRegistry::OverrideSectionName = "ComputeKernelArtefact";

/*-------------------------------------------------------------------------*/

namespace {

CString
OverrideCacheKey( const CString& kernelId, const CString& backendName )
{
    return kernelId + "|" + backendName;
}

CString
HashOrNone( const CString& hash )
{
    return hash.IsNULLOrEmpty() ? CString( "<none>" ) : hash;
}

}

/*-------------------------------------------------------------------------*/

CComputeKernelRegistry::CComputeKernelRegistry( void )
    : m_lock()
    , m_kernels()
    , m_overrideRoot()
    , m_overrideCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeKernelRegistry::~CComputeKernelRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::RegisterKernel( const CComputeKernelDecl& decl, const void* owner )
{GUCEF_TRACE;

    if ( !decl.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Refusing the invalid kernel declaration \"" << decl.GetKernelId() << "\"" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );
    if ( m_kernels.end() != m_kernels.find( decl.GetKernelId() ) )
        return false;

    SKernelEntry& entry = m_kernels[ decl.GetKernelId() ];
    entry.decl = TComputeKernelDeclPtr( GUCEF_NEW CComputeKernelDecl( decl ) );
    entry.owner = owner;
    m_overrideCache.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::RegisterArtefact( const CString& kernelId, const CComputeKernelArtefact& artefact )
{GUCEF_TRACE;

    if ( !artefact.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Refusing an invalid artefact for kernel \"" << kernelId << "\"" );
        return false;
    }

    MT::CScopeMutex lock( m_lock );
    TKernelMap::iterator i = m_kernels.find( kernelId );
    if ( i == m_kernels.end() )
        return false;
    (*i).second.artefacts[ artefact.GetBackendName() ] = TComputeKernelArtefactPtr( GUCEF_NEW CComputeKernelArtefact( artefact ) );
    m_overrideCache.clear();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelRegistry::UnregisterModuleKernels( const void* owner )
{GUCEF_TRACE;

    // the declarations and artefacts are released outside the lock
    TKernelMap removed;
    UInt32 count = 0;
    {
        MT::CScopeMutex lock( m_lock );
        TKernelMap::iterator i = m_kernels.begin();
        while ( i != m_kernels.end() )
        {
            if ( (*i).second.owner == owner )
            {
                removed[ (*i).first ] = (*i).second;
                m_kernels.erase( i++ );
                ++count;
            }
            else
            {
                ++i;
            }
        }
        if ( count > 0 )
            m_overrideCache.clear();
    }
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::TryGetKernel( const CString& kernelId, TComputeKernelDeclPtr& decl ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TKernelMap::const_iterator i = m_kernels.find( kernelId );
    if ( i == m_kernels.end() )
        return false;
    decl = (*i).second.decl;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::IsKernelRegistered( const CString& kernelId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_kernels.end() != m_kernels.find( kernelId );
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::GetArtefact( const CString& kernelId, const CString& backendName, TComputeKernelArtefactPtr& artefact ) const
{GUCEF_TRACE;

    const CString backend = backendName.Lowercase();
    TComputeKernelDeclPtr decl;
    TComputeKernelArtefactPtr embedded;
    CString root;
    {
        MT::CScopeMutex lock( m_lock );
        TKernelMap::const_iterator i = m_kernels.find( kernelId );
        if ( i == m_kernels.end() )
            return false;
        decl = (*i).second.decl;
        TArtefactMap::const_iterator a = (*i).second.artefacts.find( backend );
        if ( a != (*i).second.artefacts.end() )
            embedded = (*a).second;
        root = m_overrideRoot;

        if ( root.IsNULLOrEmpty() || backend == "cpu" )
        {
            if ( embedded.IsNULL() )
                return false;
            artefact = embedded;
            return true;
        }

        TOverrideCache::const_iterator c = m_overrideCache.find( OverrideCacheKey( kernelId, backend ) );
        if ( c != m_overrideCache.end() )
        {
            if ( (*c).second.hasOverride )
                artefact = (*c).second.artefact;
            else
                artefact = embedded;
            return !artefact.IsNULL();
        }
    }

    // the files are read without holding the lock
    TComputeKernelArtefactPtr loaded;
    const bool hasOverride = LoadOverride( root, *decl, backend, embedded, loaded );

    MT::CScopeMutex lock( m_lock );
    if ( root == m_overrideRoot )
    {
        SOverrideCacheEntry& entry = m_overrideCache[ OverrideCacheKey( kernelId, backend ) ];
        entry.hasOverride = hasOverride;
        entry.artefact = loaded;
    }
    artefact = hasOverride ? loaded : embedded;
    return !artefact.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::LoadOverride( const CString& root, const CComputeKernelDecl& decl, const CString& backendName,
                                      const TComputeKernelArtefactPtr& embedded, TComputeKernelArtefactPtr& loaded ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CString directory = root;
    CORE::AppendToPath( directory, decl.GetKernelId() );
    CString iniPath = directory;
    CORE::AppendToPath( iniPath, backendName + ".ini" );
    if ( !vfs.FileExists( iniPath ) )
        return false;

    CORE::CDataNode document;
    if ( !vfs.LoadFile( document, iniPath, "ini" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: The kernel override \"" << iniPath << "\" cannot be read" );
        return false;
    }
    const CORE::CDataNode* section = document.Find( OverrideSectionName );
    if ( GUCEF_NULL == section )
        section = &document;

    const UInt32 version = section->GetAttributeValueOrChildValueByName( "interfaceVersion" ).AsUInt32( 0, true );
    const TComputeArtefactFormat format = StringToComputeArtefactFormat( section->GetAttributeValueOrChildValueByName( "format" ).AsString( CString::Empty, true ) );
    const CString entryPoint = section->GetAttributeValueOrChildValueByName( "entryPoint" ).AsString( CString::Empty, true );
    const CString sourceHash = section->GetAttributeValueOrChildValueByName( "sourceHash" ).AsString( CString::Empty, true );
    const CString compiler = section->GetAttributeValueOrChildValueByName( "compiler" ).AsString( CString::Empty, true );
    const CString embeddedHash = embedded.IsNULL() ? CString( "<no embedded artefact>" ) : HashOrNone( embedded->GetSourceHash() );

    if ( version != decl.GetInterfaceVersion() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Ignoring the override of kernel \"" << decl.GetKernelId() << "\" for backend \"" <<
            backendName << "\": it declares interface version " << version << ", the kernel has version " << decl.GetInterfaceVersion() <<
            " (override hash " << HashOrNone( sourceHash ) << ", embedded hash " << embeddedHash << ")" );
        return false;
    }

    const char* extension = ComputeArtefactFormatToFileExtension( format );
    if ( ARTEFACTFORMAT_UNKNOWN == format || ARTEFACTFORMAT_CPU == format || '\0' == *extension )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Ignoring the override \"" << iniPath << "\": it has no usable format" );
        return false;
    }

    CString dataPath = directory;
    CORE::AppendToPath( dataPath, backendName + "." + extension );
    CORE::CDynamicBuffer data;
    if ( !vfs.LoadFile( data, dataPath, "rb" ) || 0 == data.GetDataSize() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Ignoring the override \"" << iniPath << "\": its artefact \"" << dataPath << "\" cannot be read" );
        return false;
    }

    CComputeKernelArtefact* artefact = GUCEF_NEW CComputeKernelArtefact();
    artefact->SetBackendName( backendName );
    artefact->SetFormat( format );
    artefact->SetEntryPoint( entryPoint );
    artefact->SetOwnedData( data );
    artefact->SetInterfaceVersion( version );
    artefact->SetSourceHash( sourceHash );
    artefact->SetCompilerId( compiler );
    loaded = TComputeKernelArtefactPtr( artefact );
    if ( !artefact->IsValid() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Ignoring the override \"" << iniPath << "\": it has no entry point" );
        loaded.Unlink();
        return false;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ComputeKernelRegistry: Using the override of kernel \"" << decl.GetKernelId() << "\" for backend \"" << backendName <<
        "\" from \"" << dataPath << "\" (override hash " << HashOrNone( sourceHash ) << ", embedded hash " << embeddedHash << ")" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::HasArtefact( const CString& kernelId, const CString& backendName ) const
{GUCEF_TRACE;

    TComputeKernelArtefactPtr artefact;
    return GetArtefact( kernelId, backendName, artefact );
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeKernelRegistry::GetKernelCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return static_cast< UInt32 >( m_kernels.size() );
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelRegistry::GetKernelIds( CString::StringVector& kernelIds ) const
{GUCEF_TRACE;

    kernelIds.clear();
    MT::CScopeMutex lock( m_lock );
    for ( TKernelMap::const_iterator i = m_kernels.begin(); i != m_kernels.end(); ++i )
        kernelIds.push_back( (*i).first );
}

/*-------------------------------------------------------------------------*/

bool
CComputeKernelRegistry::IsArtefactSourceHashCurrent( const CString& kernelId, const CString& backendName, const CString& currentSourceHash ) const
{GUCEF_TRACE;

    TComputeKernelArtefactPtr artefact;
    if ( !GetArtefact( kernelId, backendName, artefact ) )
        return true;
    if ( artefact->GetSourceHash().IsNULLOrEmpty() || currentSourceHash.IsNULLOrEmpty() )
        return true;
    return artefact->GetSourceHash().Equals( currentSourceHash, false );
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelRegistry::SetOverrideRoot( const CString& vfsRoot )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( vfsRoot != m_overrideRoot )
    {
        m_overrideRoot = vfsRoot;
        m_overrideCache.clear();
    }
}

/*-------------------------------------------------------------------------*/

CString
CComputeKernelRegistry::GetOverrideRoot( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_overrideRoot;
}

/*-------------------------------------------------------------------------*/

void
CComputeKernelRegistry::ClearOverrideCache( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_overrideCache.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
