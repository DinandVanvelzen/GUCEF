/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

#include "gucefDRN_CDRNTablePublisherSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNTablePublisherSettings::ClassTypeName = "GUCEF::DRN::CDRNTablePublisherSettings";

/*-------------------------------------------------------------------------*/

CDRNTablePublisherSettings::CDRNTablePublisherSettings( void )
    : CORE::CIConfigurable()
    , maxValueBytes( 8192 )
    , checkRows( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNTablePublisherSettings::CDRNTablePublisherSettings( const CDRNTablePublisherSettings& src )
    : CORE::CIConfigurable( src )
    , maxValueBytes( src.maxValueBytes )
    , checkRows( src.checkRows )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNTablePublisherSettings::~CDRNTablePublisherSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNTablePublisherSettings&
CDRNTablePublisherSettings::operator=( const CDRNTablePublisherSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        maxValueBytes = src.maxValueBytes;
        checkRows = src.checkRows;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisherSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "maxValueBytes", maxValueBytes ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "checkRows", checkRows ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTablePublisherSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    maxValueBytes = cfg.GetAttributeValueOrChildValueByName( "maxValueBytes" ).AsUInt32( maxValueBytes, true );
    checkRows = cfg.GetAttributeValueOrChildValueByName( "checkRows" ).AsBool( checkRows, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNTablePublisherSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
