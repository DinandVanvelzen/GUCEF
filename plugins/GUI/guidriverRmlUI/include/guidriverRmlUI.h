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
 *      This is the main include header for the guidriverRmlUI library:
 *      the gucefGUI backend that draws with RmlUi, shared by the RmlUi
 *      renderer drivers (guidriverRmlUIOpenGL ...).
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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H
#include "guidriverRmlUI_CRmlUIDriver.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#include "guidriverRmlUI_CRmlUIContext.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUICONTEXT_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#include "guidriverRmlUI_CRmlUIWidgetElement.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETELEMENT_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H
#include "guidriverRmlUI_CRmlUIWidgetHandler.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIWIDGETHANDLER_H ? */

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H
#include "guidriverRmlUI_CRmlUIFormBackend.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIFORMBACKEND_H ? */

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

