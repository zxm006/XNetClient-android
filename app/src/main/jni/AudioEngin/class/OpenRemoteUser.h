//
//  OpenRemoteUser.h
//  VideoSession
//
//  Created by wang guijun on 13-2-7.


#ifndef OPENREMOTEUSER_H
#define OPENREMOTEUSER_H

#include "XNet/XNetMediaReceiver.h"
#include <string>
//#import "AudioPlay.h"
#include "AudioRender.h"
#include "uuIVideo.h"


class OpenRemoteUser
: public XNetMediaReceiverCallback
{
public:
    OpenRemoteUser(bool isVideoCalling);
    ~OpenRemoteUser();
public:
    bool ConnectMediaServer(std::string strServerIp, unsigned short nPort);
    void ReleaseMediaSever();

    void  getAudioData(unsigned char* pdata,long len);
    
    bool OpenAudioReceive(unsigned long ulPeerUserAudioId);
    bool CloseAudioReceive();
    
    bool OpenVideoReceive(unsigned long ulPeerUserVideoId);
    bool CloseVideoReceive();
 
       void SetVideoWindow(void* pVideoWindow);
	void resetVideoWindow(void* pVideoWindow);
      bool      m_isVideoCalling;//�Խ�

	void setAudioChanID(int id);
	int  getAudioChanID();
    	int  reopenMediaRecver();
    
public:
	virtual void OnXNetMediaReceiverCallbackAudioPacket(unsigned char*pData,int nLen);
	virtual void OnXNetMediaReceiverCallbackVideoPacket(unsigned char*pData,int nLen);
 private:
 	bool CreateScreenReceiver();
 	void ReleaseReceiver();
 
    
    //  AudioPlay *m_AudioPlay;
private:
	XNetMediaReceiver*	m_pMediaReceiver;
    	unsigned long           m_ulPeerVidoID ;
    //  liveFFmpegdecode *  m_liveFFmpegdecode;
	unsigned long           m_ulPeerAudioID ;			//AudioID
	std::string             m_peer_nodeid;
	std::string             m_peer_nataddr;
	std::string             m_peer_locaddr;
	int                     m_peer_locport;
	std::string	            m_peer_mcuid;
	std::string	            m_peer_mcuaddr;
	int		                m_peer_mcuport;
private:
    std::string             m_MediaServerIp;
	unsigned short          m_nPort;
    unsigned char*          m_data;
    unsigned long			m_length;

private:
	static bool					ms_bPlayAudio;		//�Ƿ񲥷���Ƶ,ȫ�ֵı��
	CAudioPlayChanle*	ms_pAudioPlay;		//��Ƶ���벥����
	bool					mbPauseAudio;		//���ͨ�����Ƿ񲥷���Ƶ
	
public:
	void setAudioPlayer(CAudioPlayChanle 	*pAudioPlay);
    	CAudioPlayChanle 	* getAudioPlayer();
	static void setAudioPlayFlag(bool bPlay);
	void setAudioMute(bool bPause);

private:
//    VIDEO_IVideoPlayChan*  m_pVideoPlayChan;
	int					m_chanID;
};

#endif 
