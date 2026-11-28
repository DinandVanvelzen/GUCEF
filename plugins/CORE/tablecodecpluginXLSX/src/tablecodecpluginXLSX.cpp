/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H
#include "tablecodecpluginXLSX_CXlsxTableCodec.h"
#define TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXTABLECODEC_H ? */

#include "tablecodecpluginXLSX.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return (GUCEF::CORE::UInt32) GUCEF_TABLECODECPLUGIN_API_VERSION;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Init( const GUCEF::CORE::CValueList* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "tablecodecpluginXLSX: initialized" );
    return 0;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_GetCodecCount( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return 1;
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::CITableCodec* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_CreateCodec( GUCEF::CORE::UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( 0 != index )
        return GUCEF_NULL;
    return GUCEF_NEW GUCEF::TABLECODECPLUGIN::XLSX::CXlsxTableCodec();
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Shutdown( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "tablecodecpluginXLSX: shut down" );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Description( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Table codec plugin for Microsoft Excel (.xlsx) workbooks: reads one worksheet as a table, read only";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Copyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
TABLECODECPLUGIN_Version( GUCEF::CORE::TVersion* version ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != version )
    {
        version->major = 1;
        version->minor = 0;
        version->patch = 0;
        version->release = 0;
    }
}

/*-------------------------------------------------------------------------*/
