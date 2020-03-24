#ifndef _C_LINK_MIC_MANAGE_H_
#define _C_LINK_MIC_MANAGE_H_

#include <list>
#include <vector>
#include <pthread.h>

#include "ConnectServer.h"
#include "CritSec.h"
#include "AutoLock.h"
#include "uuDefine.h"
#include "OpenLocalUser.h"
#include "ConnectServer.h"
#include "AudioCapture.h"
#include "AudioRender.h"

#include "CritSec.h"

#define SIZE_AUDIO_FRAME (2)

#include "uuDefine.h"

using std::vector;

typedef CONNECT_STATUS LINKMIC_STATUS;

class ILinkMicCallback
{
public:
    virtual void IConnectStatusCallback(LINKMIC_STATUS cs) = 0;
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList) = 0;
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID) = 0;
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName) = 0;
    virtual void INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen) = 0;
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen) = 0;
 
};


class CLinkMicManage
:public CmdTCPClientCallback,ISendCmdServer,IAudioOpusDataNotify
{
public:
    CLinkMicManage();
    ~CLinkMicManage(void);
public:
    void  loginLinkMicServer(const char* szIP,const char* szName ,const char* szGameId,const char* szGameServerId,const char* szRoomId,const char* szGroupId="",const char* szexpand= "",bool isencrypt = true,bool iscapscreen = NO,bool isAnchor = NO);

    void setNetWorkCallback(ILinkMicCallback*netWorkCallback);
     int getUserList();
     int logoutServer();
    void disconnect();
    virtual void SendAudioData(char*pData, int nLen);
    
    void  OpenRemoteUserVideo(const char* username, void *peerVideoView);
    void  resetRemoteUserVideo(const char* username, void *peerVideoView);
    void  closeRemoteUserVideo(const char* username);

    void  openLocalVideo(void *pVideoView);
    void  closeLocalVideo();
    
 
    virtual int SendData(KCmdPacketEx& pPacket);
   
    void OnDispatchVideoCall(KCmdPacketEx& pPacket,string videocmd);
     int SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen);
    void resetLocalVideo();
private:
    
    void openVideo();
    void closeVideo();
    
    virtual void On_SessionConnectStatus(LINKMIC_STATUS cs);
    virtual  int SendAllUserData(const char* pData, unsigned long nLen);
    
    void SetRoomList(const char *roomUserList);
     int SendLinkMic(const char* pData, unsigned long nLen,bool isinroom);
 
    virtual std::string GetLocalIP();
    virtual std::string GetNATIP();
    
    int LoginServer(const char* szName, const char* szPassword);
    int ConnectServer(const char* szIP,unsigned int nPort ,bool isEncrypt);

private:
    virtual void OnDispatchCmd(KCmdPacketEx& pPacket);
private:
    
    std::string m_serverip;
    std::string m_username;
    std::string m_szexpand;
    std::string m_strAddress;
    std::string m_userGroupId;
    std::vector<void *> m_peertouchMoveViews;
    void *mlocalView;
    
    unsigned long  m_audioID;
    unsigned long  m_uUserID;
  

    
    OpenLocalUser * m_OpenLocalUser;
    std::map<unsigned long, class OpenRemoteUser*> m_pOpenRemoteUser_map;
     CConnectServer             m_ConnectServer;
    
    ILinkMicCallback*          m_pINetWorkCallback;
    CLIENTUSERINFOLIST_MAP     m_UserInfoList;
    
    KCritSec m_mKCritSec;
    bool m_isAnchor;
    void setupSession();
    void openMediaSender(bool isCapScreen);
    void closeMediaSender();
    
    void openLocalAudio();
    void closeLocalAudio();
    int  startReceiveMedia(OpenRemoteUser* remoteUser);
    void  stopReceiveMedia(OpenRemoteUser* remoteUser);
    void  stopAudioDecoderIfNeed();
  
    void StopAllRemoteMedia();
    bool m_iscapscreen;

private:
	CAudioCapChanle *m_pAudioCap;	//音频编码类
	int mChannels;					//音频采集通道数
	int mSampleRate;				//音频采样率8000,16000,24000,48000 and so on
	int mBitRate;					//opus编码速率,Rates from 500 to 512000 bits per second are meaningful, as well as the
	
	AiyouVoiceRpm      *m_pVoiceRpm;		//音频采集播放类

	bool				m_bIsLinkUseVideo;	//连麦是否开启视频,715版本先不用视频连麦
public:
	virtual void OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen);
	void setAudioCapParam(int iChannels, int iSampleRate, int iBitRate);
	
};
#endif
