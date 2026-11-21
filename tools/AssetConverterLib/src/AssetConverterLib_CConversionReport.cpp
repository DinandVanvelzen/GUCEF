/*
 *  AssetConverterLib: the conversion report of a pack
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#include "AssetConverterLib_CConversionReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CConversionReport::CConversionReport( void )
    : m_packId()
    , m_licenceRecordId()
    , m_refusalReason()
    , m_refused( false )
    , m_cancelled( false )
    , m_cancellationSource( GUCEF_NULL )
    , m_cancellationCheck( GUCEF_NULL )
    , m_cancellationContext( GUCEF_NULL )
    , m_progressSink( GUCEF_NULL )
    , m_progressContext( GUCEF_NULL )
    , m_locked( false )
    , m_lockHolder()
    , m_mode( PIPELINEMODE_FULL )
    , m_assetFilter()
    , m_entries()
    , m_current( -1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void CConversionReport::SetPackId( const CORE::CString& packId ) { m_packId = packId; }
const CORE::CString& CConversionReport::GetPackId( void ) const { return m_packId; }
void CConversionReport::SetLicenceRecordId( const CORE::CString& recordId ) { m_licenceRecordId = recordId; }
const CORE::CString& CConversionReport::GetLicenceRecordId( void ) const { return m_licenceRecordId; }
bool CConversionReport::IsRefused( void ) const { return m_refused; }
bool CConversionReport::IsCancelled( void ) const { return m_cancelled; }
const CORE::CString& CConversionReport::GetRefusalReason( void ) const { return m_refusalReason; }
const CConversionReport::TEntryVector& CConversionReport::GetEntries( void ) const { return m_entries; }
bool CConversionReport::IsLocked( void ) const { return m_locked; }
const CORE::CString& CConversionReport::GetLockHolder( void ) const { return m_lockHolder; }
void CConversionReport::SetMode( TPipelineMode mode ) { m_mode = mode; }
TPipelineMode CConversionReport::GetMode( void ) const { return m_mode; }
void CConversionReport::SetAssetFilter( const CORE::CString::StringVector& assetGlobs ) { m_assetFilter = assetGlobs; }
const CORE::CString::StringVector& CConversionReport::GetAssetFilter( void ) const { return m_assetFilter; }

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetRefused( const CORE::CString& reason )
{GUCEF_TRACE;

    m_refused = true;
    m_refusalReason = reason;
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetCancelled( void )
{GUCEF_TRACE;

    m_cancelled = true;
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetCancellationSource( const CORE::CTask* task )
{GUCEF_TRACE;

    m_cancellationSource = task;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::IsCancellationRequested( void ) const
{GUCEF_TRACE;

    return ( GUCEF_NULL != m_cancellationSource && m_cancellationSource->IsCancellationRequested() ) ||
           ( GUCEF_NULL != m_cancellationCheck && m_cancellationCheck( m_cancellationContext ) );
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetCancellationCheck( TCancellationCheck check, void* context )
{GUCEF_TRACE;

    m_cancellationCheck = check;
    m_cancellationContext = context;
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetProgressSink( TProgressSink sink, void* context )
{GUCEF_TRACE;

    m_progressSink = sink;
    m_progressContext = context;
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::ReportProgress( Float32 fraction ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_progressSink )
        m_progressSink( m_progressContext, fraction );
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::SetLocked( const CORE::CString& holder )
{GUCEF_TRACE;

    m_locked = true;
    m_lockHolder = holder;
}

/*-------------------------------------------------------------------------*/

CConversionReportEntry&
CConversionReport::SelectAsset( const CORE::CString& assetPath )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        if ( m_entries[ i ].assetPath == assetPath )
        {
            m_current = (Int32) i;
            return m_entries[ i ];
        }
    }
    CConversionReportEntry entry;
    entry.assetPath = assetPath;
    m_entries.push_back( entry );
    m_current = (Int32) m_entries.size() - 1;
    return m_entries.back();
}

/*-------------------------------------------------------------------------*/

CConversionReportEntry*
CConversionReport::GetCurrentEntry( void )
{GUCEF_TRACE;

    if ( m_current < 0 || (size_t) m_current >= m_entries.size() )
        return GUCEF_NULL;
    return &m_entries[ m_current ];
}

/*-------------------------------------------------------------------------*/

const CConversionReportEntry*
CConversionReport::FindEntry( const CORE::CString& assetPath ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].assetPath == assetPath )
            return &m_entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CConversionReportEntry*
CConversionReport::FindEntry( const CORE::CString& assetPath )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].assetPath == assetPath )
            return &m_entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::AddItem( TReportItemKind kind, const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message )
{GUCEF_TRACE;

    CConversionReportEntry* entry = GetCurrentEntry();
    if ( GUCEF_NULL == entry )
    {
        const Int32 previous = m_current;
        entry = &SelectAsset( CORE::CString::Empty );
        m_current = previous;
    }
    entry->items.push_back( CConversionReportItem( kind, source, subject, message ) );
}

/*-------------------------------------------------------------------------*/

void CConversionReport::AddInfo( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message ) { AddItem( REPORTITEM_INFO, source, subject, message ); }
void CConversionReport::AddApproximation( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message ) { AddItem( REPORTITEM_APPROXIMATION, source, subject, message ); }
void CConversionReport::AddSkipped( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message ) { AddItem( REPORTITEM_SKIPPED, source, subject, message ); }
void CConversionReport::AddWarning( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message ) { AddItem( REPORTITEM_WARNING, source, subject, message ); }
void CConversionReport::AddError( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message ) { AddItem( REPORTITEM_ERROR, source, subject, message ); }

/*-------------------------------------------------------------------------*/

