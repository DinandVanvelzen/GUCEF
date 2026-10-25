/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 *      This is the main include header for the gucefGUI library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with providing a unified and
 *      uniform GUI interface independant of the chosen GUI backend
 */

#ifndef GUCEF_GUI_H
#define GUCEF_GUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_GUI_CONFIG_H
#include "gucefGUI_config.h"
#define GUCEF_GUI_CONFIG_H
#endif /* GUCEF_GUI_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_GUI_ETYPES_H
#include "gucefGUI_ETypes.h"
#define GUCEF_GUI_ETYPES_H
#endif /* GUCEF_GUI_ETYPES_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CFORM_H
#include "gucefGUI_CForm.h"
#define GUCEF_GUI_CFORM_H
#endif /* GUCEF_GUI_CFORM_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CGUIMANAGER_H
#include "gucefGUI_CGUIManager.h"
#define GUCEF_GUI_CGUIMANAGER_H
#endif /* GUCEF_GUI_CGUIMANAGER_H ? */

#ifndef GUCEF_GUI_CWIDGETFACTORY_H
#include "gucefGUI_CWidgetFactory.h"
#define GUCEF_GUI_CWIDGETFACTORY_H
#endif /* GUCEF_GUI_CWIDGETFACTORY_H ? */

#ifndef GUCEF_GUI_CFORMFACTORY_H
#include "gucefGUI_CFormFactory.h"
#define GUCEF_GUI_CFORMFACTORY_H
#endif /* GUCEF_GUI_CFORMFACTORY_H ? */

#ifndef GUCEF_GUI_CUVECTOR_H
#include "gucefGUI_CUVector.h"
#define GUCEF_GUI_CUVECTOR_H
#endif /* GUCEF_GUI_CUVECTOR_H ? */

#ifndef GUCEF_GUI_CUDIM_H
#include "gucefGUI_CUDim.h"
#define GUCEF_GUI_CUDIM_H
#endif /* GUCEF_GUI_CUDIM_H ? */

#ifndef GUCEF_GUI_CBUTTON_H
#include "gucefGUI_CButton.h"
#define GUCEF_GUI_CBUTTON_H
#endif /* GUCEF_GUI_CBUTTON_H ? */

#ifndef GUCEF_GUI_CPUSHBUTTON_H
#include "gucefGUI_CPushButton.h"
#define GUCEF_GUI_CPUSHBUTTON_H
#endif /* GUCEF_GUI_CPUSHBUTTON_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CTEXTBOX_H
#include "gucefGUI_CTextbox.h"
#define GUCEF_GUI_CTEXTBOX_H
#endif /* GUCEF_GUI_CTEXTBOX_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CLABEL_H
#include "gucefGUI_CLabel.h"
#define GUCEF_GUI_CLABEL_H
#endif /* GUCEF_GUI_CLABEL_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

#ifndef GUCEF_GUI_CPATHFIELDFORM_H
#include "gucefGUI_CPathFieldForm.h"
#define GUCEF_GUI_CPATHFIELDFORM_H
#endif /* GUCEF_GUI_CPATHFIELDFORM_H ? */

#ifndef GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#include "gucefGUI_CFileSystemDialogForm.h"
#define GUCEF_GUI_CFILESYSTEMDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESYSTEMDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CFILEOPENDIALOGFORM_H
#include "gucefGUI_CFileOpenDialogForm.h"
#define GUCEF_GUI_CFILEOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CFILEOPENDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CFILESAVEDIALOGFORM_H
#include "gucefGUI_CFileSaveDialogForm.h"
#define GUCEF_GUI_CFILESAVEDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESAVEDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#include "gucefGUI_CDirectoryOpenDialogForm.h"
#define GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H
#endif /* GUCEF_GUI_CDIRECTORYOPENDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CDIRECTORYSAVEDIALOGFORM_H
#include "gucefGUI_CDirectorySaveDialogForm.h"
#define GUCEF_GUI_CDIRECTORYSAVEDIALOGFORM_H
#endif /* GUCEF_GUI_CDIRECTORYSAVEDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CTABCONTENTPANE_H
#include "gucefGUI_CTabContentPane.h"
#define GUCEF_GUI_CTABCONTENTPANE_H
#endif /* GUCEF_GUI_CTABCONTENTPANE_H ? */

#ifndef GUCEF_GUI_CTABCONTROL_H
#include "gucefGUI_CTabControl.h"
#define GUCEF_GUI_CTABCONTROL_H
#endif /* GUCEF_GUI_CTABCONTROL_H ? */

#ifndef GUCEF_GUI_CCHECKBOX_H
#include "gucefGUI_CCheckbox.h"
#define GUCEF_GUI_CCHECKBOX_H
#endif /* GUCEF_GUI_CCHECKBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CSPINNER_H
#include "gucefGUI_CSpinner.h"
#define GUCEF_GUI_CSPINNER_H
#endif /* GUCEF_GUI_CSPINNER_H ? */

#ifndef GUCEF_GUI_CRENDERCONTEXT_H
#include "gucefGUI_CRenderContext.h"
#define GUCEF_GUI_CRENDERCONTEXT_H
#endif /* GUCEF_GUI_CRENDERCONTEXT_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGER_H
#include "gucefGUI_CWindowManager.h"
#define GUCEF_GUI_CWINDOWMANAGER_H
#endif /* GUCEF_GUI_CWINDOWMANAGER_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUCEF_GUI_CVIDEOOPTIONS_H
#include "gucefGUI_CVideoOptions.h"
#define GUCEF_GUI_CVIDEOOPTIONS_H
#endif /* GUCEF_GUI_CVIDEOOPTIONS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_UTILS_H
#include "gucefGUI_Utils.h"
#define GUCEF_GUI_UTILS_H
#endif /* GUCEF_GUI_UTILS_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_EWIDGETPROPERTY_H
#include "gucefGUI_EWidgetProperty.h"
#define GUCEF_GUI_EWIDGETPROPERTY_H
#endif /* GUCEF_GUI_EWIDGETPROPERTY_H ? */

#ifndef GUCEF_GUI_CWIDGETPROPERTYVALUE_H
#include "gucefGUI_CWidgetPropertyValue.h"
#define GUCEF_GUI_CWIDGETPROPERTYVALUE_H
#endif /* GUCEF_GUI_CWIDGETPROPERTYVALUE_H ? */

#ifndef GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#include "gucefGUI_CWidgetTypeRegistry.h"
#define GUCEF_GUI_CWIDGETTYPEREGISTRY_H
#endif /* GUCEF_GUI_CWIDGETTYPEREGISTRY_H ? */

#ifndef GUCEF_GUI_CWIDGETHANDLER_H
#include "gucefGUI_CWidgetHandler.h"
#define GUCEF_GUI_CWIDGETHANDLER_H
#endif /* GUCEF_GUI_CWIDGETHANDLER_H ? */

#ifndef GUCEF_GUI_CBITMAPFONT_H
#include "gucefGUI_CBitmapFont.h"
#define GUCEF_GUI_CBITMAPFONT_H
#endif /* GUCEF_GUI_CBITMAPFONT_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZER_H
#include "gucefGUI_CFontRasterizer.h"
#define GUCEF_GUI_CFONTRASTERIZER_H
#endif /* GUCEF_GUI_CFONTRASTERIZER_H ? */

#ifndef GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#include "gucefGUI_CFontRasterizerRegistry.h"
#define GUCEF_GUI_CFONTRASTERIZERREGISTRY_H
#endif /* GUCEF_GUI_CFONTRASTERIZERREGISTRY_H ? */

#ifndef GUCEF_GUI_STANDARDTHEMES_H
#include "gucefGUI_StandardThemes.h"
#define GUCEF_GUI_STANDARDTHEMES_H
#endif /* GUCEF_GUI_STANDARDTHEMES_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGS_H
#include "gucefGUI_CGeneralSettings.h"
#define GUCEF_GUI_CGENERALSETTINGS_H
#endif /* GUCEF_GUI_CGENERALSETTINGS_H ? */

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

#ifndef GUCEF_GUI_CSETTINGSFORM_H
#include "gucefGUI_CSettingsForm.h"
#define GUCEF_GUI_CSETTINGSFORM_H
#endif /* GUCEF_GUI_CSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGSFORM_H
#include "gucefGUI_CVideoSettingsForm.h"
#define GUCEF_GUI_CVIDEOSETTINGSFORM_H
#endif /* GUCEF_GUI_CVIDEOSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CGENERALSETTINGSFORM_H
#include "gucefGUI_CGeneralSettingsForm.h"
#define GUCEF_GUI_CGENERALSETTINGSFORM_H
#endif /* GUCEF_GUI_CGENERALSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#include "gucefGUI_CDeveloperConsoleForm.h"
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGUIINPUTADAPTER_H
#include "gucefGUI_CGUIInputAdapter.h"
#define GUCEF_GUI_CGUIINPUTADAPTER_H
#endif /* GUCEF_GUI_CGUIINPUTADAPTER_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CWINDOW_H
#include "gucefGUI_CWindow.h"
#define GUCEF_GUI_CWINDOW_H
#endif /* GUCEF_GUI_CWINDOW_H ? */

