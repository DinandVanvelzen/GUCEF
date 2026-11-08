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

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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
 *  Base class for anything that wishes to be invoked as part of global config loading
 *  Allows for centralized config management without having to code explicit invocations from
 *  the central config management standpoint. Also makes it clear in the class hierarchy which
 *  classes are intended to partake in a globalized context
 */
class GUCEF_CORE_PUBLIC_CPP CGloballyConfigurable : public CIConfigurable
{
    public:

    CGloballyConfigurable( bool includeInBootstrapPhase = false );

    CGloballyConfigurable( const CGloballyConfigurable& src );

    virtual ~CGloballyConfigurable() GUCEF_VIRTUAL_OVERRIDE;

    CGloballyConfigurable& operator=( const CGloballyConfigurable& src );

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param cfg the data tree you wish to store the config
     *      @return wheter storing all the config information to the provided tree was successfull
     */
    virtual bool SaveConfig( CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param cfg node that is to act as root of the config data tree
     *      @return whether loading required/mandatory settings from the given config was successfull
     */
    virtual bool LoadConfig( const CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    bool IsIncludedInGlobalBootstrapConfigLoad( void ) const;

    static bool IsGlobalBootstrapConfigLoadInProgress( void );

    static bool IsGlobalConfigLoadInProgress( void );

    protected:

    /**
     *  Takes this object out of the global config store and waits for a load or a save of the config on another thread
     *  that calls it right now. A derived class that may be destroyed while another thread loads or saves the config
     *  calls it first in its own destructor: the destructor of this class does the same, but only once the derived
     *  part of the object is gone, which such a call may still use.
     */
    void UnregisterFromConfigStore( void );
    
    private:

    bool m_includeInBootstrapPhase;  /**< wheter this configurable object should also be invoked for the bootstrap config if any */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */
