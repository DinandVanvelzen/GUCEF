/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H
#define GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CLOBBYSTATE_H
#include "gucefMULTIPLAY_CLobbyState.h"
#define GUCEF_MULTIPLAY_CLOBBYSTATE_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSTATE_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a lobby is made with
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbyCreateParams : public CORE::CIConfigurable
{
    public:

    CORE::CString name;
    CORE::CString password;                 /**< empty for none */
    UInt32 maxPlayers;
    bool allowSpectators;
    bool isPublic;                          /**< listed on the internet, otherwise on the LAN only */
    CORE::CString gameId;
    CORE::CString gameVersion;
    CLobbySettingsSchema schema;
    CORE::CValueList initialSettings;       /**< on top of the defaults of the schema */
    CLobbyState::TSlotVector slots;
    TGameTopology topology;
    DRN::CDRNSecurityPolicy securityPolicy;     /**< the game's policy: its nodes, session and tokens; a lobby server refuses one weaker than its minimum */

    CLobbyCreateParams( void );

    CLobbyCreateParams( const CLobbyCreateParams& src );

    virtual ~CLobbyCreateParams();

    CLobbyCreateParams& operator=( const CLobbyCreateParams& src );

    /** The settings the lobby starts with: the defaults of the schema with the initial settings over them */
    void GetStartSettings( CORE::CValueList& settings ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLOBBYCREATEPARAMS_H ? */
