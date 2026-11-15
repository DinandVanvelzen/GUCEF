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

#include "CNode.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void 
CNode::AttachToNode(CNode *pnewparent)	//adds at top of list - this will be the first
{
        if ( pnewparent ) // not NULL
        {
        	/*
	         *	For safety: if this node is already attached then detach
        	 */
        	if ( _pparent ) DetachNode();

        	_pparent = pnewparent;

        	if (_pparent->_pchild)
	        {	CNode *p0 = _pparent->_pchild;
		        CNode *p1 = _pparent->_pchild->_pprev;
        		CNode *p2 = this;
	        	CNode *p3 = _pprev;
		        p0->_pprev = p3;		p3->_pnext = p0;
        		p1->_pnext = p2;		p2->_pprev = p1;
	        }
        	else
	        {
                	_pparent->_pchild = this;		//	Make first child
        	}
        }
}
/*-------------------------------------------------------------------------*/

void 
CNode::AttachChildNode( CNode *pnewchild )	//adds at top of list - this will be the first
{
        if ( pnewchild )
        {
        	/*
	         *	For safety: if this node is already attached then detach
        	 */
	        if ( pnewchild->_pparent )
                {
                        pnewchild->DetachNode()                             ;
                }

                /*
                 *      Hook to this object as its parent
                 */
	        pnewchild->_pparent = this                               ;

                /*
                 *      Are there children already ?
                 *      if so add to beginning of list
                 */
	        if ( _pchild )
        	{
                	CNode *pfirst	= _pchild                        ;
	        	CNode *plast	= _pchild->_pprev                 ;
	        	CNode *pnewlast = pnewchild->_pprev              ;
		        pfirst->_pprev = pnewlast                        ;
                        pnewlast->_pnext = pfirst                        ;
        		plast->_pnext = pnewchild                        ;
                        pnewchild->_pprev = plast                        ;
	        }
        	else
	        {
                        /*
                         *      First child
                         */
                        _pchild = pnewchild                              ;
	        }
        }
}

/*-------------------------------------------------------------------------*/

void
CNode::AppendChildNode( CNode *pnewchild  )
{
        /*
         *      attach new child to end of list
         */
        if ( pnewchild )
        {
        	/*
	         *	For safety: if this node is already attached then detach
        	 */
	        if ( pnewchild->_pparent )
                {
                        pnewchild->DetachNode();
                }

                /*
                 *      Hook to this object as its parent
                 */
	        pnewchild->_pparent = this;

                /*
                 *      Are there children already ?
                 *      if so add to end of list.
                 */
	        if ( _pchild )
        	{
                        _pchild->_pprev->_pnext = pnewchild;
                        pnewchild->_pprev = _pchild->_pprev;
                        _pchild->_pprev = pnewchild;
                        pnewchild->_pnext = _pchild;
	        }
        	else
	        {
                        /*
                         *      First child
                         */
                        _pchild = pnewchild;
	        }
        }         
}

/*-------------------------------------------------------------------------*/

void 
CNode::DetachNode( void )
{
	//
	//	If this node is the first child of the parent (first in list)
	//	Let the parent point to the next child in this list
	//
	if ( _pparent && ( _pparent->_pchild == this ) )
	{
        	_pparent->_pchild = ( _pnext == this) ? GUCEF_NULL : _pnext;
	}

	//
	//	Exclude this node from the cyclic linked list
	//
        if ( _pprev )
        {
        	_pprev->_pnext = _pnext;
	        _pnext->_pprev = _pprev;
        }
	_pprev = this;
	_pnext = this;
	_pparent = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CNode* 
CNode::FindRootNode( void )
{
     return _pparent ? _pparent->FindRootNode() : this;
}

/*-------------------------------------------------------------------------*/

void
CNode::MoveToFront( void )
{
        if ( !_pparent ) return;

        /*
         *      First we take this node out of the list
         */
        CNode *parent = _pparent;
        DetachNode();

        /*
         *      Now we atach the node again as child thus first node in list
         */
        parent->AttachChildNode( this );
}

/*-------------------------------------------------------------------------*/

void
CNode::MoveToBack( void )
{
        if ( !_pparent ) return;

        /*
         *      First we take this node out of the list
         */
        CNode *parent = _pparent;
        DetachNode();

        /*
         *      Now we append the node.
         */
        parent->AppendChildNode( this );
}

/*-------------------------------------------------------------------------*/

void
CNode::MoveForward( void )
{
        if ( !_pparent ) return;
        if ( this == _pparent->_pchild ) return;

        CNode *tmp;
        CNode *other = this->_pprev;

        tmp = this->_pchild;
        this->_pchild = other->_pchild;
        other->_pchild = tmp;

        tmp = this->_pprev;
        this->_pprev = other->_pprev;
        other->_pprev = tmp;

        tmp = this->_pnext;
        this->_pnext = other->_pnext;
        other->_pnext = tmp;
}

/*-------------------------------------------------------------------------*/

void
CNode::MoveBackward( void )
{
        if ( !_pparent ) return;
        if ( this == _pparent->_pchild->_pprev ) return;

        CNode *tmp;
        CNode *other = this->_pnext;

        tmp = this->_pchild;
        this->_pchild = other->_pchild;
        other->_pchild = tmp;

        tmp = this->_pprev;
        this->_pprev = other->_pprev;
        other->_pprev = tmp;

        tmp = this->_pnext;
        this->_pnext = other->_pnext;
        other->_pnext = tmp;
}

/*-------------------------------------------------------------------------*/

bool 
CNode::IsFirst( void ) const
{	
        if ( _pparent )
        {	
                return _pparent->_pchild == this;
	}
	else
	{	
	        return false;	//should be undefined (root node found)
	}
}
	
/*-------------------------------------------------------------------------*/
	
bool 
CNode::IsLast( void ) const
{	
        if ( _pparent )
	{	
	        return _pparent->_pchild->_pprev == this;
	}
	else
	{	
	        return false;	//should be undefined (root node found)
	}
}

/*-------------------------------------------------------------------------*/

CNode* 
CNode::GetNextNode( void ) const
{ 
        return _pnext; 
}

/*-------------------------------------------------------------------------*/

CNode* 
CNode::GetPrevNode( void ) const 
{ 
        return _pprev; 
}

/*-------------------------------------------------------------------------*/

CNode* 
CNode::GetParentNode( void ) const 
{ 
        return _pparent; 
}

/*-------------------------------------------------------------------------*/

CNode* 
CNode::GetChildNode( void ) const 
{ 
        return _pchild; 
}

/*-------------------------------------------------------------------------*/
 
CNode::~CNode()
{
        DetachNode();
}

/*-------------------------------------------------------------------------*/

CNode::CNode( void ) 
        : _pparent( GUCEF_NULL ), 
          _pchild( GUCEF_NULL )
{
        _pprev = _pnext = this;
}

/*-------------------------------------------------------------------------*/

CNode::CNode( CNode *pnode ) 
        : _pparent( GUCEF_NULL ), 
          _pchild( GUCEF_NULL )
{	
        _pprev = _pnext = this;
	AttachToNode( pnode );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
