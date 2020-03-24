//#include "StdAfx.h"
#include "AppClient.h"
#include <assert.h>
#include "log.h"
#include "aes.h"

#include <string.h>
#define CBC 1
#define ECB 1

 const uint8_t kcrypt[] = {0x1b, 0x6e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa9, 0x1b, 0xf7, 0x85, 0x88, 0x09, 0xcf, 0x4f, 0x11};

CAppClient::CAppClient(void)
{
	m_pClient=NULL;
	m_bConnected=false;
	m_pCallback = NULL;
    m_isEncrypt = true;
}

CAppClient::~CAppClient(void)
{
}

bool CAppClient::IsConnnected(void)
{
	return m_bConnected;
}

int CAppClient::SendData(const char*pData,int nLen)
{
    if (m_pClient!=NULL)
    {
        if(m_isEncrypt)
        {
            nLen = nLen +1;
            int sum = (nLen/16);
            if (nLen%16>0)
            {
                sum=sum+1;
            }
            uint8_t buffer[sum*16];
            memset(buffer,'\0',sum*16);
            for(int i = 0; i < sum; ++i)
            {
                AES128_ECB_encrypt((const uint8_t*)pData+(i*16),kcrypt,( uint8_t*)buffer+(i*16));
            }
            return m_pClient->SendDataToServer((const char*)buffer,sum*16);
        }
        else
        { return m_pClient->SendDataToServer((const char*)pData,nLen);
        }
    }
    return 0;
}

void CAppClient::SetFunPointer(CmdTCPClientCallback* pCallback)
{
	assert(pCallback);
	m_pCallback = pCallback;
}

std::string CAppClient::GetLocalIP()
{
	if (m_pClient!=NULL)
	{
		return m_pClient->GetLocalIP();
	}
	return "";
}

std::string CAppClient::GetNATIP()
{
	if (m_pClient!=NULL)
	{
		return m_pClient->GetNATIP();
	}
	return "";
}

int CAppClient::Open(const char*cszIP,unsigned short usPort,bool isEncrypt)
{
    m_isEncrypt = isEncrypt;
    
	XNetCore::Start(0);
	m_bConnected=false;
	m_pClient=XNetClient::Create(*this);
	return m_pClient->Connect(1,cszIP,usPort,"");
}

void CAppClient::Close(void)
{
	if (NULL != m_pClient)
	{
		m_pClient->Disconnect();
		delete m_pClient;
		m_pClient=NULL;
	}
	m_pCallback = NULL;
	XNetCore::Stop();
}

//�յ�����״̬
void CAppClient::OnXNetSessionConnectStatus(XNetSession::CONNECT_STATUS cs,int nErrorCode)
{
	char szText[1024]="";
	switch (cs)
	{
	case XNetSession::CS_CONNECTING:
		strcpy(szText,"Connecting...");
		break;
	case XNetSession::CS_FAILED:
		strcpy(szText,"Failed");
		break;
	case XNetSession::CS_DISCONNECTED:
		strcpy(szText,"Disconnected");
		break;
	case XNetSession::CS_CONNECTED:
		strcpy(szText,"Connected");
		break;
	case XNetSession::CS_BUSY:
		strcpy(szText,"Busy");
		break;
	case XNetSession::CS_RECONNECTED:
		strcpy(szText,"Reconnected");
		break;
	case XNetSession::CS_RESTARTED:
		strcpy(szText,"Restarted");
		break;
	default:
		strcpy(szText,"Unkown");
		break;
	}
	LOGI("[CClient] OnXNetSessionConnectStatus %s, callback addr:%p",szText, m_pCallback);

	m_bConnected=(XNetSession::CS_CONNECTED==cs || XNetSession::CS_RECONNECTED==cs);

	if(m_pCallback)
	{
		m_pCallback->On_SessionConnectStatus((CONNECT_STATUS)cs);
	}
}

//����������Ӧ���յ������ݰ��ص�
void CAppClient::OnXNetClientReceivedFromServer(const char* pData,int nLen)
{
    
 if(m_isEncrypt)
    {
    int sum = (nLen/16);
     if (nLen%16>0)
    {
        sum=sum+1;
    }
    uint8_t outbuffer[nLen];
    memset(outbuffer,'\0',nLen);
    for(int i = 0; i < sum; ++i)
    {
        
        AES128_ECB_decrypt((const uint8_t*)pData+(i*16),kcrypt,outbuffer+(i*16));
    }
    printf("OnXNetClientReceivedFromServer nLen = %d,outbuffer==>%s \n\r",nLen,outbuffer);
	KCmdPacketEx rPacket((const char*)outbuffer,nLen+1);
	rPacket.SetDomain("");
	rPacket.SetSourceType(0);
	rPacket.SetSourceID("");
 m_pCallback->OnDispatchCmd(rPacket);
    }
    else
    {
       	KCmdPacketEx rPacket((const char*)pData,nLen);
        rPacket.SetDomain("");
        rPacket.SetSourceType(0);
        rPacket.SetSourceID("");
        m_pCallback->OnDispatchCmd(rPacket);
    }
}

