/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H
#define GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTGeoObjectBase;

/*-------------------------------------------------------------------------*/

/**
 *  The intrusive link from linked objects to the spatial structures that hold them (section 7.4.3 of the design): a
 *  dynamic AABB tree, a ray caster or a physics broadphase derives from this class and registers itself with each
 *  object it holds (CTGeoObjectBase::RegisterStructure). It is not a GUCEF event: the calls are direct, on the owner
 *  thread of the tree, and allocation-free once registered.
 *
 *  - OnGeoObjectMoved: the object's world placement (moved itself or through an ancestor) or its shape changed. Called
 *    at most once per object until the structure calls CTGeoObjectBase::AcknowledgeMove for it, typically in its
 *    Update() after refitting the object's bounds: the structure keeps a moved list and refits O(moved) objects.
 *    The implementation must not change the tree; reading the object (bounds, matrices) and unregistering from it
 *    are allowed.
 *  - OnGeoObjectDestroyed: the object is being destroyed (from CTGeoObjectBase's destructor, so the shape is already
 *    gone): remove every reference to it. The link is already removed when this runs.
 *  - The base keeps the list of its registered objects (GetObjectCount, GetObject; the order changes on removal) and
 *    unregisters from all of them when it is destroyed, so an object never holds a link to a destroyed structure.
 *    A derived structure may unregister earlier with UnregisterAll.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoObjectStructure
{
    public:

    /** Unregisters from every object still registered */
    virtual ~CTGeoObjectStructure() { UnregisterAll(); }

    /** The number of objects registered with this structure */
    UInt32 GetObjectCount( void ) const { return (UInt32) m_objects.size(); }

    /** Registered object 'index' (below GetObjectCount) */
    CTGeoObjectBase< T >* GetObject( UInt32 index ) const { return m_objects[ index ]; }

    protected:

    friend class CTGeoObjectBase< T >;

    CTGeoObjectStructure( void ) : m_objects() {}

    /** The object moved or changed its shape: append it to the moved list (once until AcknowledgeMove) */
    virtual void OnGeoObjectMoved( CTGeoObjectBase< T >& object ) = 0;

    /** The object is being destroyed: remove it */
    virtual void OnGeoObjectDestroyed( CTGeoObjectBase< T >& object ) = 0;

    /** Unregisters from every registered object (no callbacks) */
    void UnregisterAll( void );

    private:

    CTGeoObjectStructure( const CTGeoObjectStructure& );
    CTGeoObjectStructure& operator=( const CTGeoObjectStructure& );

    GUCEF::vector< CTGeoObjectBase< T >* > m_objects;   /**< maintained by CTGeoObjectBase: each link knows its slot */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* the members of the structure that need the object class (UnregisterAll) are defined there */
#ifndef GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#include "gucefGEOMETRY_CTGeoObjectBase.h"
#define GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECTBASE_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H ? */
