#ifndef LISTS_H
#define LISTS_H

#include <stdlib.h>

template <class LINKCLASS>
class CListNode // Node class, containing data and links to next and previous items in list */
{
private:
	LINKCLASS *m_data;
	CListNode<LINKCLASS> *cNext, *cPrev;

public:
	CListNode (LINKCLASS *e) { m_data = e; cNext = NULL; cPrev = NULL; }; // Setting the element in a new containter 

	CListNode<LINKCLASS> *next () { return ( cNext ); } // Return Next contanir in list
	CListNode<LINKCLASS> *prev () { return ( cPrev ); } // Return Next contanir in list

	LINKCLASS *Data () { return m_data; } // returning the data in the list
	void ChangeData ( LINKCLASS *data ) { m_data = data; }

	void setNext( CListNode<LINKCLASS> *node ) { cNext = node; } // Setting the Next item in list
	void setPrev( CListNode<LINKCLASS> *node ) { cPrev = node; } // Setting the previous item in list
	void delNext() { cNext = NULL; } // Removing next Node
	void delPrev() { cPrev = NULL; } // Removing prev Node
};


/*
	######################################################################################
	############################## Double Linked List Functions ##########################
	######################################################################################
*/

template <class LINKCLASS>
class CLinkList  /* Main list Class */
{
private:
	int elements;
	CListNode<LINKCLASS> *m_pBack, *m_pFront, *m_pCurrent, *m_pItr;
	bool m_bCurrentStart;

public:
	CLinkList() { // default constructor, creats an empty list
		elements = 0; 
		m_pFront = NULL;
		m_pBack = NULL;
		m_pCurrent = NULL;
		m_pItr = NULL;
		m_bCurrentStart = false;
	}
	CLinkList(int n, LINKCLASS *e); // Create n copies each with element of e
	~CLinkList() { this->destroy(); } // Deconstructer, delete current list

	void clear() { this->destroy(); }  // clears the list
	void destroy();

	void ResetIterate () { m_pItr = NULL; }
	CListNode<LINKCLASS> *Iterate () 
	{ 
		if ( !m_pItr ) 
			m_pItr = m_pFront; 
		else 
			m_pItr = m_pItr->next(); 
		return m_pItr; 
	}
	LINKCLASS *FoundIterate () { if ( !m_pItr ) return NULL; LINKCLASS *data = m_pItr->Data(); m_pItr = NULL; return data; }
	LINKCLASS *IterateData () 
	{ 
		if ( !m_pItr ) 
			return NULL; 
		return m_pItr->Data(); 
	}
	CListNode<LINKCLASS> *CurrentNode () { if ( m_bCurrentStart ) return m_pFront; return m_pCurrent; }
	LINKCLASS *CurrentData () { if ( (m_bCurrentStart) && (m_pFront) ) return m_pFront->Data(); if ( (m_pCurrent) && (!m_bCurrentStart) ) return m_pCurrent->Data(); return NULL; }
	CListNode<LINKCLASS> *SetCurrentFront () { m_pCurrent = m_pFront; m_bCurrentStart = false; return m_pCurrent; }
	CListNode<LINKCLASS> *SetCurrentBack () { m_pCurrent = m_pBack; m_bCurrentStart = false; return m_pCurrent; }
	CListNode<LINKCLASS> *MoveCurrentNext () { if ( m_bCurrentStart ) { m_pCurrent = m_pFront; m_bCurrentStart = false; } else if ( m_pCurrent ) m_pCurrent = m_pCurrent->next(); return m_pCurrent; }
	CListNode<LINKCLASS> *MoveCurrentPrev () { if ( m_pCurrent ) m_pCurrent = m_pCurrent->prev(); return m_pCurrent; }
	void RemoveCurrent () 
	{ 
		m_bCurrentStart = false;

		CListNode<LINKCLASS> *tmp = m_pCurrent->prev(); 
		this->remove ( m_pCurrent ); 
		m_pCurrent = tmp; 
		if ( !m_pCurrent ) 
		{
			m_pCurrent = m_pFront; 
			m_bCurrentStart = true; 
		}
	}
	void RemoveIterate ( bool back = false ) 
	{ 
		CListNode<LINKCLASS> *node = m_pItr->prev();
		if ( m_pItr ) 
			this->remove ( m_pItr ); 

		if ( back )
		{
			m_pItr = node;
			if ( !node )
				m_pItr = m_pFront;
		}
		else
			m_pItr = NULL; 
	}

	int size () { return elements; } // return the num of elements in list
	void incElement () { elements++; } // Increment elements, used when new node is added
	void decElement () { elements--; } // Decrement elements, used when new node is deleted

