#ifndef _C_VIDEO_CHAT_H_
#define _C_VIDEO_CHAT_H_

#include "ConnectServer.h"
#include "CritSec.h"
#include "AutoLock.h"
#include "uuDefine.h"
#include "OpenLocalUser.h"
#include "OpenRemoteUser.h"
#include "AudioCapture.h"
#include "AudioRender.h"

#include <list>
#include <vector>

#include "CritSec.h"
#define SIZE_AUDIO_FRAME (2)

typedef enum {
    MS_CREAT_OK=0,		//正在连接
    MS_CREAT_FAILED,
    MS_JOIN_OK,
    MS_JOIN_FAILED,
    MS_EXIT_OK,
    MS_DESTROYED_OK,
} CONF_STATUS;

#define NAME_BUF (256)

class IVideoChatCallback
{
public:
    virtual void  MtgStatuse(int status, const char *usename, const char *headurl, const char *nickname, const char *jingdu, const char *weidu, const char * mtgTm)=0;
    virtual void  UserVideoView(const char *userid)=0;
    virtual void  UserMediaStatus(std::string strName,bool isvideo,bool isopen) = 0;
    virtual void  MtgInfoReg(MTG_CMD mtgCmd,bool result,std::string &info) =0;
    virtual void  MtgInfo(MTG_CMD mtgCmd,std::string username ,std::string &info) =0;
    virtual void  MtgInvite(std::string username ,std::string mtgId) =0;
    virtual void  MtgBeKicked(std::string username ,std::string mtgId) =0;
    virtual void  MtgJoin(CLIENTUSERINFOLIST &userinfo) =0;
    virtual void  MtgExit(CLIENTUSERINFOLIST &userinfo) =0;
    virtual void  MtgUserList(CLIENTUSERINFOLIST_MAP& UserInfoList) =0;
    
    virtual void IConnectStatusCallback(CONNECT_STATUS cs) = 0;
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList) = 0;
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID) = 0;
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName) = 0;
    virtual void INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen) = 0;
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen) = 0;
 
};


class CVideoChat
:public CmdTCPClientCallback,ISendCmdServer,IAudioOpusDataNotify
{
public:
    CVideoChat();
    ~CVideoChat(void);
public:
    void LoginServer(const char* szIP,const char* szName , const char* headUrl, 
                     const char* nickName,double latitude,double longitude,const char* szGameId,const char* szGameServerId,const char* szRoomId,const char* szGroupId="",const char* szexpand= "",bool isencrypt = true);
    int logoutServer();
    
    void setNetWorkCallback(IVideoChatCallback*netWorkCallback);

    void  OpenRemoteUserVideo(const char* username, void* pVideoWnd);
    void  closeRemoteUserVideo(const char* username);
    
    void OpenRemoteUserAudio(const char* username);
    //void closeRemoteUserAudio(const char* username);

	int startReceiveRemoteAudio(OpenRemoteUser* remoteUser);
	void stopReceiveRemoteAudio(OpenRemoteUser* remoteUser);
	int remoteAudioSetPause(const char* username, bool bPause);
	int localAudioSetPause(bool bPause);
	int localCameraSwitch();
	int localNetworkSwitched();
    
     int SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen);
    void resetLocalVideo();
    
    void  openLocalVideo(void *pWnd);
    void  closeLocalVideo();
    
    void disconnect();

    void creatMtg(std::string mtgid, bool openCamera,MTG_TYPE mtgType);
    void joinMtg(std::string mtgid, bool openCamera);
    void exitMtg();
    void destroyMtg();
    
    void setAudioSession();
    int  mtgInviteUser(const char* mtgId, const char* iuserId);
    
    virtual void SendAudioData(char*pData, int nLen);
    virtual void OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen);
    void setAudioCapParam(int iChannels, int iSampleRate, int iBitRate);
    void SetVideoCapParam(int width, int height, int bitrate, int frameRate);
    
private:
    virtual void OnDispatchCmd(KCmdPacketEx& pPacket);
    
    void OnDispatchVideoCall(KCmdPacketEx& pPacket,string videocmd);
   
private:
    int  SendMtgCmd(MTG_CMD mtgCmd, std::string mtgid);
    
    void openMediaSender();
    void closeMediaSender();
    
    void  openLocalMedia(bool isCapScreen);
    
    void StopAllRemoteMedia();
    void openLocalAudio();
    void closeLocalAudio();
    int getUserList();

    virtual int SendData(KCmdPacketEx& pPacket);
    int sendloginServer(const char* szName, const char* szPassword, bool isConfDoing);
    int ConnectServer(const char* szIP,unsigned int nPort ,bool isEncrypt);
    
    int  SendUserMediaStatus(bool isvideo, bool isopen );
    
    virtual void On_SessionConnectStatus(CONNECT_STATUS cs);
    
    virtual  int SendAllUserData(const char* pData, unsigned long nLen);
    
    
    
    void OnDispatchMtgCmd(KCmdPacketEx& pPacket);
    void OnDispatchMtgCmdReg(KCmdPacketEx& pPacket);
    void MtgInfoReg(MTG_CMD mtgCmd,bool result,std::string &info);
    void MtgInfo(MTG_CMD mtgCmd,std::string username ,std::string &info);
    
private:
    
    virtual std::string GetLocalIP();
    virtual std::string GetNATIP();
    std::string m_serverip;
    std::string m_username;
    std::string m_szexpand;
    std::string m_strAddress;
    std::string m_userGroupId;
    std::string m_mtgID;
    std::string m_headUrl;
    std::string m_nickName;
    double m_latitude;
    double m_longitude;
    MTG_TYPE m_mtgType;
    
    void *m_touchMoveView;
    
    unsigned long  m_audioID;
    unsigned long  m_uUserID;
    bool m_openCamera;
    
    OpenLocalUser * m_OpenLocalUser;
    std::map<unsigned long, class OpenRemoteUser*> m_pOpenRemoteUser_map;
     CConnectServer             m_ConnectServer;
    IVideoChatCallback*          m_pINetWorkCallback;
    CLIENTUSERINFOLIST_MAP     m_UserInfoList;
    
    KCritSec m_mKCritSec;

	int			mVWidth;
	int			mVHeight;
	int			mVBitRate;
	int			mVFrameRate;

private:
	CAudioCapChanle *m_pAudioCap;	//音频采集编码类
	int mChannels;					//音频采集通道数
	int mSampleRate;				//音频采样率8000,16000,24000,48000 and so on
	int mBitRate;					//opus编码速率,Rates from 500 to 512000 bits per second are meaningful, as well as the
	//CAudioPlayChanle 	*m_pAudioPlay;		//音频解码播放类
	AiyouVoiceRpm      *m_pVoiceRpm;
 
};
#endif
