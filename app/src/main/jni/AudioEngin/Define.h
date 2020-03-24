#ifndef _PACKET_H__
#define _PACKET_H__

#include <unistd.h>
#include <string.h>
typedef struct PACKET
{
	PACKET(void*pPacketData,int nPacketSize)
	{
		m_pPacketData = new char[nPacketSize];
		m_nPacketSize = nPacketSize;
		memcpy(m_pPacketData,pPacketData,nPacketSize);
	}

    ~PACKET() {
        if (m_pPacketData != NULL) {
            delete[] (char *)m_pPacketData;
            m_pPacketData = NULL;
            m_nPacketSize = 0;
        }
    }
	void*	m_pPacketData;
	int		m_nPacketSize;
}PACKET,*PPACKET;


#define Pause(ulMS) usleep(ulMS*1000)


#endif
