/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#include "gucefWEB_CStdParamParser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStdParams::CStdParams( void )
    : viewType( VIEW_TYPE_UNDEFINED )
    , levelOfDetail( CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_Undefined )
    , viewTypeWasProvided( false )
    , levelOfDetailWasProvided( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdParams::CStdParams( const CORE::CDataNodeSerializableSettings& settings )
    : viewType( VIEW_TYPE_UNDEFINED )
    , levelOfDetail( CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_Undefined )
    , viewTypeWasProvided( false )
    , levelOfDetailWasProvided( false )
{GUCEF_TRACE;

    levelOfDetail = settings.levelOfDetail;
    if ( levelOfDetail >= CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MaximumDetails )
        viewType = VIEW_TYPE_MAXIMUM_OBJECT_DETAIL;
    else
    if ( levelOfDetail >= CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_AverageDetails )
        viewType = VIEW_TYPE_AVERAGE_OBJECT_DETAIL;
    else
    if ( levelOfDetail >= CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails )
        viewType = VIEW_TYPE_MINIMAL_OBJECT_DETAIL;
    else
    if ( levelOfDetail >= CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly )
        viewType = VIEW_TYPE_INDEX_ONLY;
    else
        viewType = VIEW_TYPE_UNDEFINED;
}

/*-------------------------------------------------------------------------*/

CStdParams::CStdParams( const CStdParams& src )
    : viewType( src.viewType )
    , levelOfDetail( src.levelOfDetail )
    , viewTypeWasProvided( src.viewTypeWasProvided )
    , levelOfDetailWasProvided( src.levelOfDetailWasProvided )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdParams::~CStdParams()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CStdParamParser::ParseStdParams( const CString& params, CStdParams& outStdParams, bool& foundParams )
{GUCEF_TRACE;

    foundParams = false;
    
    CORE::CValueList paramOptions;
    paramOptions.SetMultiple( params, '&', '=' );

    CORE::CVariant param;
    if ( paramOptions.TryGetValue( "view", param ) )
    {
        // A view type is presented
        // We will look for the supported keywords. If found we initial sync view type and lod
        // lod can later be overwritten to provide more granularity via a numerical representation
        CORE::CString viewTypeStr = param.AsString();
        if ( "index" == viewTypeStr )
        {
            outStdParams.viewType = VIEW_TYPE_INDEX_ONLY;
            outStdParams.viewTypeWasProvided = true;
            outStdParams.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_KeyOnly;            
            foundParams = true;            
        }
        else
        if ( "objmin" == viewTypeStr )
        {
            outStdParams.viewType = VIEW_TYPE_MINIMAL_OBJECT_DETAIL;
            outStdParams.viewTypeWasProvided = true;
            outStdParams.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MinimumDetails;
            foundParams = true;
        }
        else
        if ( "objavg" == viewTypeStr )
        {
            outStdParams.viewType = VIEW_TYPE_AVERAGE_OBJECT_DETAIL;
            outStdParams.viewTypeWasProvided = true;
            outStdParams.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_AverageDetails;
            foundParams = true;
        }
        else
        if ( "objmax" == viewTypeStr )
        {
            outStdParams.viewType = VIEW_TYPE_MAXIMUM_OBJECT_DETAIL;
            outStdParams.viewTypeWasProvided = true;
            outStdParams.levelOfDetail = CORE::CDataNodeSerializableSettings::DataNodeSerializableLod_MaximumDetails;
            foundParams = true;
        }
    }

    if ( paramOptions.TryGetValue( "lod", param ) )
    {
        outStdParams.levelOfDetail = param.AsUInt32( outStdParams.levelOfDetail );
        outStdParams.levelOfDetailWasProvided = true;
        foundParams = true;
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

CStdParamParser::CStdParamParser( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdParamParser::CStdParamParser( const CStdParamParser& src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdParamParser::~CStdParamParser()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace WEB */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
