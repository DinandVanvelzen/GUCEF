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

#ifndef GUCEF_GUI_CACTIVITYINDICATORFORM_H
#include "gucefGUI_CActivityIndicatorForm.h"
#define GUCEF_GUI_CACTIVITYINDICATORFORM_H
#endif /* GUCEF_GUI_CACTIVITYINDICATORFORM_H ? */

#ifndef GUCEF_GUI_CACTIVITYINFO_H
#include "gucefGUI_CActivityInfo.h"
#define GUCEF_GUI_CACTIVITYINFO_H
#endif /* GUCEF_GUI_CACTIVITYINFO_H ? */

#ifndef GUCEF_GUI_CACTIVITYSCOPE_H
#include "gucefGUI_CActivityScope.h"
#define GUCEF_GUI_CACTIVITYSCOPE_H
#endif /* GUCEF_GUI_CACTIVITYSCOPE_H ? */

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

#ifndef GUCEF_GUI_CPATHFIELDFORM_H
#include "gucefGUI_CPathFieldForm.h"
#define GUCEF_GUI_CPATHFIELDFORM_H
#endif /* GUCEF_GUI_CPATHFIELDFORM_H ? */

#ifndef GUCEF_GUI_CRECENTLOCATIONS_H
#include "gucefGUI_CRecentLocations.h"
#define GUCEF_GUI_CRECENTLOCATIONS_H
#endif /* GUCEF_GUI_CRECENTLOCATIONS_H ? */

#ifndef GUCEF_GUI_COPENACTIONSTRIPFORM_H
#include "gucefGUI_COpenActionStripForm.h"
#define GUCEF_GUI_COPENACTIONSTRIPFORM_H
#endif /* GUCEF_GUI_COPENACTIONSTRIPFORM_H ? */

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

#ifndef GUCEF_GUI_CKEYBOARDLAYOUT_H
#include "gucefGUI_CKeyboardLayout.h"
#define GUCEF_GUI_CKEYBOARDLAYOUT_H
#endif /* GUCEF_GUI_CKEYBOARDLAYOUT_H ? */

#ifndef GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H
#include "gucefGUI_CKeyboardLayoutRegistry.h"
#define GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H
#endif /* GUCEF_GUI_CKEYBOARDLAYOUTREGISTRY_H ? */

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
#ifndef GUCEF_GUI_CINPUTSETTINGSFORM_H
#include "gucefGUI_CInputSettingsForm.h"
#define GUCEF_GUI_CINPUTSETTINGSFORM_H
#endif /* GUCEF_GUI_CINPUTSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CAUDIOSETTINGSFORM_H
#include "gucefGUI_CAudioSettingsForm.h"
#define GUCEF_GUI_CAUDIOSETTINGSFORM_H
#endif /* GUCEF_GUI_CAUDIOSETTINGSFORM_H ? */

#ifndef GUCEF_GUI_CONSCREENKEYBOARDFORM_H
#include "gucefGUI_COnScreenKeyboardForm.h"
#define GUCEF_GUI_CONSCREENKEYBOARDFORM_H
#endif /* GUCEF_GUI_CONSCREENKEYBOARDFORM_H ? */

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#include "gucefGUI_CDeveloperConsoleForm.h"
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */

#ifndef GUCEF_GUI_CITHUMBNAILPROVIDER_H
#include "gucefGUI_CIThumbnailProvider.h"
#define GUCEF_GUI_CITHUMBNAILPROVIDER_H
#endif /* GUCEF_GUI_CITHUMBNAILPROVIDER_H ? */

#ifndef GUCEF_GUI_CICONTENTTYPERESOLVER_H
#include "gucefGUI_CIContentTypeResolver.h"
#define GUCEF_GUI_CICONTENTTYPERESOLVER_H
#endif /* GUCEF_GUI_CICONTENTTYPERESOLVER_H ? */

#ifndef GUCEF_GUI_CCONTENTBROWSERFORM_H
#include "gucefGUI_CContentBrowserForm.h"
#define GUCEF_GUI_CCONTENTBROWSERFORM_H
#endif /* GUCEF_GUI_CCONTENTBROWSERFORM_H ? */

#ifndef GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#include "gucefGUI_CPropertyRowDescriptor.h"
#define GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H
#endif /* GUCEF_GUI_CPROPERTYROWDESCRIPTOR_H ? */

#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#include "gucefGUI_CPropertyInspectorForm.h"
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */

#ifndef GUCEF_GUI_CIMAGEVIEWFORM_H
#include "gucefGUI_CImageViewForm.h"
#define GUCEF_GUI_CIMAGEVIEWFORM_H
#endif /* GUCEF_GUI_CIMAGEVIEWFORM_H ? */

#ifndef GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#include "gucefGUI_CIViewportController.h"
#define GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#endif /* GUCEF_GUI_CIVIEWPORTCONTROLLER_H ? */

#ifndef GUCEF_GUI_CVIEWPORTFORM_H
#include "gucefGUI_CViewportForm.h"
#define GUCEF_GUI_CVIEWPORTFORM_H
#endif /* GUCEF_GUI_CVIEWPORTFORM_H ? */

#ifndef GUCEF_GUI_CWEBVIEWFORM_H
#include "gucefGUI_CWebViewForm.h"
#define GUCEF_GUI_CWEBVIEWFORM_H
#endif /* GUCEF_GUI_CWEBVIEWFORM_H ? */

#ifndef GUCEF_GUI_CWEBVIEWFORMPRESENTER_H
#include "gucefGUI_CWebViewFormPresenter.h"
#define GUCEF_GUI_CWEBVIEWFORMPRESENTER_H
#endif /* GUCEF_GUI_CWEBVIEWFORMPRESENTER_H ? */

#ifndef GUCEF_GUI_CTIMELINEFORM_H
#include "gucefGUI_CTimelineForm.h"
#define GUCEF_GUI_CTIMELINEFORM_H
#endif /* GUCEF_GUI_CTIMELINEFORM_H ? */

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#include "gucefGUI_CITransferQueueModel.h"
#define GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#endif /* GUCEF_GUI_CITRANSFERQUEUEMODEL_H ? */

#ifndef GUCEF_GUI_CTRANSFERQUEUEFORM_H
#include "gucefGUI_CTransferQueueForm.h"
#define GUCEF_GUI_CTRANSFERQUEUEFORM_H
#endif /* GUCEF_GUI_CTRANSFERQUEUEFORM_H ? */

#ifndef GUCEF_GUI_CSEARCHQUERY_H
#include "gucefGUI_CSearchQuery.h"
#define GUCEF_GUI_CSEARCHQUERY_H
#endif /* GUCEF_GUI_CSEARCHQUERY_H ? */

#ifndef GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#include "gucefGUI_CSearchFilterDescription.h"
#define GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#endif /* GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H ? */

#ifndef GUCEF_GUI_CSEARCHRESULTPAGE_H
#include "gucefGUI_CSearchResultPage.h"
#define GUCEF_GUI_CSEARCHRESULTPAGE_H
#endif /* GUCEF_GUI_CSEARCHRESULTPAGE_H ? */

#ifndef GUCEF_GUI_CSAVEDSEARCH_H
#include "gucefGUI_CSavedSearch.h"
#define GUCEF_GUI_CSAVEDSEARCH_H
#endif /* GUCEF_GUI_CSAVEDSEARCH_H ? */

#ifndef GUCEF_GUI_CISEARCHMODEL_H
#include "gucefGUI_CISearchModel.h"
#define GUCEF_GUI_CISEARCHMODEL_H
#endif /* GUCEF_GUI_CISEARCHMODEL_H ? */

#ifndef GUCEF_GUI_CASSETSEARCHFORM_H
#include "gucefGUI_CAssetSearchForm.h"
#define GUCEF_GUI_CASSETSEARCHFORM_H
#endif /* GUCEF_GUI_CASSETSEARCHFORM_H ? */

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

#ifndef GUCEF_GUI_CGRIDPREFERENCES_H
#include "gucefGUI_CGridPreferences.h"
#define GUCEF_GUI_CGRIDPREFERENCES_H
#endif /* GUCEF_GUI_CGRIDPREFERENCES_H ? */

#ifndef GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#include "gucefGUI_CIGridPreferenceStore.h"
#define GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#endif /* GUCEF_GUI_CIGRIDPREFERENCESTORE_H ? */

