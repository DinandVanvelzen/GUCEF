/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H
#define GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNNode;

/**
 *  The base of the objects of a node that tell the application of what happens: sessions, grids and streams, bulk
 *  transfers. Their events reach the application on its pulse generator, as those of the node (CDRNNode).
 *
 *  Such an object is made and let go of by the node, also on the node's thread and under the node lock (a grid that
 *  another member announced, a transfer that another member offered). It therefore takes no subscription to the pulse
 *  generator of the application, whose pulses take the node lock (an application handler that calls into the node):
 *  subscribing or unsubscribing would wait for such a pulse under the node lock. With a node thread of its own its
 *  events go into the mail of the node (CDRNNode::PostApplicationEvent()), which the pulse generator of the application
 *  delivers, in the order in which the objects of the node raised them; without one they go out at once, on the thread
 *  that works on the node.
 */
class GUCEF_DRN_EXPORT_CPP CDRNApplicationNotifier : public CORE::CTSGNotifier
{
    public:

    explicit CDRNApplicationNotifier( CDRNNode& node );

    /** Mail of this object that the node did not deliver yet goes; a delivery under way on another thread is waited for */
    virtual ~CDRNApplicationNotifier() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  Raises an event for the application: through the mail of the node when the node has a thread of its own, at once
     *  otherwise. The node lock may be held, the event data is copied when it is mailed.
     */
    void EmitToApplication( const CORE::CEvent& eventid, CORE::CICloneable* eventData = GUCEF_NULL );

    private:

    friend class CDRNNode;

    /** Called by the node on the thread of the application's pulse generator, for mail of this object */
    void DeliverApplicationEvent( const CORE::CEvent& eventid, CORE::CICloneable* eventData );

    CDRNApplicationNotifier( const CDRNApplicationNotifier& src );              /**< not implemented */
    CDRNApplicationNotifier& operator=( const CDRNApplicationNotifier& src );   /**< not implemented */

    CDRNNode* m_applicationNode;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNAPPLICATIONNOTIFIER_H ? */
