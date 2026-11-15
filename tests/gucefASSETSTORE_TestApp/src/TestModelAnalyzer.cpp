/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#include "TestModelAnalyzer.h"

using namespace GUCEF;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestModelAnalyzer::CTestModelAnalyzer( void )
    : ASSETSTORE::CIAssetContentAnalyzer()
    , m_failNext( 0 )
    , m_analysed( 0 )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CTestModelAnalyzer::~CTestModelAnalyzer()
{
}

/*-------------------------------------------------------------------------*/

void
CTestModelAnalyzer::FailNext( UInt32 count )
{
    MT::CScopeMutex lock( m_lock );
    m_failNext = count;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestModelAnalyzer::GetAnalysedCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_analysed;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestModelAnalyzer::GetName( void ) const
{
    return "testmodel";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestModelAnalyzer::GetVersion( void ) const
{
    return "1";
}

/*-------------------------------------------------------------------------*/

bool
CTestModelAnalyzer::CanAnalyze( const ASSETSTORE::CAssetRecord& asset ) const
{
    return ASSETSTORE::CAssetNameHints::IsModelKind( asset.kind ) || asset.kind == "animation";
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CTestModelAnalyzer::Analyze( const CORE::CString& vfsPath, ASSETSTORE::CAssetRecord& asset )
{
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_failNext > 0 )
        {
            --m_failNext;
            return ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
        }
    }
    CORE::CDynamicBuffer content;
    if ( !ReadVfsFile( vfsPath, content ) )
        return ASSETSTORE::ASSETSTORE_STATUS_IO_ERROR;
    CORE::CString text;
    text.Set( content.AsConstTypePtr< char >(), content.GetDataSize() );
    CORE::CString::StringMap values;
    const CORE::CString::StringVector pairs = text.ParseElements( ';', false );
    for ( size_t p=0; p<pairs.size(); ++p )
        values[ pairs[ p ].SubstrToChar( '=', true ).Trim( true ).Trim( false ) ] = pairs[ p ].SubstrToChar( '=', false, true ).Trim( true ).Trim( false );
    if ( values.empty() )
        return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED;

    if ( asset.kind == "animation" )
    {
        asset.animation.isSet = true;
        asset.animation.lengthSeconds = CORE::StringToDouble( values[ "length" ] );
        asset.animation.frameRate = CORE::StringToDouble( values[ "fps" ] );
    }
    else
    {
        asset.model.isSet = true;
        asset.model.triangles = CORE::StringToInt64( values[ "triangles" ] );
        asset.model.vertices = CORE::StringToInt64( values[ "vertices" ] );
        asset.model.bones = CORE::StringToInt64( values[ "bones" ] );
        asset.model.animations = CORE::StringToInt64( values[ "animations" ] );
        asset.model.rigged = values[ "rigged" ] == "1";
        asset.model.animated = values[ "animated" ] == "1";
        if ( asset.model.rigged )
            asset.kind = "skeletal-model";
    }
    MT::CScopeMutex lock( m_lock );
    ++m_analysed;
    return ASSETSTORE::ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
