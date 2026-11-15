/*
 *  AssetStoreConverterHook_TestApp: Test application for the converter hook of the primary asset store (AssetStoreConverterHook)
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

#ifndef CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define CDYNAMICBUFFER_H
#endif /* CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CCOOKSETTINGS_H
#include "AssetConverterLib_CCookSettings.h"
#define ASSETCONVERTERLIB_CCOOKSETTINGS_H
#endif /* ASSETCONVERTERLIB_CCOOKSETTINGS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "TestPipelineCooker.h"

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace HOOKTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
StoreBytes( const CORE::CString& path, UInt32 size, char fill )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString folder = CVfsPaths::FolderOf( path );
    CORE::CString::StringVector missing;
    while ( !folder.IsNULLOrEmpty() && !vfs.DirExists( folder ) )
    {
        missing.push_back( folder );
        folder = CVfsPaths::FolderOf( folder );
    }
    for ( size_t m=missing.size(); m>0; --m )
        vfs.CreateDir( missing[ m-1 ] );
    CORE::CDynamicBuffer buffer;
    for ( UInt32 i=0; i<size; ++i )
        buffer.Append( &fill, 1, true );
    return vfs.StoreAsFile( path, buffer, 0, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelineTestCooker::CPipelineTestCooker( void )
    : CIAssetCooker()
{
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPipelineTestCooker::GetTargetName( void ) const
{
    return "testcook";
}

/*-------------------------------------------------------------------------*/

UInt32
CPipelineTestCooker::GetVersion( void ) const
{
    return 1;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineTestCooker::Cook( const CORE::CString& /* intermediateDir */, const CORE::CString& outDir, const CCookSettings& settings, CConversionReport& report )
{
    const CORE::CString base = CVfsPaths::Join( outDir, settings.packId );
    const CORE::CString shared = CVfsPaths::Join( base, "textures/shared.tex" );
    bool ok = StoreBytes( shared, 100, 's' );
    for ( size_t a=0; a<settings.assets.size(); ++a )
    {
        const CCookSettings::SAsset& asset = settings.assets[ a ];
        report.SelectAsset( asset.assetPath );
        const CORE::CString stem = CVfsPaths::StemOf( asset.assetPath );
        const CORE::CString mesh = CVfsPaths::Join( base, stem + ".mesh" );
        const CORE::CString material = CVfsPaths::Join( base, stem + ".material" );
        ok = StoreBytes( mesh, 40, 'm' ) && ok;
        ok = StoreBytes( material, 30, 'a' ) && ok;
        CORE::CString::StringVector references;
        references.push_back( material );
        report.AddOutputFile( mesh, references, settings.profile.name );
        references.clear();
        references.push_back( shared );
        report.AddOutputFile( material, references, settings.profile.name );
        report.AddOutputFile( shared, CORE::CString::StringVector(), settings.profile.name );
    }
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace HOOKTEST */

/*-------------------------------------------------------------------------*/
