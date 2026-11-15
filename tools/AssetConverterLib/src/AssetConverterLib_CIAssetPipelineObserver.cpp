/*
 *  AssetConverterLib: conversion of asset packs into an engine-neutral intermediate and its cooking
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

#include "AssetConverterLib_CIAssetPipelineObserver.h"

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

CIAssetPipelineObserver::CIAssetPipelineObserver( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetPipelineObserver::~CIAssetPipelineObserver()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIAssetPipelineObserver::OnPackIndexed( UInt32 /* assetCount */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIAssetPipelineObserver::OnPhase( TPipelinePhase /* phase */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CIAssetPipelineObserver::OnAssetReport( const CConversionReportEntry& /* entry */, UInt32 /* done */, UInt32 /* total */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
