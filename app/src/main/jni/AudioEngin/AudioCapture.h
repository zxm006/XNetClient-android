#ifndef _AUDIO_CAPTURE_H__
#define _AUDIO_CAPTURE_H__


#include "opus.h"  
#include "opus_types.h"  
#include "opus_multistream.h"  
  
//#include "AudioRender.h"

#include "XListPtr.h"
#include "CritSec.h"
#include "AutoLock.h"


#include <pthread.h>
#include "aiyou_voice_rpm.h"



class IAudioCaptureNotify
{
public:
	IAudioCaptureNotify(void){};
	virtual ~IAudioCaptureNotify(void){};
public:
	virtual void OnAudioCaptureNotifyOutput(unsigned char*pPcm,int nLen)=0;
};

class IAudioOpusDataNotify
{
public:
	IAudioOpusDataNotify(void){};
	virtual ~IAudioOpusDataNotify(void){};
public:
	virtual void OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen)=0;
};

class IAudioVolumeNotify
{
public:
	IAudioVolumeNotify(void){};
	virtual ~IAudioVolumeNotify(void){};
public:
	virtual void OnAudioVolumeOutput(int volume)=0;
};



class IAudioCapture
{
public:
	IAudioCapture(void){};
	virtual ~IAudioCapture(void){};
	virtual int  Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0)=0;
	virtual void ReleaseConnections(void)=0;

	virtual	int  Start() = 0;
	virtual void Stop() = 0;

	virtual	void SetSampleRate(int nRate) = 0;
	virtual void SetSamplesPerFrame(int nSamplesPerFrame) = 0;
	virtual	void SetBitrate(int nBitrate) = 0;
};


IAudioCapture* CreateAudioCapture(IAudioCaptureNotify& rIAudioCaptureNotify);

class CAudioCapChanle: 
	public IAudioCaptureNotify, 
	public webrtc::CAyVoiceCapCb
{
public:
	CAudioCapChanle(bool isUseAec, int sampleRate, IAudioOpusDataNotify *pOpusDataCb = NULL);
	virtual ~CAudioCapChanle(void);
	virtual int  Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0);
	virtual void ReleaseConnections(void);

	virtual	int  Start();
	virtual void Stop();

	virtual void setPause(bool isPause);
public:
	virtual void OnAudioCaptureNotifyOutput(unsigned char*pPcm,int nLen);
	virtual int32_t CapDataCallback(char *pData, int len, int sampleRate, int bits, int channelNumb);

	void setVolumeParam(bool enable, long minSec, IAudioVolumeNotify *pVolumeNtf);
	
private :
	int mSampleRate;
	int mBitRate;
	int mChannels;
	
	OpusEncoder   *m_OpusEncoder;

	int encode2Opus(char* input, int inlen, unsigned char* output, int& outlen);

	IAudioOpusDataNotify *m_pOpusDataCb;
	IAudioVolumeNotify *mpVolumeNtf;			//音量回调
    	bool					mVolueEnable;		//是否计算音量
	long					mLastVolumeTm;		//上次计算音量的时间
    	long					mIntervalVolumeTm;	//计算音量的时间间隔
    
	bool	mbIsPaused;

	XListPtr		m_ListPcmData;
	KCritSec		m_kCritSec;

	pthread_t	m_thread;
	bool			m_bThreadStopped;

	static void* InitThreadProc(void*pObj)
	{
		((CAudioCapChanle*)pObj)->ThreadProcMain();
		return 0;
	}

	virtual void ThreadProcMain(void);

	int getVolueByPcm(const unsigned char* ptr, int size);
    	long getCurrentTm();
};

#endif
