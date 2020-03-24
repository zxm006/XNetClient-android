//XNetClient.h
#ifndef __XNetCLIENT_H__
#define __XNetCLIENT_H__

#include "XNetSession.h"
#include "XNetDefine.h"

//网络应用服务
class XNetAPI XNetClientCallback
{
public:
	XNetClientCallback(void){};
	virtual~XNetClientCallback(void){};
public:
	//收到连接状态
	virtual void OnXNetSessionConnectStatus(XNetSession::CONNECT_STATUS /*cs*/,int /*nErrorCode=0*/){};

	//从其它网络应用收到的数据包回调
	virtual void OnXNetClientReceivedFromServer(const char* /*pData*/,int /*nLen*/){};
};

class XNetAPI XNetClient
{
public:
	XNetClient(void){};
	virtual~XNetClient(void){};
public:
	//连接服务器
	virtual int Connect(unsigned short nServerType,const char*cszIP,unsigned short usPort,const char*cszInterface)=0;
	//断开连接
	virtual void Disconnect(void)=0;

	//发送数据给指定的网络Server
	virtual int SendDataToServer(const char*pData,int nLen)=0;

	virtual unsigned int GetClientID(void)=0;

	//取本地地址
	virtual const char*GetLocalIP(void)=0;

	//取外网地址
	virtual const char*GetNATIP(void)=0;

	static XNetClient*Create(XNetClientCallback&rCallback);
};

#endif
