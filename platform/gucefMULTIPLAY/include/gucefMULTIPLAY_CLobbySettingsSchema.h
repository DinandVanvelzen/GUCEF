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

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

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
 *  One game setting of a lobby: what it is, what values it takes and who may change it
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbySettingDef
{
    public:

    enum EType
    {
        TYPE_BOOL   = 0 ,
        TYPE_INT    = 1 ,
        TYPE_FLOAT  = 2 ,
        TYPE_ENUM   = 3 ,
        TYPE_STRING = 4
    };
    typedef enum EType TType;

    enum EChangePolicy
    {
        CHANGEPOLICY_BOSS  = 0 ,     /**< only the boss changes it */
        CHANGEPOLICY_VOTE  = 1 ,     /**< the boss changes it, a proposal of anyone else becomes a vote */
        CHANGEPOLICY_ANY   = 2 ,     /**< every member changes it */
        CHANGEPOLICY_OWNER = 3       /**< it belongs to a member: only that member changes it */
    };
    typedef enum EChangePolicy TChangePolicy;

    CORE::CString key;
    CORE::CString label;
    TType type;
    CORE::CString defaultValue;
    Float64 minValue;
    Float64 maxValue;
    Float64 step;                               /**< 0 for any value between min and max */
    bool hasRange;
    CORE::CString::StringVector options;        /**< the values of an ENUM */
    TChangePolicy changePolicy;
    CORE::CString uiHint;
    CORE::CString group;

    CLobbySettingDef( void );

    /** Checks a value against the type, range, step and options */
    bool IsValidValue( const CORE::CString& value, CORE::CString& reason ) const;

    void Save( CORE::CDataNode& node ) const;

    void Load( const CORE::CDataNode& node );

    static const char* TypeToString( TType type );
    static TType StringToType( const CORE::CString& type );
    static const char* ChangePolicyToString( TChangePolicy policy );
    static TChangePolicy StringToChangePolicy( const CORE::CString& policy );
};

/*-------------------------------------------------------------------------*/

/**
 *  The settings a game offers in its lobbies. The game gives it, it travels with the lobby so any client can
 *  show the settings without knowing the game.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLobbySettingsSchema : public CORE::CIConfigurable
{
    public:

    typedef GUCEF::vector< CLobbySettingDef > TSettingDefVector;

    TSettingDefVector settings;
    CORE::CString::StringVector factions;       /**< what a slot can pick, empty for anything */
    CORE::CString::StringVector colours;        /**< what a slot can pick, empty for anything */
    bool uniqueColours;                         /**< no two slots with the same colour */

    CLobbySettingsSchema( void );

    CLobbySettingsSchema( const CLobbySettingsSchema& src );

    virtual ~CLobbySettingsSchema();

    CLobbySettingsSchema& operator=( const CLobbySettingsSchema& src );

    const CLobbySettingDef* FindSetting( const CORE::CString& key ) const;

    void AddSetting( const CLobbySettingDef& def );

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

#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */
