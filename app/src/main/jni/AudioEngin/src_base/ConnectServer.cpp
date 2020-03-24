// TestClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#ifdef WIN32
#include <assert.h>
#endif
#include <assert.h>
#include "XNet/XNetCore.h"
#include "XNet/XNetSetting.h"
#include "XNet/XNetClient.h"
#include "ConnectServer.h"
#include "AppClient.h"

#include "log.h"

CConnectServer::CConnectServer()
{
	m_pAppClient = NULL;

}
CConnectServer::~CConnectServer()
{
	
}

int CConnectServer::Start(const char* ip,unsigned port,bool isEncrypt, CmdTCPClientCallback* pCallback)
{
	assert(ip);
	assert(pCallback);

	LOGI("CConnect Server::Start in");
	
	if (m_pAppClient == NULL)
	{
		m_pAppClient = (CAppClient*)new CAppClient();
		assert(m_pAppClient);

		((CAppClient*)m_pAppClient)->SetFunPointer(pCallback);
		return ((CAppClient*)m_pAppClient)->Open(ip,port,isEncrypt);
	}
	return 0;
}

bool CConnectServer::Stop()
{
	if(m_pAppClient == NULL)
		return false;
	else{
		((CAppClient*)m_pAppClient)->Close();
		delete (CAppClient*)m_pAppClient;
		m_pAppClient = NULL;
	}
	return true;
}

  
void CConnectServer::SendAudioData(char*pData, int nLen)
{
    
}

int CConnectServer::SendData( KCmdPacketEx& pPacket)
{
	std::string data = pPacket.GetString();
	return  (int)((CAppClient*)m_pAppClient)->SendData(data.c_str(),(int)data.length()+1);
}

std::string CConnectServer::GetLocalIP()
{
	if(m_pAppClient)
		return ((CAppClient*)m_pAppClient)->GetLocalIP();
	return "";
}

std::string CConnectServer::GetNATIP()
{
	if(m_pAppClient)
		return ((CAppClient*)m_pAppClient)->GetNATIP();
	return "";
}

int CConnectServer::getIpByDomain(const char *domain, char *ip)
{
	char **pptr;
	struct hostent *hptr;

	hptr = gethostbyname(domain);
	if(NULL == hptr)
	{
		printf("gethostbyname error for host:%s\n", domain);
		return -1;
	}

	for(pptr = hptr->h_addr_list ; *pptr != NULL; pptr++)
	{
		if (NULL != inet_ntop(hptr->h_addrtype, *pptr, ip, IP_SIZE) )
		{
			return 0; // ֻ��ȡ��һ�� ip
		}
	}

	return -1;  
}

int CConnectServer::checkIsDomain(const char *pAddr)
{
	if(!pAddr || strlen(pAddr) < 1)
	{
		return -1;
	}
	unsigned long  add = inet_addr(pAddr);
	if (add == INADDR_NONE)
	{
		//printf("");
        	return 0;
	}
    	else
       {
       	return 1;
       }
}




