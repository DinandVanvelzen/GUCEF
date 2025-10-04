/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H
#define GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <map>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CExclusiveActiveObj;

/*-------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CExclusiveActivationManager
{
    public:

    CExclusiveActivationManager( void );

    ~CExclusiveActivationManager();

    bool SetActivateObj( const CString& objTypeName ,
                         const CString& objName     );

    bool DisableActivateObj( const CString& objTypeName );

    CExclusiveActiveObj* GetActiveObject( const CString& objTypeName ) const;

    bool SetDefaultObjForType( const CString& objTypeName ,
                               const CString& objName     );

    bool GetDefaultObjForType( const CString& objTypeName ,
                               CString& objName           );

    UInt32 GetRegisteredTypeCount( void ) const;

    private:
    friend class CExclusiveActiveObj;

    void RegisterObj( CExclusiveActiveObj* newObj );

    void UnregisterObj( CExclusiveActiveObj* theObj );

    private:

    CExclusiveActivationManager( const CExclusiveActivationManager& src );
    CExclusiveActivationManager& operator=( const CExclusiveActivationManager& src );

    private:
    typedef GUCEF::map< CString, CExclusiveActiveObj* >   TTypedObjList;
    struct SExclusiveObjList
    {
        TTypedObjList objList;
        CString defaultObj;
        CExclusiveActiveObj* activeObj;
    };
    typedef struct SExclusiveObjList                    TExclusiveObjList;
    typedef GUCEF::map< CString, TExclusiveObjList >    TObjTypeList;

    TObjTypeList m_typeList;

    private:

    static CExclusiveActivationManager* m_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H  ? */
