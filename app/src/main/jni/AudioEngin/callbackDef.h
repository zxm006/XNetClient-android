#ifndef _CALLBACK_DEF_H__
#define _CALLBACK_DEF_H__

/************通话接口*******************/
class CNetWork
:public INetWorkCallback
{
public:
	virtual void IConnectStatusCallback(CONNECT_STATUS cs);
	virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList);
	virtual void INetReceiveUserLogin(CLIENTUSERINFOLIST *usrinfo);
	virtual void INetReceiveUserLogOut(CLIENTUSERINFOLIST *usrinfo);
	virtual void INetReceiveData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
	virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
	virtual void INetAudioStatus(unsigned long uPeerUserID, bool isRoom, AUDIO_SEND_STATUS AudioStatus, std::string speaker);
	virtual void INetVideoCall(unsigned long uPeerUserID,bool videocall, std::string strName);
	virtual void INetCallConnected();
	virtual void INetCallDisConn(int iReason, const char*pInfo);
	virtual void INetVolumeChanged(int volume);
};


/************连麦接口*******************/
class CLinkMicCallback
:public ILinkMicCallback
{
public:
    virtual void IConnectStatusCallback(LINKMIC_STATUS cs) ;
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList) ;
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID);
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName);
    virtual void INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen);
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
 
};

/****************会议接口*******************/
class CConfCallback
:public IVideoChatCallback
{
public:
    virtual void  MtgStatuse(int status, const char *usename, const char *headurl, const char *nickname, const char *jingdu, const char *weidu, const char* mtgTm);
    virtual void  UserVideoView(const char *userid);
    virtual void  UserMediaStatus(std::string strName,bool isvideo,bool isopen);
    virtual void  MtgInfoReg(MTG_CMD mtgCmd,bool result,std::string &info);
    virtual void  MtgInfo(MTG_CMD mtgCmd,std::string username ,std::string &info);
    virtual void  MtgInvite(std::string username ,std::string mtgId);
    virtual void  MtgBeKicked(std::string username ,std::string mtgId);
    virtual void  MtgJoin(CLIENTUSERINFOLIST &userinfo);
    virtual void  MtgExit(CLIENTUSERINFOLIST &userinfo);
    virtual void  MtgUserList(CLIENTUSERINFOLIST_MAP& UserInfoList);
    
    virtual void IConnectStatusCallback(CONNECT_STATUS cs);
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList);
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID);
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName);
    virtual void INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen);
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
};




#endif
