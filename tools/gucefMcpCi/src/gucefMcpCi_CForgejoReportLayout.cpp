/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CForgejoReportLayout.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CForgejoReportLayout::DefaultReportsOwner = "ci-bot";
const char* CForgejoReportLayout::PackageName = "ci-reports";
const char* CForgejoReportLayout::PackageType = "generic";
const char* CForgejoReportLayout::ReportFileName = "report.json";
const char* CForgejoReportLayout::CompressedLogSuffix = ".log.gz";
const char* CForgejoReportLayout::LogSuffix = ".log";
const char* CForgejoReportLayout::ImagesFileName = "images.json";
const char* CForgejoReportLayout::ImagesVersionSuffix = "images";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::MakeImagesVersion( const CORE::CString& runId )
{GUCEF_TRACE;

    return runId + "-" + ImagesVersionSuffix;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::MakeVersion( const CORE::CString& runId, const CORE::CString& jobId )
{GUCEF_TRACE;

    return runId + "-" + jobId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::VersionUrl( const CORE::CString& baseUrl, const CORE::CString& owner, const CORE::CString& version )
{GUCEF_TRACE;

    return baseUrl + "/api/packages/" + CCiUtils::PercentEncode( owner ) + "/" + PackageType + "/" + PackageName + "/" + CCiUtils::PercentEncode( version );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::FileUrl( const CORE::CString& baseUrl, const CORE::CString& owner, const CORE::CString& version, const CORE::CString& fileName )
{GUCEF_TRACE;

    return VersionUrl( baseUrl, owner, version ) + "/" + CCiUtils::PercentEncode( fileName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::ListUrl( const CORE::CString& baseUrl, const CORE::CString& owner, UInt32 page, UInt32 limit )
{GUCEF_TRACE;

    return baseUrl + "/api/v1/packages/" + CCiUtils::PercentEncode( owner ) + "?type=" + PackageType + "&q=" + PackageName +
           "&page=" + CORE::UInt32ToString( page ) + "&limit=" + CORE::UInt32ToString( limit );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::DefaultLogFileName( const CORE::CString& name )
{GUCEF_TRACE;

    return name + CompressedLogSuffix;
}

/*-------------------------------------------------------------------------*/

bool
CForgejoReportLayout::IsCompressed( const CORE::CString& fileName )
{GUCEF_TRACE;

    return fileName.Lowercase().EndsWith( ".gz" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CForgejoReportLayout::PlainLogFileName( const CORE::CString& fileName )
{GUCEF_TRACE;

    if ( IsCompressed( fileName ) )
        return fileName.CutChars( 3, false );
    return fileName;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
