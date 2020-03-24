#ifndef _UU_CAPP_MANAGE_H_
#define _UU_CAPP_MANAGE_H_

#include "ConnectServer.h"
#include "CritSec.h"
#include "AutoLock.h"
#include "uuDefine.h"
#include "OpenLocalUser.h"
#include <list>
#include <vector>
#include "AudioCapture.h"
#include "AudioRender.h"

#include <pthread.h>
#include "CritSec.h"

#include "aiyou_voice_rpm.h"
#include "OpenRemoteUser.h"

#define SIZE_AUDIO_FRAME (2)

#ifndef BOOL
#define BOOL bool
#endif

typedef int (*AIYOUVOICECALLBACK)(int msgcode,bool issucceed,const char * funtype,const char *status,const char *username,const char * inRoom);

typedef struct _UserInfo{
    _UserInfo()
    :uUserID(0)
    ,strUserName("")
    {
    }
    ~_UserInfo(){}
    
    unsigned long uUserID;
    std::string   strUserName;
    
}UserInfo;

/*
typedef enum
{
    AUDIO_SEND_ENABLE=0,	//可以发送
    AUDIO_SEND_DISABLE,	 //不能发送
    AUDIO_SEND_SENDING		 //正在发送
    
} AUDIO_SEND_STATUS;*/


class INetWorkCallback
{
public:
    virtual void IConnectStatusCallback(CONNECT_STATUS cs) = 0;
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList) = 0;
    virtual void INetReceiveUserLogin(CLIENTUSERINFOLIST *usrinfo) = 0;
    virtual void INetReceiveUserLogOut(CLIENTUSERINFOLIST *usrinfo) = 0;
    virtual void INetReceiveData(unsigned long uPeerUserID, const char* pData, unsigned long nLen) = 0;
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen) = 0;
    virtual void INetAudioStatus(unsigned long uPeerUserID,bool isRoom,AUDIO_SEND_STATUS AudioStatus, std::string strName) = 0;
    virtual void INetVideoCall(unsigned long uPeerUserID,bool videocall, std::string strName) = 0;
    virtual void INetCallConnected() = 0;
	virtual void INetCallDisConn(int iReason, const char*pInfo) = 0;
	virtual void INetVolumeChanged(int volume) = 0;
};

class CVoiceManage
:public CmdTCPClientCallback,ISendCmdServer,IAudioOpusDataNotify,IAudioVolumeNotify
{
public:
    CVoiceManage(AIYOUVOICECALLBACK  Callback);
    ~CVoiceManage(void);
public:
    void  loginVoiceServer(const char* szIP,const char* szName ,const char* szGameId,const char* szGameServerId,
        					const char* szRoomId,const char* szGroupId="",BOOL listenInRoom = true,const char* szexpand= "", bool isencrypt = true,
        					const char* headUrl="",const char* nickName="",double latitude=0,double longitude=0);
    
    void setNetWorkCallback(INetWorkCallback*netWorkCallback);
    int  getUserList();
    int  logoutServer();
    void disconnect();
    void SendAudioData(char*pData, int nLen);
    
    AUDIO_SEND_STATUS  getAudioStatus();

    void  enableUpVolume(BOOL isspeakinroom);
    void  startSendAudio(BOOL isspeakinroom);
    void  stopSendAudio();
    void  StartReceiveMedia(OpenRemoteUser* openRemoteUser, bool isptt);
    void  StopReceiveMediaAndroid(OpenRemoteUser* openRemoteUser);
    
    virtual int SendData(KCmdPacketEx& pPacket);
   
    void  OnDispatchVideoCall(KCmdPacketEx& pPacket,string videocmd);
    void invite(bool isVideo,const char*username,void* localVideo,void* peerVideo);
    void accept(void* localVideo,void* peerVideo);
    void cancel();
    void refuse();
    void hangup();
    void *mlocalView;
    void *mpeerView;
    int SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen);
    
private:
    virtual void On_SessionConnectStatus(CONNECT_STATUS cs);
     virtual int SendAllUserData(const char* pData, unsigned long nLen);
    
    void SetRoomList(const char *roomUserList);
    int SendVoice(const char* pData, unsigned long nLen,bool isinroom);
    
    virtual std::string GetLocalIP();
    virtual std::string GetNATIP();
    
    int LoginServer(const char* szName, const char* szPassword);
    int ConnectServer(const char* szIP,unsigned int nPort ,bool isEncrypt);
    
private:
    virtual void OnDispatchCmd(KCmdPacketEx& pPacket);
private:
    
    std::string   m_serverip;
    std::string   m_username;
    std::string   m_szexpand;
    std::string m_headUrl;
    std::string m_nickName;
    double m_latitude;
    double m_longitude;
    
    std::string   m_RoomSpeakerName;
    std::string   m_GroupSpeakerName;
    
    std::string   m_strAddress;
    std::string   m_userGroupId;
    
    unsigned long  m_audioID;
    unsigned long  m_uUserID;
     bool      m_isVideoCalling;//对讲
     bool      m_isEnableVolue;	//实时音量变化
   
    //是否在房间内说话，为内部控制
    BOOL m_isSpeakinRoom;
   
    //是否收听房间内的人说话，为外部设置
    BOOL mlistenInRoom ;
    
    AUDIO_SEND_STATUS  m_ROOMaudioStatus;
    AUDIO_SEND_STATUS  m_GROUPaudioStatus;
    
    OpenLocalUser * m_OpenLocalUser;
    std::map<unsigned long, class OpenRemoteUser*> m_pOpenRemoteUser_map;
    AIYOUVOICECALLBACK m_callback;
    CConnectServer             m_ConnectServer;
    
    INetWorkCallback*          m_pINetWorkCallback;
    CLIENTUSERINFOLIST_MAP     m_UserInfoList;
    
    KCritSec m_mKCritSec;
    
    void  openMediaSender(bool isVideoCalling);
    void  closeMediaSender(bool isVideoCalling);
    
    void  openLocalAudio(bool isVideoCalling);
    void  closeLocalAudio(bool isVideoCalling);
    
    void  openVideo();
    void  closeVideo();

    void  StopAllRemoteMedia(bool isVideoCalling);
    void  sendVideoCallMsg(unsigned long userid, const char*msg);
    
    void inviteVideoCall(unsigned long userid,bool hasVideo);
    
    CLIENTUSERINFOLIST m_videoCallUserinfo;
    bool mhasVideo;
  
private:
	CAudioCapChanle *m_pAudioCap;	//音频采集编码类
	int mChannels;					//音频采集通道数
	int mSampleRate;				//音频采样率8000,16000,24000,48000 and so on
	int mBitRate;					//opus编码速率,Rates from 500 to 512000 bits per second are meaningful, as well as the
	CAudioPlayChanle 	*m_pAudioPlay;		//音频解码播放类
	AiyouVoiceRpm      *m_pVoiceRpm;
public:
	virtual void OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen);
	void 	setAudioCapParam(int iChannels, int iSampleRate, int iBitRate);
	virtual void OnAudioVolumeOutput(int volume);

	static int SetAndroidObjects(void* javaVM, void* env, void* context);
private:
	int 	cb2UiCallback(int msgcode, bool issucceed, const char *funtype, const char *status, const char *username, const char * inRoom);
};
#endif
