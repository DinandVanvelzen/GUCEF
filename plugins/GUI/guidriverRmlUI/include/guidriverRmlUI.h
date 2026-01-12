/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
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
 *      This is the main include header for the guceCEGUIOgre library.
 *      It is an optional part of an engine called GUCE.
 *      All classes in this library have to do with providing a GUI backend
 *      based on CEGUI with Ogre doing the rendering.
 */

#ifndef GUCEF_guidriverRmlUI_H
#define GUCEF_guidriverRmlUI_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_guidriverRmlUI_CONFIG_H
#include "guidriverRmlUI_config.h"
#define GUCEF_guidriverRmlUI_CONFIG_H
#endif /* GUCEF_guidriverRmlUI_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_guidriverRmlUI_ETYPES_H
#include "guidriverRmlUI_ETypes.h"
#define GUCEF_guidriverRmlUI_ETYPES_H
#endif /* GUCEF_guidriverRmlUI_ETYPES_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H
#include "guidriverRmlUI_CFormBackendImp.h"
#define GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H
#endif /* GUCEF_guidriverRmlUI_CFORMBACKENDIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CWIDGETIMP_H
#include "guidriverRmlUI_CWidgetImp.h"
#define GUCEF_guidriverRmlUI_CWIDGETIMP_H
#endif /* GUCEF_guidriverRmlUI_CWIDGETIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CBUTTONIMP_H
#include "guidriverRmlUI_CButtonImp.h"
#define GUCEF_guidriverRmlUI_CBUTTONIMP_H
#endif /* GUCEF_guidriverRmlUI_CBUTTONIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CPUSHBUTTONIMP_H
#include "guidriverRmlUI_CPushButtonImp.h"
#define GUCEF_guidriverRmlUI_CPUSHBUTTONIMP_H
#endif /* GUCEF_guidriverRmlUI_CPUSHBUTTONIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CEDITBOXIMP_H
#include "guidriverRmlUI_CEditboxImp.h"
#define GUCEF_guidriverRmlUI_CEDITBOXIMP_H
#endif /* GUCEF_guidriverRmlUI_CEDITBOXIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CLISTBOXIMP_H
#include "guidriverRmlUI_CListBoxImp.h"
#define GUCEF_guidriverRmlUI_CLISTBOXIMP_H
#endif /* GUCEF_guidriverRmlUI_CLISTBOXIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CWINDOWIMP_H
#include "guidriverRmlUI_CWindowImp.h"
#define GUCEF_guidriverRmlUI_CWINDOWIMP_H
#endif /* GUCEF_guidriverRmlUI_CWINDOWIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CIMAGEFRAMEIMP_H
#include "guidriverRmlUI_CImageFrameImp.h"
#define GUCEF_guidriverRmlUI_CIMAGEFRAMEIMP_H
#endif /* GUCEF_guidriverRmlUI_CIMAGEFRAMEIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CHECKBOXIMP_H
#include "guidriverRmlUI_CCheckboxImp.h"
#define GUCEF_guidriverRmlUI_CHECKBOXIMP_H
#endif /* GUCEF_guidriverRmlUI_CHECKBOXIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CLABELIMP_H
#include "guidriverRmlUI_CLabelImp.h"
#define GUCEF_guidriverRmlUI_CLABELIMP_H
#endif /* GUCEF_guidriverRmlUI_CLABELIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CCOMBOBOXIMP_H
#include "guidriverRmlUI_CComboboxImp.h"
#define GUCEF_guidriverRmlUI_CCOMBOBOXIMP_H
#endif /* GUCEF_guidriverRmlUI_CCOMBOBOXIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CTABCONTROLIMP_H
#include "guidriverRmlUI_CTabControlImp.h"
#define GUCEF_guidriverRmlUI_CTABCONTROLIMP_H
#endif /* GUCEF_guidriverRmlUI_CTABCONTROLIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CTABCONTENTPANEIMP_H
#include "guidriverRmlUI_CTabContentPaneImp.h"
#define GUCEF_guidriverRmlUI_CTABCONTENTPANEIMP_H
#endif /* GUCEF_guidriverRmlUI_CTABCONTENTPANEIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CSPINNERIMP_H
#include "guidriverRmlUI_CSpinnerImp.h"
#define GUCEF_guidriverRmlUI_CSPINNERIMP_H
#endif /* GUCEF_guidriverRmlUI_CSPINNERIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CGRIDVIEWIMP_H
#include "guidriverRmlUI_CGridViewImp.h"
#define GUCEF_guidriverRmlUI_CGRIDVIEWIMP_H
#endif /* GUCEF_guidriverRmlUI_CGRIDVIEWIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CFILEOPENDIALOGIMP_H
#include "guidriverRmlUI_CFileOpenDialogImp.h"
#define GUCEF_guidriverRmlUI_CFILEOPENDIALOGIMP_H
#endif /* GUCEF_guidriverRmlUI_CFILEOPENDIALOGIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CFILESAVEDIALOGIMP_H
#include "guidriverRmlUI_CFileSaveDialogImp.h"
#define GUCEF_guidriverRmlUI_CFILESAVEDIALOGIMP_H
#endif /* GUCEF_guidriverRmlUI_CFILESAVEDIALOGIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CFILESYSTEMDIALOGIMP_H
#include "guidriverRmlUI_CFileSystemDialogImp.h"
#define GUCEF_guidriverRmlUI_CFILESYSTEMDIALOGIMP_H
#endif /* GUCEF_guidriverRmlUI_CFILESYSTEMDIALOGIMP_H ? */

#ifndef GUCEF_guidriverRmlUI_CRENDERCONTEXTIMP_H
#include "guidriverRmlUI_CRenderContextImp.h"
#define GUCEF_guidriverRmlUI_CRENDERCONTEXTIMP_H
#endif /* GUCEF_guidriverRmlUI_CRENDERCONTEXTIMP_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_guidriverRmlUI_PUBLIC_C const char* GUCEF_guidriverRmlUI_ModuleCompileTimeStamp( void );

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverRmlUI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 21-04-2007 :
       - Made the initial version of the library header.

---------------------------------------------------------------------------*/

