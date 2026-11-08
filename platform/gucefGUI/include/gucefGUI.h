/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CSECRETINPUTSETTINGS_H
#include "gucefGUI_CSecretInputSettings.h"
#define GUCEF_GUI_CSECRETINPUTSETTINGS_H
#endif /* GUCEF_GUI_CSECRETINPUTSETTINGS_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

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

#ifndef GUCEF_GUI_CAUDIOSETTINGSFORM_H
#include "gucefGUI_CAudioSettingsForm.h"
#define GUCEF_GUI_CAUDIOSETTINGSFORM_H
#endif /* GUCEF_GUI_CAUDIOSETTINGSFORM_H ? */

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

#ifndef GUCEF_GUI_CGUIUSERACTIONLOG_H
#include "gucefGUI_CGUIUserActionLog.h"
#define GUCEF_GUI_CGUIUSERACTIONLOG_H
#endif /* GUCEF_GUI_CGUIUSERACTIONLOG_H ? */

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

#ifndef GUCEF_GUI_CTABLEVIEWRANGE_H
#include "gucefGUI_CTableViewRange.h"
#define GUCEF_GUI_CTABLEVIEWRANGE_H
#endif /* GUCEF_GUI_CTABLEVIEWRANGE_H ? */

#ifndef GUCEF_GUI_CTABLEVIEWCOLUMN_H
#include "gucefGUI_CTableViewColumn.h"
#define GUCEF_GUI_CTABLEVIEWCOLUMN_H
#endif /* GUCEF_GUI_CTABLEVIEWCOLUMN_H ? */

#ifndef GUCEF_GUI_CITABLEVIEWMODEL_H
#include "gucefGUI_CITableViewModel.h"
#define GUCEF_GUI_CITABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CITABLEVIEWMODEL_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#ifndef GUCEF_GUI_CDATATABLEVIEWMODEL_H
#include "gucefGUI_CDataTableViewModel.h"
#define GUCEF_GUI_CDATATABLEVIEWMODEL_H
#endif /* GUCEF_GUI_CDATATABLEVIEWMODEL_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNLAYOUT_H
#include "gucefGUI_CGridColumnLayout.h"
#define GUCEF_GUI_CGRIDCOLUMNLAYOUT_H
#endif /* GUCEF_GUI_CGRIDCOLUMNLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDSELECTION_H
#include "gucefGUI_CGridSelection.h"
#define GUCEF_GUI_CGRIDSELECTION_H
#endif /* GUCEF_GUI_CGRIDSELECTION_H ? */

#ifndef GUCEF_GUI_CGRIDVIEWPAINTER_H
#include "gucefGUI_CGridViewPainter.h"
#define GUCEF_GUI_CGRIDVIEWPAINTER_H
#endif /* GUCEF_GUI_CGRIDVIEWPAINTER_H ? */

#ifndef GUCEF_GUI_CTABLEEDITRESULT_H
#include "gucefGUI_CTableEditResult.h"
#define GUCEF_GUI_CTABLEEDITRESULT_H
#endif /* GUCEF_GUI_CTABLEEDITRESULT_H ? */

#ifndef GUCEF_GUI_CTABLEEDITHISTORY_H
#include "gucefGUI_CTableEditHistory.h"
#define GUCEF_GUI_CTABLEEDITHISTORY_H
#endif /* GUCEF_GUI_CTABLEEDITHISTORY_H ? */

#ifndef GUCEF_GUI_CTABLECELLPARSER_H
#include "gucefGUI_CTableCellParser.h"
#define GUCEF_GUI_CTABLECELLPARSER_H
#endif /* GUCEF_GUI_CTABLECELLPARSER_H ? */

#ifndef GUCEF_GUI_CTABULARTEXT_H
#include "gucefGUI_CTabularText.h"
#define GUCEF_GUI_CTABULARTEXT_H
#endif /* GUCEF_GUI_CTABULARTEXT_H ? */

#ifndef GUCEF_GUI_CGRIDEDITREPORT_H
#include "gucefGUI_CGridEditReport.h"
#define GUCEF_GUI_CGRIDEDITREPORT_H
#endif /* GUCEF_GUI_CGRIDEDITREPORT_H ? */

#ifndef GUCEF_GUI_CGRIDCELLEDITOR_H
#include "gucefGUI_CGridCellEditor.h"
#define GUCEF_GUI_CGRIDCELLEDITOR_H
#endif /* GUCEF_GUI_CGRIDCELLEDITOR_H ? */

#ifndef GUCEF_GUI_CGRIDEDITCONTROLLER_H
#include "gucefGUI_CGridEditController.h"
#define GUCEF_GUI_CGRIDEDITCONTROLLER_H
#endif /* GUCEF_GUI_CGRIDEDITCONTROLLER_H ? */

#ifndef GUCEF_GUI_CGRIDCLIPBOARD_H
#include "gucefGUI_CGridClipboard.h"
#define GUCEF_GUI_CGRIDCLIPBOARD_H
#endif /* GUCEF_GUI_CGRIDCLIPBOARD_H ? */

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

#ifndef GUCEF_GUI_CCHARTSERIES_H
#include "gucefGUI_CChartSeries.h"
#define GUCEF_GUI_CCHARTSERIES_H
#endif /* GUCEF_GUI_CCHARTSERIES_H ? */

#ifndef GUCEF_GUI_CCHARTAXIS_H
#include "gucefGUI_CChartAxis.h"
#define GUCEF_GUI_CCHARTAXIS_H
#endif /* GUCEF_GUI_CCHARTAXIS_H ? */

#ifndef GUCEF_GUI_CCHARTDECIMATOR_H
#include "gucefGUI_CChartDecimator.h"
#define GUCEF_GUI_CCHARTDECIMATOR_H
#endif /* GUCEF_GUI_CCHARTDECIMATOR_H ? */

#ifndef GUCEF_GUI_CCHARTMARKER_H
#include "gucefGUI_CChartMarker.h"
#define GUCEF_GUI_CCHARTMARKER_H
#endif /* GUCEF_GUI_CCHARTMARKER_H ? */

#ifndef GUCEF_GUI_CCHARTPALETTE_H
#include "gucefGUI_CChartPalette.h"
#define GUCEF_GUI_CCHARTPALETTE_H
#endif /* GUCEF_GUI_CCHARTPALETTE_H ? */

#ifndef GUCEF_GUI_CCHARTVIEW_H
#include "gucefGUI_CChartView.h"
#define GUCEF_GUI_CCHARTVIEW_H
#endif /* GUCEF_GUI_CCHARTVIEW_H ? */

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
