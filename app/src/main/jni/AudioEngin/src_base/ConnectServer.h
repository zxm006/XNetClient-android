#ifndef _UU_CMD_TCPCLIENT_H_
#define _UU_CMD_TCPCLIENT_H_

#include "CmdPacket.h"
#include "XNet/XNetCore.h"
#include "XNet/XNetSetting.h"
#include "XNet/XNetClient.h"
#include "XNet/XNetMediaSender.h"
#include "XNet/XNetMediaReceiver.h"

typedef enum {
	CS_CONNECTING=0,		//��������
	CS_FAILED,				//�޷�����
	CS_CONNECTED,			//�Ѿ�����
	CS_DISCONNECTED,		//�Ͽ�����
	CS_BUSY,				//����æ(�ѶϿ�������)
	CS_RECONNECTED,			//�����ɹ�
	CS_IDLE,				//����
	CS_RESTARTED,			//���������ˡ����ӶϿ��ˣ������������������ˣ����ǻ���һ�������ӡ�
	CS_LOGINED,             //��½�������ɹ�
	CS_LOGINFAILED,         //��½������ʧ��
	CS_LOGOUT,			//�˳�������
	CS_RELOGIN,		//��������
} CONNECT_STATUS;

#define IP_SIZE	16

class CAppClient;

class CmdTCPClientCallback
{
public:
	//�������ӷ����״̬
	virtual void On_SessionConnectStatus(CONNECT_STATUS cs) = 0;
	//���ط��������
	virtual void OnDispatchCmd(KCmdPacketEx& pPacket) = 0;
};

class ISendCmdServer
{
public:
	virtual int SendData(KCmdPacketEx& pPacket) = 0;
	virtual std::string GetLocalIP() = 0;
	virtual std::string GetNATIP() = 0;
    virtual void SendAudioData(char*pData, int nLen)=0;
};

class CConnectServer
	:public ISendCmdServer
{
public:
	CConnectServer();
	~CConnectServer();
public:
	int Start(const char* ip,unsigned port,bool isEncrypt, CmdTCPClientCallback* pCallback);
	bool Stop();
	virtual int SendData(KCmdPacketEx& pPacket);
	virtual std::string GetLocalIP();
	virtual std::string GetNATIP();
	virtual void SendAudioData(char*pData, int nLen);

	int getIpByDomain(const char *domain, char *ip);
	int checkIsDomain(const char *pAddr);
private:
	CAppClient* m_pAppClient;
};


#endif