#ifndef GUCEF_GUI_CPANEL_H
#include "gucefGUI_CPanel.h"
#define GUCEF_GUI_CPANEL_H
#endif /* GUCEF_GUI_CPANEL_H ? */

#ifndef GUCEF_GUI_CGROUPBOX_H
#include "gucefGUI_CGroupBox.h"
#define GUCEF_GUI_CGROUPBOX_H
#endif /* GUCEF_GUI_CGROUPBOX_H ? */

#ifndef GUCEF_GUI_CSCROLLPANEL_H
#include "gucefGUI_CScrollPanel.h"
#define GUCEF_GUI_CSCROLLPANEL_H
#endif /* GUCEF_GUI_CSCROLLPANEL_H ? */

#ifndef GUCEF_GUI_CIMAGEFRAME_H
#include "gucefGUI_CImageFrame.h"
#define GUCEF_GUI_CIMAGEFRAME_H
#endif /* GUCEF_GUI_CIMAGEFRAME_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CSLIDER_H
#include "gucefGUI_CSlider.h"
#define GUCEF_GUI_CSLIDER_H
#endif /* GUCEF_GUI_CSLIDER_H ? */

#ifndef GUCEF_GUI_CPROGRESSBAR_H
#include "gucefGUI_CProgressBar.h"
#define GUCEF_GUI_CPROGRESSBAR_H
#endif /* GUCEF_GUI_CPROGRESSBAR_H ? */

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CVIDEOVIEW_H
#include "gucefGUI_CVideoView.h"
#define GUCEF_GUI_CVIDEOVIEW_H
#endif /* GUCEF_GUI_CVIDEOVIEW_H ? */

#ifndef GUCEF_GUI_CAUDIOPLAYER_H
#include "gucefGUI_CAudioPlayer.h"
#define GUCEF_GUI_CAUDIOPLAYER_H
#endif /* GUCEF_GUI_CAUDIOPLAYER_H ? */

#ifndef GUCEF_GUI_CTREEVIEW_H
#include "gucefGUI_CTreeview.h"
#define GUCEF_GUI_CTREEVIEW_H
#endif /* GUCEF_GUI_CTREEVIEW_H ? */

#ifndef GUCEF_GUI_CMENUBAR_H
#include "gucefGUI_CMenuBar.h"
#define GUCEF_GUI_CMENUBAR_H
#endif /* GUCEF_GUI_CMENUBAR_H ? */

#ifndef GUCEF_GUI_CPOPUPMENU_H
#include "gucefGUI_CPopupMenu.h"
#define GUCEF_GUI_CPOPUPMENU_H
#endif /* GUCEF_GUI_CPOPUPMENU_H ? */

#ifndef GUCEF_GUI_STANDARDWIDGETTYPES_H
#include "gucefGUI_StandardWidgetTypes.h"
#define GUCEF_GUI_STANDARDWIDGETTYPES_H
#endif /* GUCEF_GUI_STANDARDWIDGETTYPES_H ? */

#ifndef GUCEF_GUI_CWIDGETPAINTER_H
#include "gucefGUI_CWidgetPainter.h"
#define GUCEF_GUI_CWIDGETPAINTER_H
#endif /* GUCEF_GUI_CWIDGETPAINTER_H ? */

#ifndef GUCEF_GUI_CSOFTWARERENDERER_H
#include "gucefGUI_CSoftwareRenderer.h"
#define GUCEF_GUI_CSOFTWARERENDERER_H
#endif /* GUCEF_GUI_CSOFTWARERENDERER_H ? */

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#include "gucefGUI_CGuiGlobal.h"
#define GUCEF_GUI_CGUIGLOBAL_H
#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GUI_PUBLIC_C const char* GUCEF_GUI_ModuleCompileTimeStamp( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-04-2007 :
       - Made the initial version of the library header.

---------------------------------------------------------------------------*/
