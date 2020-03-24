#ifndef _UU_CAPP_CLIENT_H_
#define _UU_CAPP_CLIENT_H_

#include "ConnectServer.h"

class CAppClient
	: public XNetClientCallback
{
public:
	CAppClient(void);
	virtual~CAppClient(void);
public:
	int Open(const char*cszIP,unsigned short usPort,bool isEncrypt);
	void Close(void);
	void SetFunPointer(CmdTCPClientCallback* pCallback);
	std::string GetLocalIP();
	std::string GetNATIP();
public:
	//�յ�����״̬
	virtual void OnXNetSessionConnectStatus(XNetSession::CONNECT_STATUS cs,int nErrorCode=0);

	//����������Ӧ���յ������ݰ��ص�
	virtual void OnXNetClientReceivedFromServer(const char* pData,int nLen);

	virtual bool IsConnnected(void);

	virtual int SendData(const char*pData,int nLen);

protected:
	XNetClient*	m_pClient;
	bool			m_bConnected;
private:
	CmdTCPClientCallback* m_pCallback;
      bool m_isEncrypt;
};

#endif

