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

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#define GUCEF_CORE_CICONFIGURABLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h" /* macros that are GUCEF specific and generic macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "ETypes.h"           /* simple types used */
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

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
 *      Abstract base class for adding a configuration API to decending classes.
 *      Defines a uniform interface for loading and saving configuration data.
 *      Optionally the global CConfigStore can be used to load/unload the config of
 *      all configurable classes that have the 'use global config' switch set.
 */
class GUCEF_CORE_PUBLIC_CPP CIConfigurable
{
        public:

        CIConfigurable( void );

        CIConfigurable( const CIConfigurable& src );

        CIConfigurable( bool useglobalconfig );

        virtual ~CIConfigurable();

        CIConfigurable& operator=( const CIConfigurable& src );

        /**
         *      Attempts to store the given tree in the file
         *      given according to the method of the codec metadata
         *
         *      @param tree the data tree you wish to store
         *      @return wheter storing the tree was successfull
         */
        virtual bool SaveConfig( CDataNode& tree ) = 0;

        /**
         *      Attempts to load data from the given file to the
         *      root node given. The root data will be replaced
         *      and any children the node may already have will be deleted.
         *
         *      @param treeroot pointer to the node that is to act as root of the data tree
         *      @return whether building the tree from the given file was successfull.
         */
        virtual bool LoadConfig( const CDataNode& treeroot ) = 0;

        void SetUseGlobalConfig( bool use );

        bool GetUseGlobalConfig( void ) const;

        private:
        friend class CConfigStore;

        UInt32 _configid;       /**< used by the CConfigStore class to speed up lookup ops */
        bool _useglobal;        /**< wheter this configurable object reacts to global config events */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-04-2005 :
        - Modified to use the new CDataNode class
- 08-02-2005 :
        - Initial implementation

---------------------------------------------------------------------------*/
