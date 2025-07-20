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

#ifndef GUCEF_CORE_CINUMERICIDGENERATORBASE_H
#define GUCEF_CORE_CINUMERICIDGENERATORBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"           /* often used gucef macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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

class CINumericID;

/**
 *  Interface class that serves as an abstract callback facility
 *  for numeric ID objects. It is basically a workaround for a circular
 *  dependency between templates.
 */
class GUCEF_CORE_PUBLIC_CPP CINumericIDGeneratorBase : public MT::CILockable
{
    public:

    /**
     *  abstract facility for releasing template IDs
     */
    virtual void ReleaseID( CINumericID* idObj ) = 0;

    /**
     *  How many freed IDs are allowed to accumulate before
     *  we start taking from the pool of freed IDs instead of issuing new IDs
     *
     *  The purpose of this functionality is to provide you with a +- time period based on
     *  the consumption rate within which you wont see the same ID again. This is mainly usefull for
     *  IDs being used in human centric interfaces where fast recycle would likely confuse the humans due to race conditions
     */
    virtual void SetRecycleCheckThreshold( UInt32 recycleThreshold ) = 0;

    /**
     *  How many freed IDs are allowed to accumulate before
     *  we start taking from the pool of freed IDs instead of issuing new IDs
     *
     *  The purpose of this functionality is to provide you with a +- time period based on
     *  the consumption rate within which you wont see the same ID again. This is mainly usefull for
     *  IDs being used in human centric interfaces where fast recycle would likely confuse the humans due to race conditions
     */
    virtual UInt32 GetRecycleCheckThreshold( void ) const = 0;

    protected:

    CINumericIDGeneratorBase( void );
    CINumericIDGeneratorBase( const CINumericIDGeneratorBase& src );
    virtual ~CINumericIDGeneratorBase();
    CINumericIDGeneratorBase& operator=( const CINumericIDGeneratorBase& src );
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CINumericIDGeneratorBase, MT::CMutex > CINumericIDGeneratorBasePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CINUMERICIDGENERATORBASE_H ? */