#ifndef GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H
#include "gucefGUI_CLocalGridPreferenceStore.h"
#define GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H
#endif /* GUCEF_GUI_CLOCALGRIDPREFERENCESTORE_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H
#include "gucefGUI_CGridColumnChooserModel.h"
#define GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H
#endif /* GUCEF_GUI_CGRIDCOLUMNCHOOSERMODEL_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H
#include "gucefGUI_CGridColumnChooserForm.h"
#define GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H
#endif /* GUCEF_GUI_CGRIDCOLUMNCHOOSERFORM_H ? */

#ifndef GUCEF_GUI_CTOOLTIPSERVICE_H
#include "gucefGUI_CTooltipService.h"
#define GUCEF_GUI_CTOOLTIPSERVICE_H
#endif /* GUCEF_GUI_CTOOLTIPSERVICE_H ? */
#ifndef GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H
#include "gucefGUI_CITooltipTextProvider.h"
#define GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H
#endif /* GUCEF_GUI_CITOOLTIPTEXTPROVIDER_H ? */

#ifndef GUCEF_GUI_CTOOLTIPLAYOUT_H
#include "gucefGUI_CTooltipLayout.h"
#define GUCEF_GUI_CTOOLTIPLAYOUT_H
#endif /* GUCEF_GUI_CTOOLTIPLAYOUT_H ? */

#ifndef GUCEF_GUI_CTOOLTIPPAINTER_H
#include "gucefGUI_CTooltipPainter.h"
#define GUCEF_GUI_CTOOLTIPPAINTER_H
#endif /* GUCEF_GUI_CTOOLTIPPAINTER_H ? */

#ifndef GUCEF_GUI_CIGRIDHELPPROVIDER_H
#include "gucefGUI_CIGridHelpProvider.h"
#define GUCEF_GUI_CIGRIDHELPPROVIDER_H
#endif /* GUCEF_GUI_CIGRIDHELPPROVIDER_H ? */

#ifndef GUCEF_GUI_CHELPDOCUMENT_H
#include "gucefGUI_CHelpDocument.h"
#define GUCEF_GUI_CHELPDOCUMENT_H
#endif /* GUCEF_GUI_CHELPDOCUMENT_H ? */

#ifndef GUCEF_GUI_CHELPMARKUPPARSER_H
#include "gucefGUI_CHelpMarkupParser.h"
#define GUCEF_GUI_CHELPMARKUPPARSER_H
#endif /* GUCEF_GUI_CHELPMARKUPPARSER_H ? */

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#include "gucefGUI_CIHelpSource.h"
#define GUCEF_GUI_CIHELPSOURCE_H
#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */

#ifndef GUCEF_GUI_CHELPCATALOGUE_H
#include "gucefGUI_CHelpCatalogue.h"
#define GUCEF_GUI_CHELPCATALOGUE_H
#endif /* GUCEF_GUI_CHELPCATALOGUE_H ? */

#ifndef GUCEF_GUI_CHELPHISTORY_H
#include "gucefGUI_CHelpHistory.h"
#define GUCEF_GUI_CHELPHISTORY_H
#endif /* GUCEF_GUI_CHELPHISTORY_H ? */

#ifndef GUCEF_GUI_CHELPVIEWERFORM_H
#include "gucefGUI_CHelpViewerForm.h"
#define GUCEF_GUI_CHELPVIEWERFORM_H
#endif /* GUCEF_GUI_CHELPVIEWERFORM_H ? */

#ifndef GUCEF_GUI_CGRIDEXPORTSOURCE_H
#include "gucefGUI_CGridExportSource.h"
#define GUCEF_GUI_CGRIDEXPORTSOURCE_H
#endif /* GUCEF_GUI_CGRIDEXPORTSOURCE_H ? */

#ifndef GUCEF_GUI_CGRIDCSVOPTIONS_H
#include "gucefGUI_CGridCsvOptions.h"
#define GUCEF_GUI_CGRIDCSVOPTIONS_H
#endif /* GUCEF_GUI_CGRIDCSVOPTIONS_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#include "gucefGUI_CGridCsvExportReport.h"
#define GUCEF_GUI_CGRIDCSVEXPORTREPORT_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTREPORT_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTER_H
#include "gucefGUI_CGridCsvExporter.h"
#define GUCEF_GUI_CGRIDCSVEXPORTER_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTER_H ? */

#ifndef GUCEF_GUI_CIGRIDEXPORTTARGET_H
#include "gucefGUI_CIGridExportTarget.h"
#define GUCEF_GUI_CIGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CIGRIDEXPORTTARGET_H ? */

#ifndef GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H
#include "gucefGUI_CNativeGridExportTarget.h"
#define GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H ? */

#ifndef GUCEF_GUI_CVFSGRIDEXPORTTARGET_H
#include "gucefGUI_CVfsGridExportTarget.h"
#define GUCEF_GUI_CVFSGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CVFSGRIDEXPORTTARGET_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H
#include "gucefGUI_CGridCsvExportController.h"
#define GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNFIT_H
#include "gucefGUI_CGridColumnFit.h"
#define GUCEF_GUI_CGRIDCOLUMNFIT_H
#endif /* GUCEF_GUI_CGRIDCOLUMNFIT_H ? */

#ifndef GUCEF_GUI_CGRIDCOLUMNFITTER_H
#include "gucefGUI_CGridColumnFitter.h"
#define GUCEF_GUI_CGRIDCOLUMNFITTER_H
#endif /* GUCEF_GUI_CGRIDCOLUMNFITTER_H ? */

#ifndef GUCEF_GUI_CCELLSTATUSCOLOURS_H
#include "gucefGUI_CCellStatusColours.h"
#define GUCEF_GUI_CCELLSTATUSCOLOURS_H
#endif /* GUCEF_GUI_CCELLSTATUSCOLOURS_H ? */

#ifndef GUCEF_GUI_CCONTEXTMENUITEM_H
#include "gucefGUI_CContextMenuItem.h"
#define GUCEF_GUI_CCONTEXTMENUITEM_H
#endif /* GUCEF_GUI_CCONTEXTMENUITEM_H ? */

#ifndef GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H
#include "gucefGUI_CIGridContextMenuProvider.h"
#define GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H
#endif /* GUCEF_GUI_CIGRIDCONTEXTMENUPROVIDER_H ? */

#ifndef GUCEF_GUI_CTABCONTROLPAINTER_H
#include "gucefGUI_CTabControlPainter.h"
#define GUCEF_GUI_CTABCONTROLPAINTER_H
#endif /* GUCEF_GUI_CTABCONTROLPAINTER_H ? */

#ifndef GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#include "gucefGUI_CKeyboardShortcutMap.h"
#define GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H
#endif /* GUCEF_GUI_CKEYBOARDSHORTCUTMAP_H ? */

#ifndef GUCEF_GUI_CTABAREANAVIGATOR_H
#include "gucefGUI_CTabAreaNavigator.h"
#define GUCEF_GUI_CTABAREANAVIGATOR_H
#endif /* GUCEF_GUI_CTABAREANAVIGATOR_H ? */

#ifndef GUCEF_GUI_CNAVIGATIONHISTORY_H
#include "gucefGUI_CNavigationHistory.h"
#define GUCEF_GUI_CNAVIGATIONHISTORY_H
#endif /* GUCEF_GUI_CNAVIGATIONHISTORY_H ? */

#ifndef GUCEF_GUI_CBREADCRUMBBAR_H
#include "gucefGUI_CBreadcrumbBar.h"
#define GUCEF_GUI_CBREADCRUMBBAR_H
#endif /* GUCEF_GUI_CBREADCRUMBBAR_H ? */

#ifndef GUCEF_GUI_CSLIDER_H
#include "gucefGUI_CSlider.h"
#define GUCEF_GUI_CSLIDER_H
#endif /* GUCEF_GUI_CSLIDER_H ? */

#ifndef GUCEF_GUI_CSPLITTER_H
#include "gucefGUI_CSplitter.h"
#define GUCEF_GUI_CSPLITTER_H
#endif /* GUCEF_GUI_CSPLITTER_H ? */

#ifndef GUCEF_GUI_CFOLDABLEPANEL_H
#include "gucefGUI_CFoldablePanel.h"
#define GUCEF_GUI_CFOLDABLEPANEL_H
#endif /* GUCEF_GUI_CFOLDABLEPANEL_H ? */

