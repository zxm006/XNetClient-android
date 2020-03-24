#ifndef _AUDIO_RENDER_H__
#define _AUDIO_RENDER_H__


#include <pthread.h>

#include "opus.h"  
#include "opus_types.h"  
#include "opus_multistream.h"  

#include "XListPtr.h"
#include "CritSec.h"
#include "AutoLock.h"
#include "aiyou_voice_rpm.h"

#define PKG_BUF_LEN (1920)
typedef struct AUDIO_PKG
{
	AUDIO_PKG(){}
    
	AUDIO_PKG(void*pPacketData,int nPacketSize, int channelID, unsigned long timestamp, unsigned short seq, unsigned long recvedTime)
	{
		m_pPacketData = new char[nPacketSize];
		m_nPacketSize = nPacketSize;
		m_nChannelID = channelID;
		m_nTimeStamp= timestamp;
	       m_nRecvedTime = recvedTime;
		m_nSeq = seq;
        	m_hasData = true;
		memcpy(m_pPacketData,pPacketData,nPacketSize);
	}

	~AUDIO_PKG() {
		if (m_pPacketData != NULL) {
			delete[] (char *)m_pPacketData;
			m_pPacketData = NULL;
		}
		m_nPacketSize = 0;
		m_hasData = false;
	}

/*
    	void setData(void*pPacketData,int nPacketSize, int channelID, unsigned long timestamp, unsigned short seq)
	{
		m_nPacketSize = nPacketSize;
		m_nChannelID = channelID;
		m_nTimeStamp= timestamp;
		m_nSeq = seq;
        	m_hasData = true;
            	memset(m_pPacketData, 0, PKG_BUF_LEN);
		memcpy(m_pPacketData,pPacketData,nPacketSize);
	}*/
        
	void*	m_pPacketData;
	int		m_nPacketSize;
	int		m_nChannelID;
	unsigned long		m_nTimeStamp;
	unsigned short		m_nSeq;
	unsigned long		m_nRecvedTime;
      bool        m_hasData;
}AUDIO_PKG;


class IAudioRender
{
public:
	virtual ~IAudioRender(void){};

	virtual int  Connect(int nChannels,int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0)=0;
	
    virtual void ReleaseConnections(void)=0;

	virtual	int  Start() = 0;

	virtual void Stop() = 0;

	virtual int PushPacket(unsigned char*pData,int nLen) = 0;
};

IAudioRender *CreateAudioRender();


#define JB_CNT	(64)

//1秒内音频包个数
#define JB_AUDIO_PKG_CNT (100)

class CAudioPlayChanle
{
public:
	CAudioPlayChanle(void);
	virtual ~CAudioPlayChanle(void);
	virtual int  Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0);
	virtual void ReleaseConnections(void);

	virtual	int  Start();
	virtual	void Stop();
	
public:
	virtual void PushPacket(unsigned char*pData,int nLen, int chanelID, unsigned short seq, unsigned long timestamp);

	virtual void setAudioPcmPlayer(AiyouVoiceRpm *pVoiceRpm, int chanid);
	
private :
	int mSampleRate;
	int mBitRate;
	int mChannel;
	int mSamplesPerFrame;
	int mParamExt;
	
	IAudioRender *m_pAudioRender;
	OpusDecoder  *m_OpusDecoder;

	XListPtr			m_ListPcmData;
	KCritSec			m_kCritSec;
    	int				mChannelID;
	AiyouVoiceRpm      *m_pPcmPlayer;

	int decode2Pcm(char* input, int inlen, unsigned char* output, int outlen);

	KCritSec		m_kCritThread;
	pthread_t	m_thread;
	bool		m_bThreadStopped;

	static void* InitThreadProc(void*pObj)
	{
		((CAudioPlayChanle*)pObj)->ThreadProcMain();
		return 0;
	}

	virtual void ThreadProcMain(void);
	unsigned long TimeGetTimestamp();

	//jitter buffer
	struct timeval m_tmInit;
};


#endif
