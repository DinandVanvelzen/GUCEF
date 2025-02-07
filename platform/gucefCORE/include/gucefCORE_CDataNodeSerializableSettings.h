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

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

class CDataNode;

/*-------------------------------------------------------------------------*/

/**
 *  Class to generically convey settings to a data node based serializer
 */
class GUCEF_CORE_PUBLIC_CPP CDataNodeSerializableSettings : public CIConfigurable ,
                                                            public CICloneable
{
    public:

    static const CString ClassTypeName;

    enum EDataNodeSerializableLod : UInt32
    {
        DataNodeSerializableLod_Undefined = 0,
        
        DataNodeSerializableLod_KeyOnly = 1,
        DataNodeSerializableLod_MinimumDetails = 2,
        DataNodeSerializableLod_AverageDetails = GUCEF_INT32MAX,
        DataNodeSerializableLod_MaximumDetails = GUCEF_UINT32MAX,
    };

    CDataNodeSerializableSettings( void );

    CDataNodeSerializableSettings( const CDataNodeSerializableSettings& src );

    virtual ~CDataNodeSerializableSettings();

    CDataNodeSerializableSettings& operator=( const CDataNodeSerializableSettings& src );

    virtual bool SaveConfig( CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetUserData( void* userData );

    void* GetUserData( void ) const;

    UInt32 levelOfDetail;
    void* m_userData;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */
