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

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H
#include "gucefDRNUTILSGUI_CTimeDisplaySettings.h"
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H
#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H ? */

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
 *  Turns UTC milliseconds into display text in the chosen zone (CORE::CTimeZoneRules::UtcToLocal() for a named zone,
 *  CORE::CDateTime::ToSystemLocal() for "local") and operator input in the display zone back into UTC. Applied through
 *  the column hints of timestamp columns at display time: cell values, sorting and filters stay UTC.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CTimeDisplayFormatter
{
    public:

    CTimeDisplayFormatter( void );

    explicit CTimeDisplayFormatter( const CTimeDisplaySettings& settings );

    void SetSettings( const CTimeDisplaySettings& settings );

    const CTimeDisplaySettings& GetSettings( void ) const;

    /**
     *  "2026-11-15 14:05:00 UTC+01:00" by the date time format of the settings (strftime style: %Y %m %d %H %M %S, %f for the
     *  milliseconds, %% ); the zone suffix is left out when the settings say so. Empty for 0 (no time). A named zone that the
     *  rules do not know is shown as UTC, with the suffix saying so
     */
    CORE::CString FormatUtcMs( Int64 utcMs ) const;

    /**
     *  The zone suffix at the instant: "UTC", or "UTC+01:00" with the offset the zone has at that instant (daylight saving
     *  included). The offset form is used because neither the operating system conversion nor CORE::CTimeZoneRules gives the
     *  abbreviation of a zone ("CET")
     */
    CORE::CString GetZoneSuffix( Int64 utcMs ) const;

    /** Whether the display zone of the settings can be converted to: "utc" and "local" always, a named zone when the rules have it */
    bool IsDisplayZoneKnown( void ) const;

    /** The zone as the status line names it: "UTC", "local (UTC+01:00)" or "Europe/Amsterdam (UTC+01:00)" at the instant */
    CORE::CString GetZoneDescription( Int64 utcMs ) const;

    /** The UTC text of the instant, for tooltips: "2026-11-15T13:05:00Z" */
    CORE::CString FormatUtcTooltip( Int64 utcMs ) const;

    /** A time the operator typed in the display zone, to UTC; refuses nonexistent and ambiguous local times with a message */
    bool ParseDisplayTime( const CORE::CString& text, Int64& utcMs, CORE::CString& error ) const;

    private:

    CORE::CString FormatPattern( const CORE::CDateTime& shown ) const;

    /** The instant as a date and time in the display zone, with the offset of the zone at the instant; false when the zone is not known (shown is then UTC) */
    bool ToDisplayDateTime( Int64 utcMs, CORE::CDateTime& shown ) const;

    bool WallClockToUtcMs( Int64 wallSeconds, Int32 milliseconds, Int64& utcMs, CORE::CString& error ) const;

    CTimeDisplaySettings m_settings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYFORMATTER_H ? */
