/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H
#define GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

class CConfigStore;

/*-------------------------------------------------------------------------*/

/**
 *  Scope in which the globally configurable objects that get created are optional: when the config cannot be loaded for one
 *  of them that is logged as a warning, but it does not fail the load of the global config.
 *  Simply create an object of this class to make the scope of a block fault tolerant, the way CScopeMutex locks one:
 *
 *      {
 *          CFaultTolerantConfigScope faultTolerantScope;
 *          <do stuff with an optional item, for example start the globals of a plugin that may fail>
 *      }
 *
 *  The scope ends when the object is destroyed: at the end of the block, on a return from it and when an exception unwinds it.
 *  - The configurables that register while the scope is open, on the thread that opened it, are the optional ones. The
 *    configurables of other threads are not affected
 *  - Scopes nest, the innermost one counts: a scope created with faultTolerant set to false inside a fault tolerant one is a
 *    scope of configurables that are not optional. The outer scope is back in effect when the inner one ends
 *  - Objects of this class must end in the reverse order of their creation, which block scoping gives for free
 *  The plugin control uses this around the loading of a plugin that may fail ("LoadFailAllowed"): the globals that such a plugin
 *  starts register their configurables along with it, and an optional plugin must never be the reason that an application
 *  cannot start.
 */
class GUCEF_CORE_PUBLIC_CPP CFaultTolerantConfigScope
{
    public:

    /**
     *  Opens the scope in the config store of the application
     *  @param faultTolerant    true: the configurables that register in the scope are optional, false: they are not optional
     *                          (also when an outer scope has them optional)
     */
    explicit CFaultTolerantConfigScope( bool faultTolerant = true );

    /**
     *  Opens the scope in the given config store
     */
    explicit CFaultTolerantConfigScope( CConfigStore& configStore ,
                                        bool faultTolerant = true );

    /**
     *  Closes the scope
     */
    ~CFaultTolerantConfigScope();

    private:

    CFaultTolerantConfigScope( const CFaultTolerantConfigScope& src );              /* Copying doesnt make sense */
    CFaultTolerantConfigScope& operator=( const CFaultTolerantConfigScope& src );   /* Copying doesnt make sense */

    CConfigStore* m_configStore;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CFAULTTOLERANTCONFIGSCOPE_H ? */
