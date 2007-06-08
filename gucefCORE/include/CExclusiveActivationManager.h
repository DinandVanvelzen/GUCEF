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
#include "gucefCORE_macros.h"
#include "gucefCORE_ETypes.h"

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_CPP
    #pragma warning( push )
#endif

#pragma warning( disable: 4251 ) // 'classname' needs to have dll-interface to be used by clients of class 'classname'
#pragma warning( disable: 4284 ) // return type for operator -> is 'const *' (ie; not a UDT or reference to a UDT).
#pragma warning( disable: 4786 ) // identifier was truncated to 'number' characters

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

class GUCEFCORE_EXPORT_CPP CExclusiveActivationManager
{
    public:
    
    static CExclusiveActivationManager* Instance( void );
    
    bool SetActivateObj( const std::string& objTypeName ,
                         const std::string& objName     );
    
    bool DisableActivateObj( const std::string& objTypeName );

    CExclusiveActiveObj* GetActiveObject( const std::string& objTypeName ) const;
    
    bool SetDefaultObjForType( const std::string& objTypeName ,
                               const std::string& objName     );

    bool GetDefaultObjForType( const std::string& objTypeName ,
                               std::string& objName           );
    
    UInt32 GetRegisteredTypeCount( void ) const;    

    private:
    friend class CExclusiveActiveObj;
    
    void RegisterObj( CExclusiveActiveObj* newObj );
    
    void UnregisterObj( CExclusiveActiveObj* theObj );
    
    private:
    friend class CXBGeneralModule;
    
    static void Deinstance( void );

    private:

    CExclusiveActivationManager( void );
    CExclusiveActivationManager( const CExclusiveActivationManager& src );
    ~CExclusiveActivationManager();
    CExclusiveActivationManager& operator=( const CExclusiveActivationManager& src );
    
    private:
    typedef std::map< std::string, CExclusiveActiveObj* >   TTypedObjList;
    struct SExclusiveObjList
    {
        TTypedObjList objList;
        std::string defaultObj;
        CExclusiveActiveObj* activeObj;
    };
    typedef struct SExclusiveObjList                        TExclusiveObjList;
    typedef std::map< std::string, TExclusiveObjList >      TObjTypeList;
    
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

#ifndef GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_CPP
    #pragma warning( pop )
#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H  ? */
