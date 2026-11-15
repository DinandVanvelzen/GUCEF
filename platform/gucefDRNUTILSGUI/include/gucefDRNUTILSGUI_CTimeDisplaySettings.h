/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMEZONERULES_H
#include "gucefCORE_CTimeZoneRules.h"
#define GUCEF_CORE_CTIMEZONERULES_H
#endif /* GUCEF_CORE_CTIMEZONERULES_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_DRNUTILSGUI_MACROS_H
#include "gucefDRNUTILSGUI_macros.h"
#define GUCEF_DRNUTILSGUI_MACROS_H
#endif /* GUCEF_DRNUTILSGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the forms show times. A preference of the GUI application (never of the service): every protocol, table and
 *  request carries UTC; only display is translated. LoadConfig() / SaveConfig() use "displayTimeZone",
 *  "showZoneSuffix" and "dateTimeFormat" as attributes of the node TimeDisplay.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CTimeDisplaySettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ClassTypeName;

    static const char* LocalZoneName;   /**< "local" */
    static const char* UtcZoneName;     /**< "utc" */
    static const char* ConfigSectionName;   /**< "TimeDisplay": the node of LoadConfig() / SaveConfig() */

    CTimeDisplaySettings( void );

    virtual ~CTimeDisplaySettings() GUCEF_VIRTUAL_OVERRIDE;

    /** "local" (the zone of the operating system the GUI runs on), "utc", or a zone name known to CORE::CTimeZoneRules */
    void SetDisplayTimeZone( const CORE::CString& displayTimeZone );
    const CORE::CString& GetDisplayTimeZone( void ) const;

    /** Every displayed time carries its zone ("14:05 CET"), so a screenshot is unambiguous */
    void SetShowZoneSuffix( bool showZoneSuffix );
    bool GetShowZoneSuffix( void ) const;

    /** strftime style pattern of displayed date times */
    void SetDateTimeFormat( const CORE::CString& dateTimeFormat );
    const CORE::CString& GetDateTimeFormat( void ) const;

    /**
     *  The rules the named zones are looked up in. Not owned: the application loads them (CORE::CTimeZoneRules ships no zone
     *  data) and keeps them alive while settings that point at them are in use. "local" and "utc" need none
     */
    void SetTimeZoneRules( const CORE::CTimeZoneRules* rules );
    const CORE::CTimeZoneRules* GetTimeZoneRules( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString m_displayTimeZone;
    bool m_showZoneSuffix;
    CORE::CString m_dateTimeFormat;
    const CORE::CTimeZoneRules* m_rules;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H ? */
