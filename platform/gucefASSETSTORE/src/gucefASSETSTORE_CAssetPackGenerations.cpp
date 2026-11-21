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
#ifndef GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#include "gucefASSETSTORE_CAssetStoreJson.h"
#define GUCEF_ASSETSTORE_CASSETSTOREJSON_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREJSON_H ? */

#include "gucefASSETSTORE_CAssetPackGenerations.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
Join( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    return CPackStorageLayout::Join( a, b );
}

bool
IsKnownArea( const CORE::CString& area )
{GUCEF_TRACE;

    return CPackStorageLayout::IntermediateArea == area || CPackStorageLayout::CookedArea == area;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetPackGenerations::PreviousFolderName = "previous";
const CORE::CString CAssetPackGenerations::GenerationFileName = "generation.json";
const CORE::CString CAssetPackGenerations::MarkerFileName = "swap.json";

/*-------------------------------------------------------------------------*/

CAssetPackGenerations::CAssetPackGenerations( const CPackStorageLayout& layout, const CORE::CString& locationRoot )
    : m_layout( layout )
    , m_locationRoot( locationRoot )
    , m_root( layout.GetGenerationsFolder( locationRoot ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::LoadJson( const CORE::CString& vfsPath, CORE::CDataNode& root )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDynamicBuffer bytes;
    if ( vfsPath.IsNULLOrEmpty() || !vfs.FileExists( vfsPath ) || !vfs.LoadFile( bytes, vfsPath, "rb" ) )
        return false;
    const CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    root.Clear();
    return CAssetStoreJson::FromText( text, root ) && GUCEF_DATATYPE_OBJECT == root.GetNodeType();
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::StoreJson( const CORE::CString& vfsPath, const CORE::CDataNode& root )
{GUCEF_TRACE;

    CORE::CString text;
    if ( !CAssetStoreJson::ToText( root, text ) )
        return false;
    CORE::CDynamicBuffer bytes;
    bytes.Append( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    return VFS::CVfsGlobal::Instance()->GetVfs().StoreAsFile( vfsPath, bytes, 0, true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::IsSupported( void ) const
{GUCEF_TRACE;

    return m_layout.IsAreaLayout() && !m_root.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetPackGenerations::GetRoot( void ) const
{GUCEF_TRACE;

    return m_root;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackGenerations::GetPreviousFolder( void ) const
{GUCEF_TRACE;

    return Join( m_root, PreviousFolderName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetPackGenerations::GetCurrentFolder( const CORE::CString& area ) const
{GUCEF_TRACE;

    if ( CPackStorageLayout::IntermediateArea == area )
        return m_layout.GetIntermediateFolder( m_locationRoot, CORE::CString::Empty );
    if ( CPackStorageLayout::CookedArea == area )
        return m_layout.GetCookedFolder( m_locationRoot, CORE::CString::Empty );
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::HasPrevious( void ) const
{GUCEF_TRACE;

    return IsSupported() && VFS::CVfsGlobal::Instance()->GetVfs().FileExists( Join( GetPreviousFolder(), GenerationFileName ) );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::ReadPrevious( CORE::CDataNode& generation ) const
{GUCEF_TRACE;

    return IsSupported() && LoadJson( Join( GetPreviousFolder(), GenerationFileName ), generation );
}

/*-------------------------------------------------------------------------*/

void
CAssetPackGenerations::GetPreviousAreas( CORE::CString::StringVector& areas ) const
{GUCEF_TRACE;

    areas.clear();
    if ( !IsSupported() )
        return;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString previous = GetPreviousFolder();
    if ( vfs.DirExists( Join( previous, CPackStorageLayout::IntermediateArea ) ) )
        areas.push_back( CPackStorageLayout::IntermediateArea );
    if ( vfs.DirExists( Join( previous, CPackStorageLayout::CookedArea ) ) )
        areas.push_back( CPackStorageLayout::CookedArea );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::EnsureFolder( const CORE::CString& folder ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    return vfs.DirExists( folder ) || vfs.CreateDir( folder );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::WriteMarker( const CORE::CDataNode& marker )
{GUCEF_TRACE;

    return EnsureFolder( m_root ) && StoreJson( Join( m_root, MarkerFileName ), marker );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::Replace( const CORE::CString& stagingRoot, const CORE::CString::StringVector& areas, const CORE::CDataNode& replacedGeneration, CORE::CString& problem )
{GUCEF_TRACE;

    if ( !IsSupported() )
    {
        problem = "the pack is of the earlier storage layout: it has no generations";
        return false;
    }
    CORE::CString note;
    Recover( note );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDataNode marker( "swap", GUCEF_DATATYPE_OBJECT );
    marker.SetAttribute( "op", "replace" );
    CORE::CDataNode* areaList = marker.AddChild( "areas", GUCEF_DATATYPE_ARRAY );
    for ( size_t a=0; a<areas.size(); ++a )
    {
        if ( !IsKnownArea( areas[ a ] ) )
        {
            problem = "the area " + areas[ a ] + " is no output area of a pack";
            return false;
        }
        CORE::CDataNode* entry = areaList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "name", areas[ a ] );
        entry->SetAttribute( "hadCurrent", vfs.DirExists( GetCurrentFolder( areas[ a ] ) ) );
    }
    if ( !WriteMarker( marker ) )
    {
        problem = "the marker of the swap cannot be written in " + m_root;
        return false;
    }
    const CORE::CString previous = GetPreviousFolder();
    const CORE::CString older = Join( m_root, "older" );
    if ( vfs.DirExists( older ) )
        vfs.DeleteDir( older, true );
    if ( vfs.DirExists( previous ) && !vfs.MoveDir( previous, older ) )
    {
        problem = "the previous generation cannot be set aside in " + older;
        vfs.DeleteFile( Join( m_root, MarkerFileName ), true );
        return false;
    }
    if ( !EnsureFolder( previous ) )
    {
        CORE::CString undoProblem;
        Undo( undoProblem );
        problem = "the folder " + previous + " cannot be made";
        return false;
    }
    for ( size_t a=0; a<areas.size(); ++a )
    {
        const CORE::CString current = GetCurrentFolder( areas[ a ] );
        if ( vfs.DirExists( current ) && !vfs.MoveDir( current, Join( previous, areas[ a ] ) ) )
        {
            CORE::CString undoProblem;
            Undo( undoProblem );
            problem = "the current " + areas[ a ] + " folder cannot be moved to the previous generation";
            return false;
        }
    }
    for ( size_t a=0; a<areas.size(); ++a )
    {
        const CORE::CString staged = Join( stagingRoot, areas[ a ] );
        if ( !vfs.DirExists( staged ) )
            continue;
        const CORE::CString current = GetCurrentFolder( areas[ a ] );
        const Int32 slash = current.HasChar( '/', false );
        if ( slash > 0 )
            EnsureFolder( current.SubstrToIndex( (UInt32) slash, true ) );
        if ( !vfs.MoveDir( staged, current ) )
        {
            CORE::CString undoProblem;
            Undo( undoProblem );
            problem = "the regenerated " + areas[ a ] + " folder cannot be put in place";
            return false;
        }
    }
    CORE::CString::StringVector kept;
    GetPreviousAreas( kept );
    if ( kept.empty() )
    {
        // the pack had no outputs before: there is nothing to roll back to
        vfs.DeleteDir( previous, true );
        return true;
    }
    if ( !StoreJson( Join( previous, GenerationFileName ), replacedGeneration ) )
    {
        CORE::CString undoProblem;
        Undo( undoProblem );
        problem = "the generation file of the previous generation cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::Rollback( const CORE::CDataNode& currentGeneration, CORE::CDataNode& restored, CORE::CString::StringVector& areas, CORE::CString& problem )
{GUCEF_TRACE;

    areas.clear();
    if ( !IsSupported() )
    {
        problem = "the pack is of the earlier storage layout: it has no generations";
        return false;
    }
    CORE::CString note;
    Recover( note );
    if ( !ReadPrevious( restored ) )
    {
        problem = "there is no previous generation of the outputs of the pack to roll back to";
        return false;
    }
    GetPreviousAreas( areas );
    if ( areas.empty() )
    {
        problem = "the previous generation holds no outputs";
        return false;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString previous = GetPreviousFolder();
    const CORE::CString swap = Join( m_root, "swap" );
    if ( vfs.DirExists( swap ) )
        vfs.DeleteDir( swap, true );
    CORE::CDataNode marker( "swap", GUCEF_DATATYPE_OBJECT );
    marker.SetAttribute( "op", "rollback" );
    marker.AddChild( restored )->SetName( "previousGeneration" );
    CORE::CDataNode* areaList = marker.AddChild( "areas", GUCEF_DATATYPE_ARRAY );
    for ( size_t a=0; a<areas.size(); ++a )
    {
        CORE::CDataNode* entry = areaList->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "name", areas[ a ] );
        entry->SetAttribute( "hadCurrent", vfs.DirExists( GetCurrentFolder( areas[ a ] ) ) );
        entry->SetAttribute( "done", false );
    }
    if ( !WriteMarker( marker ) || !EnsureFolder( swap ) )
    {
        problem = "the marker of the swap cannot be written in " + m_root;
        return false;
    }
    for ( size_t a=0; a<areas.size(); ++a )
    {
        const CORE::CString current = GetCurrentFolder( areas[ a ] );
        const CORE::CString parked = Join( swap, areas[ a ] );
        const CORE::CString kept = Join( previous, areas[ a ] );
        bool moved = !vfs.DirExists( current ) || vfs.MoveDir( current, parked );
        moved = moved && vfs.MoveDir( kept, current );
        moved = moved && ( !vfs.DirExists( parked ) || vfs.MoveDir( parked, kept ) );
        if ( !moved )
        {
            CORE::CString undoProblem;
            Undo( undoProblem );
            problem = "the " + areas[ a ] + " folders cannot be exchanged with the previous generation";
            return false;
        }
        // the marker says which areas are exchanged completely
        CORE::CDataNode::iterator e = areaList->Begin();
        for ( size_t i=0; e != areaList->End(); ++e, ++i )
            if ( i == a )
                (*e)->SetAttribute( "done", true );
        WriteMarker( marker );
    }
    if ( !StoreJson( Join( previous, GenerationFileName ), currentGeneration ) )
    {
        CORE::CString undoProblem;
        Undo( undoProblem );
        problem = "the generation file of the previous generation cannot be written";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackGenerations::Commit( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.DeleteFile( Join( m_root, MarkerFileName ), true );
    if ( vfs.DirExists( Join( m_root, "older" ) ) )
        vfs.DeleteDir( Join( m_root, "older" ), true );
    if ( vfs.DirExists( Join( m_root, "swap" ) ) )
        vfs.DeleteDir( Join( m_root, "swap" ), true );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::Undo( CORE::CString& problem )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CDataNode marker;
    if ( !LoadJson( Join( m_root, MarkerFileName ), marker ) )
        return true;
    const CORE::CString op = marker.GetAttributeValueOrChildValueByName( "op" ).AsString( CORE::CString::Empty, true );
    const CORE::CString previous = GetPreviousFolder();
    const CORE::CDataNode* areaList = marker.FindChild( "areas" );
    bool ok = true;
    if ( GUCEF_NULL != areaList )
    {
        CORE::CDataNode::const_iterator e = areaList->ConstBegin();
        for ( ; e != areaList->ConstEnd(); ++e )
        {
            const CORE::CString area = (*e)->GetAttributeValueOrChildValueByName( "name" ).AsString( CORE::CString::Empty, true );
            const bool hadCurrent = (*e)->GetAttributeValueOrChildValueByName( "hadCurrent" ).AsBool( false, true );
            if ( !IsKnownArea( area ) )
                continue;
            const CORE::CString current = GetCurrentFolder( area );
            const CORE::CString kept = Join( previous, area );
            if ( "replace" == op )
            {
                // the regenerated folder goes, the one it replaced comes back
                if ( vfs.DirExists( kept ) )
                {
                    if ( vfs.DirExists( current ) )
                        vfs.DeleteDir( current, true );
                    ok = vfs.MoveDir( kept, current ) && ok;
                }
                else
                if ( !hadCurrent && vfs.DirExists( current ) )
                    vfs.DeleteDir( current, true );
            }
            else
            if ( "rollback" == op )
            {
                const CORE::CString parked = Join( Join( m_root, "swap" ), area );
                const bool done = (*e)->GetAttributeValueOrChildValueByName( "done" ).AsBool( false, true );
                if ( done )
                {
                    // exchanged completely: exchanged back
                    bool moved = !vfs.DirExists( current ) || vfs.MoveDir( current, parked );
                    moved = moved && ( !vfs.DirExists( kept ) || vfs.MoveDir( kept, current ) );
                    moved = moved && ( !vfs.DirExists( parked ) || vfs.MoveDir( parked, kept ) );
                    ok = moved && ok;
                }
                else
                if ( vfs.DirExists( parked ) )
                {
                    // the current folder was parked; what is current now is the previous generation's
                    if ( vfs.DirExists( current ) && !vfs.DirExists( kept ) )
                        ok = vfs.MoveDir( current, kept ) && ok;
                    ok = vfs.MoveDir( parked, current ) && ok;
                }
            }
        }
    }
    if ( "replace" == op )
    {
        // the generation before comes back as the previous one
        const CORE::CString older = Join( m_root, "older" );
        if ( vfs.DirExists( older ) )
        {
            if ( vfs.DirExists( previous ) )
                vfs.DeleteDir( previous, true );
            ok = vfs.MoveDir( older, previous ) && ok;
        }
        else
        {
            // no generation before: what is left of the previous folder is the attempt's
            CORE::CString::StringVector left;
            GetPreviousAreas( left );
            if ( left.empty() && vfs.DirExists( previous ) && !vfs.FileExists( Join( previous, GenerationFileName ) ) )
                vfs.DeleteDir( previous, true );
        }
    }
    else
    if ( "rollback" == op )
    {
        const CORE::CDataNode* before = marker.FindChild( "previousGeneration" );
        if ( GUCEF_NULL != before )
            StoreJson( Join( previous, GenerationFileName ), *before );
        if ( vfs.DirExists( Join( m_root, "swap" ) ) )
            vfs.DeleteDir( Join( m_root, "swap" ), true );
    }
    if ( !ok )
    {
        problem = "the files of the interrupted " + op + " in " + m_root + " could not all be taken back; the marker is kept";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AssetPackGenerations: " << problem );
        return false;
    }
    vfs.DeleteFile( Join( m_root, MarkerFileName ), true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::Recover( CORE::CString& note )
{GUCEF_TRACE;

    note.Clear();
    if ( !IsSupported() || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( Join( m_root, MarkerFileName ) ) )
        return false;
    CORE::CString problem;
    const bool undone = Undo( problem );
    note = undone ? "an interrupted swap of the outputs in " + m_root + " was undone" : problem;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetPackGenerations: " << note );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetPackGenerations::MakeGeneration( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CDataNode* extra, CORE::CDataNode& generation )
{GUCEF_TRACE;

    generation.Clear();
    generation.SetName( "generation" );
    generation.SetNodeType( GUCEF_DATATYPE_OBJECT );
    generation.SetAttribute( "format", "gucef.packGeneration" );
    generation.SetAttribute( "version", (UInt32) 1 );
    generation.SetAttribute( "packId", pack.packId );
    generation.SetAttribute( "state", CORE::CString( AssetPackStateToString( pack.state ) ) );
    generation.SetAttribute( "stateReason", pack.stateReason );
    CORE::CString::StringVector formats( pack.formatsConverted.begin(), pack.formatsConverted.end() );
    generation.SetValuesOfChildByName( "formatsConverted", formats );
    CORE::CDataNodeSerializableSettings settings;
    pack.conversion.Serialize( *generation.AddChild( "conversion", GUCEF_DATATYPE_OBJECT ), settings );
    CORE::CDataNode* list = generation.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
    for ( size_t a=0; a<assets.size(); ++a )
    {
        if ( assets[ a ].convertedPath.IsNULLOrEmpty() )
            continue;
        CORE::CDataNode* entry = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "assetId", assets[ a ].assetId );
        entry->SetAttribute( "convertedPath", assets[ a ].convertedPath );
        entry->SetAttribute( "convertedFormat", assets[ a ].convertedFormat );
    }
    if ( GUCEF_NULL != extra )
        generation.AddChild( *extra )->SetName( "made" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetPackGenerations::ApplyGeneration( const CORE::CDataNode& generation, CAssetPackRecord& pack, TAssetRecordVector& assets )
{GUCEF_TRACE;

    if ( "gucef.packGeneration" != generation.GetAttributeValueOrChildValueByName( "format" ).AsString( CORE::CString::Empty, true ) )
        return false;
    pack.state = AssetPackStateFromString( generation.GetAttributeValueOrChildValueByName( "state" ).AsString( CORE::CString( AssetPackStateToString( pack.state ) ), true ) );
    pack.stateReason = generation.GetAttributeValueOrChildValueByName( "stateReason" ).AsString( CORE::CString::Empty, true );
    CORE::CString::StringVector formats;
    generation.GetValuesOfChildByName( "formatsConverted", formats );
    pack.formatsConverted.clear();
    pack.formatsConverted.insert( formats.begin(), formats.end() );
    const CORE::CDataNode* conversion = generation.FindChild( "conversion" );
    if ( GUCEF_NULL != conversion )
    {
        CORE::CDataNodeSerializableSettings settings;
        pack.conversion = CAssetConversionRecord();
        pack.conversion.Deserialize( *conversion, settings );
    }
    GUCEF::map< CORE::CString, const CORE::CDataNode* > converted;
    const CORE::CDataNode* list = generation.FindChild( "assets" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        for ( ; i != list->ConstEnd(); ++i )
            converted[ (*i)->GetAttributeValueOrChildValueByName( "assetId" ).AsString( CORE::CString::Empty, true ) ] = (*i);
    }
    for ( size_t a=0; a<assets.size(); ++a )
    {
        GUCEF::map< CORE::CString, const CORE::CDataNode* >::const_iterator i = converted.find( assets[ a ].assetId );
        assets[ a ].convertedPath = i == converted.end() ? CORE::CString::Empty : (*i).second->GetAttributeValueOrChildValueByName( "convertedPath" ).AsString( CORE::CString::Empty, true );
        assets[ a ].convertedFormat = i == converted.end() ? CORE::CString::Empty : (*i).second->GetAttributeValueOrChildValueByName( "convertedFormat" ).AsString( CORE::CString::Empty, true );
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
