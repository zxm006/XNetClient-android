//XListPtr.cpp
#include "XListPtr.h"


XListPtr::XListPtr(void)
	: m_ulSize(0)
{
}
XListPtr::~XListPtr(void)
{
	clear();
}

XListPtr::iterator XListPtr::begin(void)
{
	return *m_iterator.m_pNext;
}

XListPtr::iterator XListPtr::end(void)
{
	return m_iterator;
}

void*XListPtr::front(void)
{
	return m_iterator.m_pNext->m_pMember;
}

void XListPtr::pop_front(void)
{
	iterator*piterator=m_iterator.m_pNext;
	if (piterator!=&m_iterator)
	{
		m_iterator.m_pNext=piterator->m_pNext;
		piterator->m_pNext->m_pPrev=&m_iterator;
		delete piterator;
		piterator=NULL;

		m_ulSize--;
	}
}
void XListPtr::push_front(void*ptr)
{
	iterator*piterator=new iterator(ptr);
	if (piterator)
	{
		iterator*pBeginiterator=m_iterator.m_pNext;
		m_iterator.m_pNext=piterator;
		pBeginiterator->m_pPrev=piterator;
		piterator->m_pPrev=&m_iterator;
		piterator->m_pNext=pBeginiterator;
		m_ulSize++;
	}
}

void*XListPtr::back(void)
{
	return m_iterator.m_pPrev->m_pMember;
}

void XListPtr::pop_back(void)
{
	iterator*piterator=m_iterator.m_pPrev;
	if (piterator!=&m_iterator)
	{
		m_iterator.m_pPrev=piterator->m_pPrev;
		piterator->m_pPrev->m_pNext=&m_iterator;
		delete piterator;
		piterator=NULL;

		m_ulSize--;
	}
}

void XListPtr::push_back(void*ptr)
{
	iterator*piterator=new iterator(ptr);
	if (piterator)
	{
		iterator*pEnditerator=m_iterator.m_pPrev;

		m_iterator.m_pPrev=piterator;
		pEnditerator->m_pNext=piterator;
		piterator->m_pPrev=pEnditerator;
		piterator->m_pNext=&m_iterator;
		m_ulSize++;
	}
}

void XListPtr::insert(XListPtr::iterator titerator,void*ptr)
{
	iterator*piterator=new iterator(ptr);
	if (piterator)
	{
		iterator*pInsertAfteriterator=titerator.m_pPrev;;
		iterator*pInsertBeforeiterator=pInsertAfteriterator->m_pNext;
		pInsertAfteriterator->m_pNext=piterator;
		piterator->m_pPrev=pInsertAfteriterator;
		piterator->m_pNext=pInsertBeforeiterator;
		pInsertBeforeiterator->m_pPrev=piterator;
		m_ulSize++;
	}
}

void XListPtr::erase(XListPtr::iterator titerator)
{
	iterator*pEraseAfteriterator=titerator.m_pPrev;;
	iterator*pEraseiterator=pEraseAfteriterator->m_pNext;
	iterator*pEraseBeforeiterator=pEraseiterator->m_pNext;
	if (pEraseiterator!=&m_iterator)
	{
		pEraseAfteriterator->m_pNext=pEraseBeforeiterator;
		pEraseBeforeiterator->m_pPrev=pEraseAfteriterator;
		delete pEraseiterator;
		pEraseiterator=NULL;

		m_ulSize--;
	}
}

unsigned int XListPtr::size(void)
{
	return m_ulSize;
}

void XListPtr::clear(void)
{
	while (size())
	{
		pop_front();
	}
}

void XListPtr::remove(void*pPtr)
{
	XListPtr::iterator item=find(pPtr);
	erase(item);
}

XListPtr::iterator XListPtr::find(void*pPtr)
{
	XListPtr::iterator item=begin();
	while (item!=end())
	{
		if (item.m_pMember==pPtr)
		{
			break;
		}
		++item;
	}
	return item;
}
