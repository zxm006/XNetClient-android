//
//  OpenLocalUser.h
//  VideoSession
//
//  Created by wang guijun on 13-2-7.
//  Copyright (c) 2013. All rights reserved.
//

#ifndef OPENLOCALUSER_H
#define OPENLOCALUSER_H

#include <string>
#include "XNet/XNetMediaSender.h"
#include "uuIVideo.h"
#include "AiYouVideoLibrary.h"
#include "AutoLock.h"



class OpenLocalUser : public XNetMediaSenderCallback, /*public VideoCaptureDataCallBack, */public VIDEO_ICapChanCallback
{
public:
    OpenLocalUser();
    ~OpenLocalUser();
public:
    bool ConnectMediaServer(std::string strServerIp, unsigned short nPort);
    void ReleaseMediaSever();

    bool OpenAudioSend(unsigned int ulUserAudioId);
    bool CloseAudioSend();
	bool SendAudioData(char*pData, int nLen);
    
    bool  OpenVideoSend(unsigned int ulUserVideoId);
    bool  CloseVideoSend();
    
  
    void openLocalVideo(void *localvideo);
    
    void closeLocalVideo();
    void  resetLocalVideo();
    void  switchLocalVideo();
    
    bool SendVideoData(unsigned char*pData, int nLen, bool bKeyFrame, unsigned long ulTimestamp, int nWidth, int nHeight);
    bool reopenMediaSender();
    
    //virtual void On_MediaReceiverCallbackVideo(unsigned char*pData,int nLen, bool bKeyFrame, int nWidth, int nHeight);
	
private:
	bool CreateSender();
	void ReleaseSender();
	void ProcessVideoFrame(unsigned char*pData, int nLen, bool bKeyFrame, unsigned long ulTimestamp, int nWidth, int nHeight);
	void ProcessAudioFrame(char*pData, int nLen, unsigned long ulTimestamp);
	unsigned long TimeGetTimestamp();
	
private:
    	KCritSec m_mKCritSec;
	bool m_bGotKeyFrameMain;
	char* m_pVideoPacket;
	int	m_nFrameBufferLength;
	unsigned short m_usVideoSequence;

	char* m_pAudioPacket;
	int m_nAudioFrameBufferLength;
	unsigned short m_usAudioSequence;
	unsigned long m_ulLastFrameTimestamp;
	unsigned long m_ulLastPacketTimestamp;
	struct timeval m_tmInit;
    
private:
	XNetMediaSender*		m_pMediaSender;
	unsigned int           m_ulLocalAudioID ;			//AudioID
	unsigned int           m_ulLocalVideoID ;
	std::string             m_MediaServerIp;
	unsigned short          m_nPort;
	
private:
	int			m_nWidth;
	int			m_nHeight;
	int			m_nBitRate;
	int			m_nFrameRate;
	
//    VIDEO_IVideoCapChan*  m_pVideoCapChan;
	virtual void OnVIDEO_CapChanCallback(unsigned char*pData,int nLen,bool& bKeyFrame,int nWidth,int nHeight,int nHeaderLen);
public:
	void SetVideoCapParam(int nWidth = 320, int nHeight = 240, int nBitRate = 128, int nFrameRate=30);
};

#endif