#ifndef GUCEF_GUI_CLAYOUTSTATE_H
#include "gucefGUI_CLayoutState.h"
#define GUCEF_GUI_CLAYOUTSTATE_H
#endif /* GUCEF_GUI_CLAYOUTSTATE_H ? */

#ifndef GUCEF_GUI_CLAYOUTMANAGER_H
#include "gucefGUI_CLayoutManager.h"
#define GUCEF_GUI_CLAYOUTMANAGER_H
#endif /* GUCEF_GUI_CLAYOUTMANAGER_H ? */

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

#ifndef GUCEF_GUI_CWEBVIEWIMAGEBINDING_H
#include "gucefGUI_CWebViewImageBinding.h"
#define GUCEF_GUI_CWEBVIEWIMAGEBINDING_H
#endif /* GUCEF_GUI_CWEBVIEWIMAGEBINDING_H ? */

#ifndef GUCEF_GUI_CWEBPAGEVIEW_H
#include "gucefGUI_CWebPageView.h"
#define GUCEF_GUI_CWEBPAGEVIEW_H
#endif /* GUCEF_GUI_CWEBPAGEVIEW_H ? */

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

#ifndef GUCEF_GUI_ETEXTOVERFLOW_H
#include "gucefGUI_ETextOverflow.h"
#define GUCEF_GUI_ETEXTOVERFLOW_H
#endif /* GUCEF_GUI_ETEXTOVERFLOW_H ? */

#ifndef GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H
#include "gucefGUI_CIDisplayTextTransform.h"
#define GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H
#endif /* GUCEF_GUI_CIDISPLAYTEXTTRANSFORM_H ? */

#ifndef GUCEF_GUI_CPSEUDOLOCALIZER_H
#include "gucefGUI_CPseudoLocalizer.h"
#define GUCEF_GUI_CPSEUDOLOCALIZER_H
#endif /* GUCEF_GUI_CPSEUDOLOCALIZER_H ? */

#ifndef GUCEF_GUI_ELAYOUTLINTSEVERITY_H
#include "gucefGUI_ELayoutLintSeverity.h"
#define GUCEF_GUI_ELAYOUTLINTSEVERITY_H
#endif /* GUCEF_GUI_ELAYOUTLINTSEVERITY_H ? */

#ifndef GUCEF_GUI_ELAYOUTLINTRULE_H
#include "gucefGUI_ELayoutLintRule.h"
#define GUCEF_GUI_ELAYOUTLINTRULE_H
#endif /* GUCEF_GUI_ELAYOUTLINTRULE_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTFINDING_H
#include "gucefGUI_CLayoutLintFinding.h"
#define GUCEF_GUI_CLAYOUTLINTFINDING_H
#endif /* GUCEF_GUI_CLAYOUTLINTFINDING_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTOPTIONS_H
#include "gucefGUI_CLayoutLintOptions.h"
#define GUCEF_GUI_CLAYOUTLINTOPTIONS_H
#endif /* GUCEF_GUI_CLAYOUTLINTOPTIONS_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#include "gucefGUI_CLayoutLint.h"
#define GUCEF_GUI_CLAYOUTLINT_H
#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTMATRIX_H
#include "gucefGUI_CLayoutLintMatrix.h"
#define GUCEF_GUI_CLAYOUTLINTMATRIX_H
#endif /* GUCEF_GUI_CLAYOUTLINTMATRIX_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTSWEEP_H
#include "gucefGUI_CLayoutLintSweep.h"
#define GUCEF_GUI_CLAYOUTLINTSWEEP_H
#endif /* GUCEF_GUI_CLAYOUTLINTSWEEP_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINTBUDGET_H
#include "gucefGUI_CLayoutLintBudget.h"
#define GUCEF_GUI_CLAYOUTLINTBUDGET_H
#endif /* GUCEF_GUI_CLAYOUTLINTBUDGET_H ? */

#ifndef GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H
#include "gucefGUI_CLayoutDebugOverlay.h"
#define GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H
#endif /* GUCEF_GUI_CLAYOUTDEBUGOVERLAY_H ? */

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