	CListNode<LINKCLASS> *Front () { return ( m_pFront ); } // return the first node in the list
	CListNode<LINKCLASS> *Back () { return ( m_pBack ); } // return the last node in the list
	LINKCLASS *First () { SetCurrentFront(); if ( m_pCurrent ) return ( m_pCurrent->Data() ); return ( NULL ); } // return the first node in the list
	LINKCLASS *Last () { SetCurrentBack(); if ( m_pCurrent ) return ( m_pCurrent->Data() ); return (NULL ); } // return the last node in the list
	LINKCLASS *Next () { if ( MoveCurrentNext() ) return m_pCurrent->Data(); return NULL; }
	LINKCLASS *Prev () { if ( MoveCurrentPrev() ) return m_pCurrent->Data(); return NULL; }

	bool empty() { // return true is the list is empty, otherwise return false
		if ( (elements <= 0) || (!m_pFront) ) 
			return true;
		else
			return false;
	}

	void ChangeCurrent ( LINKCLASS *e ) 
	{ 
		if ( m_bCurrentStart )
		{
			if ( m_pFront )
				m_pFront->ChangeData ( e );
			return;
		}
		if ( !m_pCurrent ) return; 
		m_pCurrent->ChangeData ( e ); 
	}

	void push_front (LINKCLASS *e); // Add to front of the list
	void push_back (LINKCLASS *e); // Add to end of the list
	void pop_front (); // remove node from the beginning of the list
	void pop_back (); // remove node from the end of the list

	void assign (int n, LINKCLASS *e); // Creating new list and put elements in to start with

	CListNode<LINKCLASS> *insert ( LINKCLASS *find, LINKCLASS *e )
	{
		int pos = FindPos ( find );
		if ( pos >= 0 )
			return insert ( pos + 1, e );
		return NULL;
	}
	CListNode<LINKCLASS> *insert (int pos, LINKCLASS *e); // inserts new node of element "e" into the list in position of "pos"
	CListNode<LINKCLASS> *erase (int pos ); // removes the node at element "pos"
	void remove ( CListNode<LINKCLASS> *node, bool del = true ); // remove the node
	void remove (LINKCLASS *e, bool single = false, bool del = true); // removes all elements that has the element of "e"

	CLinkList<LINKCLASS> operator=(CLinkList& list); // = operator overload
	LINKCLASS *operator[](int pos) { return Get(pos); }

	LINKCLASS *Get ( int id )
	{
		int i = 0;
		if ( !m_pFront )
			return NULL;

		CListNode<LINKCLASS> *tmpNode = m_pFront;
		while ( tmpNode )
		{
			if ( id == i )
				return tmpNode->Data();
			tmpNode = tmpNode->next();
			++i;
		}
		return NULL;
	}

	void Switch ( LINKCLASS *e, LINKCLASS *n )
	{
		CListNode<LINKCLASS> *node = m_pFront;
		while ( node )
		{
			if ( node->Data() == e )
				node->ChangeData ( n );
			node = node->next();
		}
	}

	bool FindData ( LINKCLASS *e )
	{
		CListNode<LINKCLASS> *node = m_pFront;
		while ( node )
		{
			if ( node->Data() == e )
				return true;
			node = node->next();
		}
		return false;
	}

	int FindPos ( LINKCLASS *e )
	{
		int pos = 0;
		CListNode<LINKCLASS> *node = m_pFront;
		while ( node )
		{
			if ( node->Data() == e )
				return pos;
			node = node->next();
			++pos;
		}
		return -1;
	}
};
template <class LINKCLASS>
CLinkList<LINKCLASS> CLinkList<LINKCLASS>::operator=(CLinkList& list) // = operator overload
{
	int i;
	CListNode<LINKCLASS> *oldNode, *newNode;

	this->destroy(); // removes the original list

	oldNode = list.front();
	for ( i = 1; i <= list.size(); i++ ) // loop for each node in the list
	{
		newNode = new CListNode<LINKCLASS> ( oldNode->data() ); // creating new node with old nodes element
		if ( i == 1 )
			m_pFront = newNode;
		else
		{
			m_pBack->setNext( newNode );
			newNode->setPrev( m_pBack );
		}
		m_pBack = newNode; // set newly created node as the last one in list
		oldNode = oldNode->next();
	}
	elements = list.size();

	return (*this);
}
template <class LINKCLASS>
CLinkList<LINKCLASS>::CLinkList (int n, LINKCLASS *e) // Creating new list and put elements in to start with
{ /// creates n copies of element e in a list
	int i;
	CListNode<LINKCLASS> *newNode;
	CListNode<LINKCLASS> *tmpNode;

	for ( i = 0; i < n; i++ )
	{
		newNode = new CListNode<LINKCLASS>( e ); // create new node
		tmpNode = m_pBack;  // assign last node added to tmp value
		m_pBack = newNode; // assign new node to back of list
		if ( i == 0 )  // if first node, add as front node
			m_pFront = newNode; 
		else
		{
			newNode->setPrev ( tmpNode );  // set new nodes previous pointer to last created node
			tmpNode->setNext ( newNode );  // set last created nodes next pointer to new node
		}
	}

	elements = n; // set elements to number of elements added

}
template <class LINKCLASS>
CListNode<LINKCLASS> *CLinkList<LINKCLASS>::insert (int pos, LINKCLASS *e) // insert a new node in the "pos" of the list with the element of "e"
{ // cant add outside of the list,
	int i, max;
	CListNode<LINKCLASS> *tmpNode, *newNode;

	if ( pos > (elements + 1) ) // element is out side of the list
		return NULL;

	newNode = new CListNode<LINKCLASS>( e ); // create new node

	tmpNode = m_pFront;

	max = pos;
	if ( max > elements ) // make sure it doesn't loop past the number of elements, so it doesn't try to access a point that doesn't exist
		max = elements ;

	for ( i = 1; i < max; i++ ) // get to position to insert
		tmpNode = tmpNode->next();

	if ( (pos != 1) && (pos < (elements + 1)) ) // beginning pos doesn't have previous, end pos has different values
	{ 
		newNode->setPrev( tmpNode->prev() );
		tmpNode->prev()->setNext( newNode );
	}
	if ( pos < (elements + 1) ) // end node doesn't have next
	{
		newNode->setNext( tmpNode );
		tmpNode->setPrev( newNode );
	}
	if ( pos == (elements + 1) ) // inserting at the end
	{
		tmpNode->setNext ( newNode );
		newNode->setPrev ( tmpNode );
		m_pBack = newNode;
	}
	if ( pos == 1 ) // inserting at the beginning
		m_pFront = newNode;

	elements++;

	return newNode;
}

template <class LINKCLASS>
void CLinkList<LINKCLASS>::remove ( CListNode<LINKCLASS> *node, bool del ) 
{
	if ( !node )
		return;

	// is there a next node ?
	if ( node->next() )
		node->next()->setPrev ( node->prev() );
	else if ( m_pBack == node )
		m_pBack = node->prev();

	if ( node->prev() )
		node->prev()->setNext ( node->next() );
	else if ( m_pFront == node )
		m_pFront = node->next();

	if ( del )
		delete node;
}

template <class LINKCLASS>
CListNode<LINKCLASS> *CLinkList<LINKCLASS>::erase (int pos) // erases a node at position of "pos"
{ // cant erase node outside of list
	int i;
	CListNode<LINKCLASS> *tmpNode, *returnNode;

	if ( pos > elements ) // element is out side of the list
		return NULL;

	tmpNode = m_pFront;

	for ( i = 1; i < pos; i++ ) // get to position to insert
		tmpNode = tmpNode->next();

	if ( (pos != 1) && (pos < elements) ) // dont set if start or end node
	{
		tmpNode->prev()->setNext( tmpNode->next() );
		tmpNode->next()->setPrev( tmpNode->prev() );
	}
	if ( pos == 1 )
	{
		tmpNode->delPrev();
		m_pFront = tmpNode->next();
	}
	if ( pos == elements )
	{
		if ( tmpNode->prev() )
			tmpNode->prev()->delNext();
		m_pBack = tmpNode->prev();
		returnNode = tmpNode->prev();
	}
	else
		returnNode = tmpNode->next();
	elements--;
	delete tmpNode;

	return returnNode;
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::remove (LINKCLASS *e, bool single, bool del ) // removes all nodes with elements of "e"
{
	CListNode<LINKCLASS> *tmpNode, *tmpNode2, *nextNode;

	tmpNode = m_pFront; // set node to first node in list
	while ( tmpNode != NULL ) // loop until psat last node
	{
		nextNode = tmpNode->next(); // set the next noe to look at
		if ( tmpNode->Data() == e ) // remove node
		{
			if ( (tmpNode == m_pFront) && (tmpNode == m_pBack) ) // only node in list, list will be empty
			{
				m_pFront = NULL; 
				m_pBack = NULL;
			}
			else if ( tmpNode == m_pFront ) // if front node, delete next nodes previous value
			{
				tmpNode->next()->delPrev();
				m_pFront = tmpNode->next();
			}
			else if ( tmpNode == m_pBack ) // if back node, delete previous nodes next value, and set back value
			{
				tmpNode->prev()->delNext();
				m_pBack = tmpNode->prev();
			}
			else // node is somewhere in the middle
			{
				tmpNode2 = tmpNode->next();
				tmpNode->next()->setPrev(tmpNode->prev());
				tmpNode->prev()->setNext(tmpNode2);
			}
			if ( del ) delete tmpNode; // delete node form memory
			elements--; // decrement elements
			if ( single ) break;
		}
		tmpNode = nextNode; // assign new tmpnode for loop
	}
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::assign (int n, LINKCLASS *e) // Creating new list and put elements in to start with
{
	int i;
	CListNode<LINKCLASS> *newNode, *tmpNode, *curNode;

	// Delete existiing nodes from the list
    curNode = m_pFront; // set current node as the first node in list
    while( curNode != NULL ) // loop until no nodes remain
	{
        tmpNode = curNode->next(); // set temp node as the next node in list
        delete curNode; // delete current Node

		curNode = tmpNode; // change current node to next in list
    } 
	// Add new nodes into the now empty list
	for ( i = 0; i < n; i++ )
	{
		newNode = new CListNode<LINKCLASS>( e ); // create new node
		tmpNode = m_pBack;  // assign last node added to tmp value
		m_pBack = newNode; // assign new node to back of list
		if ( i == 0 )  // if first node, add as front node
			m_pFront = newNode; 
		else
		{
			newNode->setPrev ( tmpNode );  // set new nodes previous pointer to last created node
			tmpNode->setNext ( newNode );  // set last created nodes next pointer to new node
		}
	}

	elements = n; // set elements to number of elements added
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::destroy() // removes list from memory
{
    CListNode<LINKCLASS> *curNode, *tmpNode;

    curNode = m_pFront; // set current node as the first node in list
    while( curNode != NULL ) // loop until no nodes remain
	{
        tmpNode = curNode->next(); // set temp node as the next node in list
        delete curNode; // delete current Node

		curNode = tmpNode; // change current node to next in list
    } 

    m_pFront = NULL;
    m_pBack = NULL;
	elements = 0;
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::push_back ( LINKCLASS *e ) // Insert data at end of the list
{
	CListNode<LINKCLASS> *newNode;
	CListNode<LINKCLASS> *tmpNode;

	newNode = new CListNode<LINKCLASS> ( e );

    // place the node in the list
    if( m_pFront == NULL ) // list is empty
	{
        m_pFront = newNode; 
        m_pBack = newNode;
    } 
    else // list already has something in
    {
		tmpNode = m_pBack;
        m_pBack->setNext( newNode ); // set old last node to point to new node
        m_pBack = newNode; // set new node as last item
		newNode->setPrev ( tmpNode ); // Set new nodes previous as original last node
    }
	this->incElement(); // Increment element numbers
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::push_front ( LINKCLASS *e ) // Insert data at end of the list
{
	CListNode<LINKCLASS> *newNode;
	CListNode<LINKCLASS> *tmpNode;

	newNode = new CListNode<LINKCLASS>( e );

    // place the node in the list
    if( m_pFront == NULL ) // list is empty
	{
        m_pFront = newNode; 
        m_pBack = newNode;
    } 
    else // list already has something in
    {
		tmpNode = m_pFront;
        m_pFront->setPrev( newNode ); // set old first node to point to new node
        m_pFront = newNode; // set new node as first item
		newNode->setNext ( tmpNode ); // Set new nodes next as original front node
    }
	this->incElement(); // Increment element numbers
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::pop_back()
{
	CListNode<LINKCLASS> *tmpNode;

	if ( m_pBack->prev() )
		m_pBack->prev()->delNext(); 
	tmpNode = m_pBack->prev();
	if ( m_pBack == m_pFront )
		m_pFront = NULL;
	delete m_pBack; // remove the last node from memory
	m_pBack = tmpNode; // set new m_pBack Node
	this->decElement(); // decrement number of elements
}
template <class LINKCLASS>
void CLinkList<LINKCLASS>::pop_front()
{
	CListNode<LINKCLASS> *tmpNode;

	m_pFront->next()->delPrev(); 
	tmpNode = m_pFront->next();
	delete m_pFront; // remove the first node from memory
	m_pFront = tmpNode; // set new m_pFront Node
	this->decElement(); // decrement number of elements
}

/*
template <class LINKCLASS>
LINKCLASS *Get ( int id )
{
	int i = 0;
	if ( !m_pFront )
		return NULL;

	CListNode<LINKCLASS> *tmpNode = m_pFront;
	while ( tmpNode )
	{
		if ( id == i )
			return tmpNode->data();
		tmpNode = tmpNode->next();
		++i;
	}
	return NULL;
}
*/

/*
    #####################################################################################################
	###################################### End Linked List Class ########################################
	#####################################################################################################
*/

#endif
