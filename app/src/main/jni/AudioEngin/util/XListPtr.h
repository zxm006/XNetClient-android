//XListPtr.h
#ifndef __XLISTPTR_H__
#define __XLISTPTR_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifdef WIN32
#include <winsock2.h>
#endif

class XListPtr
{
public:
	XListPtr(void);
	virtual~XListPtr(void);
public:
	class iterator
	{
	public:
		iterator(void*pMember=NULL)
		{
			m_pMember=pMember;
			m_pNext=this;
			m_pPrev=this;
		}
		virtual~iterator(void)
		{
		}
	public:

		void operator ++()
		{
			*this=*m_pNext;
		}
		void operator --()
		{
			*this=*m_pPrev;
		}
		void* operator *()
		{
			return m_pMember;
		}
		iterator &operator =(iterator titerator)
		{
			m_pMember=titerator.m_pMember;
			m_pNext=titerator.m_pNext;
			m_pPrev=titerator.m_pPrev;
			return (*this);
		}
		bool operator ==(iterator titerator)
		{
			return (m_pMember==titerator.m_pMember &&
					m_pNext==titerator.m_pNext &&
					m_pPrev==titerator.m_pPrev);
		}
		bool operator !=(iterator titerator)
		{
			return (m_pMember!=titerator.m_pMember ||
					m_pNext!=titerator.m_pNext ||
					m_pPrev!=titerator.m_pPrev);
		}
        public:
     void*        m_pMember;
        
	protected:
	
		iterator*	m_pNext;
		iterator*	m_pPrev;

		friend class XListPtr;
	};
public:

	XListPtr::iterator begin(void);
	XListPtr::iterator end(void);
	void*front(void);
	void pop_front(void);
	void push_front(void*ptr);
	void*back(void);
	void pop_back(void);
	void push_back(void*ptr);
	void insert(XListPtr::iterator titerator,void*ptr);
	void erase(XListPtr::iterator titerator);
	unsigned int size(void);
	void clear(void);
	void remove(void*pPtr);
	XListPtr::iterator find(void*pPtr);
	iterator		m_iterator;
protected:

	unsigned int	m_ulSize;
};


#endif
