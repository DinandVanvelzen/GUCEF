/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CIGRIDPREFERENCESTORE_H
#define GUCEF_GUI_CIGRIDPREFERENCESTORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CGRIDPREFERENCES_H
#include "gucefGUI_CGridPreferences.h"
#define GUCEF_GUI_CGRIDPREFERENCES_H
#endif /* GUCEF_GUI_CGRIDPREFERENCES_H ? */

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

/**
 *  Keeps the column preferences of grids (CGridPreferences) by the id of the grid. The id is the application's: stable
 *  across runs and unique per grid with a column set of its own, for example "MyApp.PositionsPage.Positions"; a store
 *  treats it as opaque text and must not let it address anything but the record of that id.
 *
 *  The grid calls the store on its own thread (the GUI thread), synchronously: a record is small. A store whose
 *  storage is slow or remote keeps a cache and writes behind it, so that the grid never waits on it. One store can
 *  serve every grid of an application. Made with GUCEF_NEW and handed around as TGridPreferenceStorePtr
 *  ( ( GUCEF_NEW CLocalGridPreferenceStore( dir ) )->CreateSharedPtr() ).
 */
class GUCEF_GUI_PUBLIC_CPP CIGridPreferenceStore : public CORE::CTSharedPtrCreator< CIGridPreferenceStore, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CIGridPreferenceStore, MT::CMutex > TGridPreferenceStorePtr;

    CIGridPreferenceStore( void );

    virtual ~CIGridPreferenceStore();

    /** The record of the grid; false when there is none or it cannot be read (the grid then keeps its defaults) */
    virtual bool Load( const CString& gridId, CGridPreferences& preferences ) = 0;

    /** Keeps the record of the grid, replacing the one it had; false when it could not be kept */
    virtual bool Save( const CString& gridId, const CGridPreferences& preferences ) = 0;

    /** Forgets the record of the grid; true when there is none afterwards */
    virtual bool Remove( const CString& gridId ) = 0;

    private:

    CIGridPreferenceStore( const CIGridPreferenceStore& src );
    CIGridPreferenceStore& operator=( const CIGridPreferenceStore& src );
};

/*-------------------------------------------------------------------------*/

typedef CIGridPreferenceStore::TGridPreferenceStorePtr TGridPreferenceStorePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIGRIDPREFERENCESTORE_H ? */
