/*
 *  AssetWorkbenchLib: the general settings page of the workbench: the shared general options and the options of the presentation of packs and the automatic thumbnails
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H
#define ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGSFORM_H
#include "gucefGUI_CGeneralSettingsForm.h"
#define GUCEF_GUI_CGENERALSETTINGSFORM_H
#endif /* GUCEF_GUI_CGENERALSETTINGSFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The page General of the workbench: what the shared general page has (the log, the developer console) and a group of the workbench:
 *  whether a pack that is opened shows its presentation, whether automatic thumbnails are made, and the action to clear them.
 *
 *  Widgets (by name) besides those of the shared page: OpenPackPresentation, PreferCookedOutput, AutomaticThumbnails, ClearAutomaticThumbnails.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchGeneralSettingsForm : public GUI::CGeneralSettingsForm
{
    public:

    /** Raised (no data) when Clear automatic thumbnails was pressed */
    static const CORE::CEvent ClearAutomaticThumbnailsRequestedEvent;
    static void RegisterEvents( void );

    CWorkbenchGeneralSettingsForm( void );
    virtual ~CWorkbenchGeneralSettingsForm() GUCEF_VIRTUAL_OVERRIDE;

    /** The preferences the group shows and changes at Apply (not owned) */
    void SetPreferences( CWorkbenchPreferences* preferences );

    /** Whether the last Apply changed the switch of the presentation (the value in the preferences is then the new one); reading it resets it */
    bool TakePresentationChanged( void );
    /** Whether "Open packs in the resource view" was changed by the last apply; reading it clears it */
    bool TakeResourceViewStartChanged( void );
    bool TakeThumbnailsChanged( void );

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchGeneralSettingsForm > TEventCallback;
    void OnClearActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CWorkbenchGeneralSettingsForm( const CWorkbenchGeneralSettingsForm& src );
    CWorkbenchGeneralSettingsForm& operator=( const CWorkbenchGeneralSettingsForm& src );

    private:

    CWorkbenchPreferences* m_preferences;
    bool m_presentationChanged;
    bool m_resourceViewStartChanged;
    bool m_thumbnailsChanged;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHGENERALSETTINGSFORM_H ? */
