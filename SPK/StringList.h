#ifndef __BASE_ENGINE_STRINGLIST_H__
#define __BASE_ENGINE_STRINGLIST_H__

#include "String.h"

typedef struct STRUCT_STRINGLIST {
	struct STRUCT_STRINGLIST *next;
	struct STRUCT_STRINGLIST *prev;
	String str;
	String data;
	bool remove;
} SStringList;

class CStringList
{
public:
	CStringList () { m_head = m_tail = NULL; m_iCount = 0; }
	~CStringList () 
	{ 
		SStringList *node = m_head;
		while ( node )
		{
			SStringList *nextnode = node->next;
			delete node;
			node = nextnode;
		}
	}

	void SplitToken ( char token, String str )
	{
		int num = 0;
		String *s = str.SplitToken ( token, &num );

		for ( int i = 0; i < num; i++ )
			PushBack ( s[i] );
	}

	CStringList &operator= ( CStringList &list ) 
	{ 
		for ( SStringList *node = list.Head(); node; node = node->next )
		{
			if ( node->remove )
				continue;
			SStringList *newNode = PushBack ( node->str, node->data );
		}
		return (*this); 
	}

	SStringList *Head () { return m_head; }
	SStringList *Tail () { return m_tail; }

	bool Empty() { if ( m_head ) return false; return true; }

	void Clear() 
	{
		SStringList *node = m_head;
		while ( node )
		{
			SStringList *nextnode = node->next;
			delete node;
			node = nextnode;
		}
		m_head = m_tail = NULL;
		m_iCount = 0;
	}

	int Count () { return m_iCount; }

	SStringList *GetAt ( int at )
	{
		if ( at >= m_iCount ) return NULL;

		SStringList *node = m_head;
		for ( int i = 0; i < at; i++ )
		{
			node = node->next;
		}
		return node;
	}

	void DontRemove ( String &str )
	{
		SStringList *node = m_head;
		while ( node )
		{
			if ( node->str == str )
			{
				node->remove = false;
				break;
			}
			node = node->next;
		}
	}

	SStringList *FindString ( String &str )
	{
		for ( SStringList *node = m_head; m_head; m_head = m_head->next )
		{
			if ( node->str == str )
				return node;
		}
		return NULL;
	}

	void RemoveMarked ()
	{
		SStringList *node = m_head;
		while ( node )
		{
			if ( node->remove )
			{
				if ( node->prev )
					node->prev->next = node->next;
				else
					m_head = node->next;

				if ( node->next )
					node->next->prev = node->prev;
				else
					m_tail = node->prev;

				SStringList *nextnode = node->next;
				delete node;
				node = nextnode;
				--m_iCount;
			}
			else
				node = node->next;
		}
		m_head = m_tail = NULL;

	}

	bool Remove ( String &str, bool single = true )
	{
		bool removed = false;

		SStringList *node = m_head;
		while ( node )
		{
			if ( node->str == str )
			{
				if ( node->prev )
					node->prev->next = node->next;
				else
					m_head = node->next;

				if ( node->next )
					node->next->prev = node->prev;
				else
					m_tail = node->prev;

				SStringList *nextnode = node->next;
				delete node;
				removed = true;
				--m_iCount;
				node = nextnode;
				if ( single )
					break;
			}
			else
				node = node->next;
		}
		return removed;
	}

	SStringList *Change ( String &str, String &to )
	{
		SStringList *node = m_head;
		while ( node )
		{
			if ( node->str == str )
			{
				node->str = to;
				return node;
			}
			node = node->next;
		}
		return NULL;
	}

	void PopBack ()
	{
		if ( !m_head )
			return;

		if ( m_tail->prev )
			m_tail->prev->next = NULL;
		else
			m_head = NULL;
		SStringList *node = m_tail->prev;
		delete m_tail;
		m_tail = node;
		--m_iCount;
	}

	void PopFront ()
	{
		if ( !m_head )
			return;

		if ( m_head->next )
			m_head->next->prev = NULL;
		else
			m_tail = NULL;
		SStringList *node = m_head->next;
		delete m_head;
		m_head = node;
		--m_iCount;
	}

	void DeleteFrom ( SStringList *node )
	{
		m_tail = node->prev;
		if ( !m_tail )
			m_head = NULL;
		else
			m_tail->next = NULL;

		while ( node )
		{
			SStringList *nextnode = node->next;
			delete node;
			--m_iCount;
			node = nextnode;
		}
	}

	void PushFront ( String &str, String &data )
	{
		SStringList *s = new SStringList;
		s->next = s->prev = NULL;
		s->str = str;
		s->data = data;
		s->remove = false;

		if ( !m_tail )
			m_tail = s;
		else
		{
			m_head->prev = s;
			s->next = m_head;
		}
		m_head = s;
		++m_iCount;
	}

	SStringList *PushBack ( String &str, String &data, bool search = false )
	{
		if ( search ) 
		{
			SStringList *n = FindString(str);
			if ( n )
			{
				n->data = data;
				n->remove = false;
				return n;
			}
		}
		SStringList *s = new SStringList;
		s->next = s->prev = NULL;
		s->str = str;
		s->data = data;
		s->remove = false;

		if ( !m_head )
			m_head = s;
		else
		{
			m_tail->next = s;
			s->prev = m_tail;
		}
		m_tail = s;
		++m_iCount;
		return m_tail;
	}

	SStringList *PushBack ( String &str, bool search = false )
	{
		if ( search ) 
		{
			SStringList *n = FindString(str);
			if ( n )
			{
				n->remove = false;
				return n;
			}
		}
		SStringList *s = new SStringList;
		s->next = s->prev = NULL;
		s->str = str;
		s->remove = false;

		if ( !m_head )
			m_head = s;
		else
		{
			m_tail->next = s;
			s->prev = m_tail;
		}
		m_tail = s;
		++m_iCount;
		return m_tail;
	}

private:
	SStringList *m_head;
	SStringList *m_tail;

	int m_iCount;
};

#endif //__BASE_ENGINE_STRINGLIST_H__
