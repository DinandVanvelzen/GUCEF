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

#ifndef GUCEF_DRN_CDRNDATASTREAMSETTINGS_H
#define GUCEF_DRN_CDRNDATASTREAMSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#include "gucefDRN_CDRNDataGridSettings.h"
#define GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#endif /* GUCEF_DRN_CDRNDATAGRIDSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Settings of a CDRNDataStream, section "DRNDataStreamSettings" of a config
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataStreamSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    CORE::CString name;
    UInt32 backlogLimit;        /**< entries a receiver may have unread before the sender is held back */
    UInt32 retainedHistory;     /**< the last entries the publisher keeps for late subscribers */
    Float32 priority;
    TDRNNotifyMode notify;      /**< PER_CHANGE: an event per entry, PER_PACKET: one per tick, POLL: none */

    CDRNDataStreamSettings( void );
    CDRNDataStreamSettings( const CDRNDataStreamSettings& src );
    virtual ~CDRNDataStreamSettings();
    CDRNDataStreamSettings& operator=( const CDRNDataStreamSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATASTREAMSETTINGS_H ? */
