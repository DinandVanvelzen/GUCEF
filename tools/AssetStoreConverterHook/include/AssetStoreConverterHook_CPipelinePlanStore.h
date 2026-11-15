/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H
#define ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETSTORECONVERTERHOOK_MACROS_H
#include "AssetStoreConverterHook_macros.h"
#define ASSETSTORECONVERTERHOOK_MACROS_H
#endif /* ASSETSTORECONVERTERHOOK_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The last plan (convert_plan) of each pack, kept in memory for the resource assettools://packs/{packId}/plan. A
 *  bounded number of packs is kept; the plan of the pack that was planned longest ago goes first. Thread safe.
 */
class ASSETSTORECONVERTERHOOK_PUBLIC_CPP CPipelinePlanStore
{
    public:

    static const UInt32 MaxPacks = 64;

    CPipelinePlanStore( void );

    void Store( const CORE::CString& packId, const CORE::CDataNode& plan );

    bool Get( const CORE::CString& packId, CORE::CDataNode& plan ) const;

    private:

    CPipelinePlanStore( const CPipelinePlanStore& src );              /**< not implemented */
    CPipelinePlanStore& operator=( const CPipelinePlanStore& src );   /**< not implemented */

    typedef GUCEF::map< CORE::CString, CORE::CDataNode > TPlanMap;

    mutable MT::CMutex m_lock;
    TPlanMap m_plans;
    CORE::CString::StringVector m_order;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEPLANSTORE_H ? */