void
CConversionReport::AddOutputFile( const CORE::CString& path, const CORE::CString::StringVector& references, const CORE::CString& profile )
{GUCEF_TRACE;

    CConversionReportEntry* entry = GetCurrentEntry();
    if ( GUCEF_NULL == entry )
    {
        const Int32 previous = m_current;
        entry = &SelectAsset( CORE::CString::Empty );
        m_current = previous;
    }
    CConversionReportFile file;
    file.path = path;
    file.profile = profile;
    file.references = references;
    entry->AddFile( file );
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::Merge( const CConversionReport& other )
{GUCEF_TRACE;

    for ( size_t i=0; i<other.m_entries.size(); ++i )
    {
        CConversionReportEntry* entry = FindEntry( other.m_entries[ i ].assetPath );
        if ( GUCEF_NULL != entry )
            *entry = other.m_entries[ i ];
        else
            m_entries.push_back( other.m_entries[ i ] );
    }
    if ( other.m_refused )
        SetRefused( other.m_refusalReason );
    if ( other.m_cancelled )
        SetCancelled();
    if ( other.m_locked )
        SetLocked( other.m_lockHolder );
}

/*-------------------------------------------------------------------------*/

UInt32
CConversionReport::GetAssetCount( TAssetStatus status ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( !m_entries[ i ].assetPath.IsNULLOrEmpty() && m_entries[ i ].GetStatus() == status )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CConversionReport::GetItemCount( TReportItemKind kind ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
        count += m_entries[ i ].GetItemCount( kind );
    return count;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::HasFailures( void ) const
{GUCEF_TRACE;

    if ( m_refused || m_cancelled || m_locked )
        return true;
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].HasErrors() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "pack", m_packId );
    node.SetAttribute( "licenceRecord", m_licenceRecordId );
    if ( m_refused )
    {
        node.SetAttribute( "refused", true );
        node.SetAttribute( "refusalReason", m_refusalReason );
    }
    if ( m_cancelled )
        node.SetAttribute( "cancelled", true );
    if ( m_locked )
    {
        node.SetAttribute( "locked", true );
        node.SetAttribute( "lockHolder", m_lockHolder );
    }
    if ( PIPELINEMODE_FULL != m_mode )
        node.SetAttribute( "mode", ToString( m_mode ) );
    if ( !m_assetFilter.empty() )
        node.SetValuesOfChildByName( "assetFilter", m_assetFilter );
    CORE::CDataNode* counts = node.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
    for ( Int32 s=0; s<ASSETSTATUS_COUNT; ++s )
        counts->SetAttribute( ToString( (TAssetStatus) s ), GetAssetCount( (TAssetStatus) s ) );
    CORE::CDataNode* assets = node.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_entries.size(); ++i )
        m_entries[ i ].Serialize( *assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    Clear();
    m_packId = node.GetAttributeValueOrChildValueByName( "pack" ).AsString();
    m_licenceRecordId = node.GetAttributeValueOrChildValueByName( "licenceRecord" ).AsString();
    m_refused = node.GetAttributeValueOrChildValueByName( "refused" ).AsBool( false );
    m_refusalReason = node.GetAttributeValueOrChildValueByName( "refusalReason" ).AsString();
    m_cancelled = node.GetAttributeValueOrChildValueByName( "cancelled" ).AsBool( false );
    m_locked = node.GetAttributeValueOrChildValueByName( "locked" ).AsBool( false );
    m_lockHolder = node.GetAttributeValueOrChildValueByName( "lockHolder" ).AsString();
    FromString( node.GetAttributeValueOrChildValueByName( "mode" ).AsString(), m_mode );
    node.GetValuesOfChildByName( "assetFilter", m_assetFilter );
    const CORE::CDataNode* assets = node.FindChild( "assets" );
    if ( GUCEF_NULL != assets )
    {
        CORE::CDataNode::const_iterator i = assets->ConstBegin();
        while ( i != assets->ConstEnd() )
        {
            CConversionReportEntry entry;
            entry.Deserialize( *(*i) );
            m_entries.push_back( entry );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::SaveJson( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CDataNode node( "report", GUCEF_DATATYPE_OBJECT );
    Serialize( node );
    return CCanonicalJson::Store( vfsPath, node );
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::LoadJson( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    CORE::CDataNode node;
    if ( !CCanonicalJson::Load( vfsPath, node ) )
        return false;
    return Deserialize( node );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CConversionReport::ToSummaryText( void ) const
{GUCEF_TRACE;

    CORE::CString text = "Conversion report of pack " + m_packId + "\n";
    text += "Licence record: " + ( m_licenceRecordId.IsNULLOrEmpty() ? CORE::CString( "(none)" ) : m_licenceRecordId ) + "\n";
    if ( m_refused )
        text += "REFUSED: " + m_refusalReason + "\n";
    if ( m_cancelled )
        text += "CANCELLED: the run was asked to stop before it was done\n";
    if ( m_locked )
        text += "LOCKED: the intermediate is in use by " + m_lockHolder + "\n";
    if ( PIPELINEMODE_FULL != m_mode )
        text += "Mode: " + ToString( m_mode ) + "\n";
    if ( !m_assetFilter.empty() )
    {
        text += "Assets restricted to:";
        for ( size_t f=0; f<m_assetFilter.size(); ++f )
            text += " " + m_assetFilter[ f ];
        text += "\n";
    }
    for ( Int32 s=0; s<ASSETSTATUS_COUNT; ++s )
        text += ToString( (TAssetStatus) s ) + ": " + CORE::UInt32ToString( GetAssetCount( (TAssetStatus) s ) ) + "\n";
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        const CConversionReportEntry& entry = m_entries[ i ];
        bool headed = false;
        for ( size_t j=0; j<entry.items.size(); ++j )
        {
            const CConversionReportItem& item = entry.items[ j ];
            if ( REPORTITEM_INFO == item.kind )
                continue;
            if ( !headed )
            {
                text += "\n" + ( entry.assetPath.IsNULLOrEmpty() ? CORE::CString( "(pack)" ) : entry.assetPath ) + " [" + ToString( entry.GetStatus() ) + "]\n";
                headed = true;
            }
            text += "  " + ToString( item.kind ) + " (" + item.source + ") " + item.subject + ": " + item.message + "\n";
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReport::SaveSummary( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    const CORE::CString text = ToSummaryText();
    CORE::CDynamicBuffer buffer( text.C_String(), text.ByteSize() > 0 ? text.ByteSize()-1 : 0, true );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = CORE::StripFilename( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );
    return vfs.StoreAsFile( vfsPath, buffer, 0, true );
}

/*-------------------------------------------------------------------------*/

void
CConversionReport::Clear( void )
{GUCEF_TRACE;

    m_packId.Clear();
    m_licenceRecordId.Clear();
    m_refusalReason.Clear();
    m_refused = false;
    m_cancelled = false;
    m_locked = false;
    m_lockHolder.Clear();
    m_mode = PIPELINEMODE_FULL;
    m_assetFilter.clear();
    m_entries.clear();
    m_current = -1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
