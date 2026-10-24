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

#ifndef GUCEF_DRN_CDRNDATAGRIDSETTINGS_H
#define GUCEF_DRN_CDRNDATAGRIDSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#include "gucefDRN_CDRNVariantCodec.h"
#define GUCEF_DRN_CDRNVARIANTCODEC_H
#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */

#ifndef GUCEF_DRN_CDRNVOTEPOLICY_H
#include "gucefDRN_CDRNVotePolicy.h"
#define GUCEF_DRN_CDRNVOTEPOLICY_H
#endif /* GUCEF_DRN_CDRNVOTEPOLICY_H ? */

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

/** Who may write the values of a grid */
enum TDRNAuthority
{
    DRN_AUTHORITY_HOST          = 0,    /**< only the host / sequencer writes, the writes of others are proposals */
    DRN_AUTHORITY_OWNER         = 1,    /**< only the member that created the grid writes */
    DRN_AUTHORITY_PROPOSAL_VOTE = 2     /**< writes by others than the sequencer are put to a vote */
};

/** How a replicated item tells the application about received changes */
enum TDRNNotifyMode
{
    DRN_NOTIFY_PER_CHANGE   = 0,    /**< an event per changed key / per arrival */
    DRN_NOTIFY_PER_PACKET   = 1,    /**< one event per tick with the changed keys */
    DRN_NOTIFY_POLL         = 2     /**< no events, the application asks */
};

/*-------------------------------------------------------------------------*/

/**
 *  Settings of a CDRNDataGrid, section "DRNDataGridSettings" of a config. A game normally fills them in code.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDataGridSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    typedef GUCEF::vector< CORE::CVariant > TVariantVector;

    /** The quantisation of one key */
    class GUCEF_DRN_EXPORT_CPP CKeyQuantisation
    {
        public:
        CORE::CVariant key;
        CDRNQuantisation quantisation;
    };
    typedef GUCEF::vector< CKeyQuantisation > TKeyQuantisationVector;

    CORE::CString name;
    TDRNAuthority authority;
    bool reliable;                          /**< deltas on lane 0 instead of lane 3 */
    TVariantVector reliableKeys;            /**< keys whose changes always go reliable */
    UInt32 sendRateHz;                      /**< deltas per second, 0 = every tick */
    UInt32 maxBytesPerTick;                 /**< budget of the deltas of one tick to one subscriber, 0 = no limit */
    bool deltaAgainstAck;                   /**< deltas hold every change since the acknowledged state */
    UInt32 capacity;                        /**< keys preallocated */
    CDRNQuantisation defaultQuantisation;   /**< for keys without a quantisation of their own */
    TKeyQuantisationVector quantisation;
    TDRNNotifyMode notify;
    Float32 priority;                       /**< share of a short budget, relative to the other items */
    bool snapshotOnSubscribe;
    CDRNVotePolicy votePolicy;

    CDRNDataGridSettings( void );
    CDRNDataGridSettings( const CDRNDataGridSettings& src );
    virtual ~CDRNDataGridSettings();
    CDRNDataGridSettings& operator=( const CDRNDataGridSettings& src );

    /** The quantisation that applies to a key */
    const CDRNQuantisation& GetQuantisation( const CORE::CVariant& key ) const;

    void SetQuantisation( const CORE::CVariant& key, const CDRNQuantisation& quantisation );

    bool IsReliableKey( const CORE::CVariant& key ) const;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static CORE::CString AuthorityToString( TDRNAuthority authority );
    static TDRNAuthority StringToAuthority( const CORE::CString& authority, TDRNAuthority defaultAuthority );
    static CORE::CString NotifyModeToString( TDRNNotifyMode mode );
    static TDRNNotifyMode StringToNotifyMode( const CORE::CString& mode, TDRNNotifyMode defaultMode );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNDATAGRIDSETTINGS_H ? */
