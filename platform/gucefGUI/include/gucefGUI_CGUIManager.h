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

#ifndef GUCEF_GUI_CGUIMANAGER_H
#define GUCEF_GUI_CGUIMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_GUI_CFORMFACTORY_H
#include "gucefGUI_CFormFactory.h"
#define GUCEF_GUI_CFORMFACTORY_H
#endif /* GUCEF_GUI_CFORMFACTORY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CGUIManager : public CORE::CObservingNotifier
{
    public:
    
    static const CORE::CEvent DriverRegisteredEvent;
    static const CORE::CEvent DriverUnregisteredEvent;
    static const CORE::CEvent FormFactoryRegisteredEvent;
    static const CORE::CEvent FormFactoryUnregisteredEvent;
    
    typedef CORE::TCloneableString TKeyContainer;  /** event data type used for the keys */
    
    static void RegisterEvents( void );
    
    public:
    
    typedef CFormFactory::TFactory       TConcreteFormFactory;
    typedef CString::StringSet TStringSet;
    typedef TStringSet TFormTypeSet;
    typedef TStringSet TDriverNameSet;
        
    TGuiContextPtr CreateGUIContext( const CString& guiDriverName    ,
                                     TWindowContextPtr windowContext );
    
    void RegisterGenericFormFactory( const CString& formTypeName   ,
                                     TConcreteFormFactory* factory );
    
    void UnregisterGenericFormFactory( const CString& formTypeName );
    
    TFormTypeSet GetGenericFormTypes( void ) const;
    
    bool HasGenericFormType( const CString& formTypeName );
    
    // creates a generic form, this form will have to context
    // associated with it. It is recommended to create forms
    // via the gui context
    // the form goes away when the last reference to it is dropped
    CFormPtr CreateGenericForm( const CString& formTypeName );
    
    void RegisterGUIDriver( const CString& guiDriverName, CGUIDriver* guiDriver );
    
    void UnregisterGUIDriverByName( const CString& guiDriverName );
    
    CGUIDriver* GetGuiDriver( const CString& guiDriverName );
    
    TDriverNameSet GetGuiDriverList( void ) const;

    UInt32 GetGuiDriverCount( void ) const;
    
    private:
    friend class CGuiGlobal;    
    
    CGUIManager( void );
    
    virtual ~CGUIManager();
    
    private:
    
    typedef CORE::CTEventHandlerFunctor< CGUIManager > TEventCallback;
        
    CGUIManager( const CGUIManager& src );                 /**< not implemented, must be unique */
    CGUIManager& operator=( const CGUIManager& src );      /**< not implemented, must be unique */
    
    void OnFormFactoryRegistered( CORE::CNotifier* notifier           ,
                                  const CORE::CEvent& eventid         ,
                                  CORE::CICloneable* eventdata = NULL );

    void OnFormFactoryUnregistered( CORE::CNotifier* notifier           ,
                                    const CORE::CEvent& eventid         ,
                                    CORE::CICloneable* eventdata = NULL );
                                      
    private:
    
    typedef GUCEF::map< CString, CGUIDriver* > TGUIDriverMap;
    
    CFormFactory m_genericFormFactory;
    TGUIDriverMap m_guiDrivers;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CGUIMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/