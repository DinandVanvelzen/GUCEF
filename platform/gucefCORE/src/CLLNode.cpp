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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "CLLNode.h"              /* definition for this class */
#ifndef GUCEF_CORE_TSPRINTING_H
#include "tsprinting.h"		  /* threadsafe printing */ 
#endif /* GUCEF_CORE_TSPRINTING_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */  

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

//-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------//

CLLNode::CLLNode( void )
{
        GUCEF_BEGIN;

        /*
         *      Default contructor
         */
        next = GUCEF_NULL;
        prev = GUCEF_NULL;
        link = GUCEF_NULL;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CLLNode::CLLNode( void* set_link )
{
        GUCEF_BEGIN;

        /*
         *      contructor, init link to given void*
         */
        next = GUCEF_NULL;
        prev = GUCEF_NULL;
        link = set_link;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CLLNode::~CLLNode()
{
        GUCEF_BEGIN;

        /*
         *      destructor
         */
        Detach();
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

void
CLLNode::Detach( void )
{
        GUCEF_BEGIN;

	/*
         *	Detaches this node from the linked list
         */

        if ( prev )
        {
        	if ( next )
                {
                	next->prev = prev;
                        prev->next = next;
                        GUCEF_END;
                        return;
                }
        	prev->next = GUCEF_NULL;
        	GUCEF_END;
                return;
        }
        if ( next )
        {
        	next->prev = GUCEF_NULL;
        }
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

void
CLLNode::AppendTo( CLLNode *node )
{
        GUCEF_BEGIN;
        
        /*
         *      Append this node to the end of the list given by the node
         *      specified
         */
        CLLNode *tmp_node = node; 
        while ( tmp_node->GetNext() )
        {
                tmp_node = tmp_node->GetNext();
        }

        tmp_node->SetNext( this );
        SetPrev( tmp_node );
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

void
CLLNode::PrefixTo( CLLNode *node )
{
        GUCEF_BEGIN;

        /*
         *      prefix this node to the start of the list given by the node
         *      specified
         */
        CLLNode *tmp_node = node;
        while ( tmp_node->GetPrev() )
        {
                tmp_node = tmp_node->GetPrev();
        }
        tmp_node->SetPrev( this );
        SetNext( tmp_node );
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CLLNode*
CLLNode::GetNext( void )
{
        GUCEF_BEGIN;

        /*
         *      return the next pointer
         */
        GUCEF_END;
        return next;
}

/*-------------------------------------------------------------------------*/

void
CLLNode::SetNext( CLLNode *set_next )
{
        GUCEF_BEGIN;

        /*
         *      Set the next pointer
         */
        next = set_next;
        GUCEF_END;
}

/*-------------------------------------------------------------------------*/

CLLNode*
CLLNode::GetPrev( void )
{
        GUCEF_BEGIN;

        /*
         *      Return the previous node
         */
        GUCEF_END; 
        return prev;
}

/*-------------------------------------------------------------------------*/

void
CLLNode::SetPrev( CLLNode *set_prev )
{
        GUCEF_BEGIN;
        
        /*
         *      Set the prev pointer
         */
        prev = set_prev; 
        GUCEF_END;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

