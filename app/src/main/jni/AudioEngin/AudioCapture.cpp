//#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "log.h"

#include "AudioCapture.h"
#include "android_opensles_input.h"

#include <time.h>
#include "define.h"


IAudioCapture* CreateAudioCapture(IAudioCaptureNotify& rIAudioCaptureNotify)
{
	return new AndroidOpenSLESInput(rIAudioCaptureNotify);
}

static char arAudioFileName[256];
void saveData2File(char *pData, int len)
{
	FILE *pFile = NULL;
	pFile = fopen(arAudioFileName, "a");
	if(pFile)
	{
		fwrite(pData , 1, len, pFile );    
		fclose(pFile); 
		pFile = NULL;
		//LOGE("save 2 file ok:%s", arAudioFileName); 
	}
	else
	{
		LOGE("open file failed:%s", arAudioFileName); 
	}
}

void CAudioCapChanle::OnAudioCaptureNotifyOutput(unsigned char*pPcm, int nLen)
{
#if 0
	//LOGE("OnAudioCaptureNotifyOutput in, data len:%d", nLen); 
	if(mbIsPaused)
	{
		return;
	}

	KAutoLock l(m_kCritSec);
	if(m_ListPcmData.size() > 200)
	{
		LOGE("pcm data list > 200, will clear this list"); 
		PPACKET pDel = NULL;
		while(m_ListPcmData.size() > 0) 
		{
			pDel=(PPACKET)m_ListPcmData.front();
			m_ListPcmData.pop_front();
			delete pDel;
		}
		return;	
	}
	PPACKET pPacket = new PACKET(pPcm, nLen);
	m_ListPcmData.push_back((void*)pPacket);
#endif	
}

int32_t CAudioCapChanle::CapDataCallback(char *pData, int len, int sampleRate, int bits, int channelNumb)
{
	//LOGI("webrtc CapData Callback in, len:%d, pause flag:%d, param:%d,%d,%d", len, mbIsPaused, sampleRate, bits, channelNumb); 
	if(mbIsPaused)
	{
		return 0;
	}

	KAutoLock l(m_kCritSec);
	if(m_ListPcmData.size() > 200)
	{
		LOGE("pcm data list > 200, will clear this list"); 
		PPACKET pDel = NULL;
		while(m_ListPcmData.size() > 0) 
		{
			pDel=(PPACKET)m_ListPcmData.front();
			m_ListPcmData.pop_front();
			delete pDel;
		}
		return 0;	
	}
	PPACKET pPacket = new PACKET(pData, len);
	m_ListPcmData.push_back((void*)pPacket);
}

void CAudioCapChanle::setVolumeParam(bool enable, long minSec, IAudioVolumeNotify *pVolumeNtf)
{
	LOGE("set Volume Param:%d,%d", enable, minSec); 
	mVolueEnable = enable;
	mIntervalVolumeTm = minSec;
	mpVolumeNtf = pVolumeNtf;
}


CAudioCapChanle::CAudioCapChanle(bool isUseAec, int sampleRate, IAudioOpusDataNotify *pOpusDataCb)
{
	mpVolumeNtf = NULL;
	mLastVolumeTm = 0;
	mIntervalVolumeTm = 300;
	mVolueEnable = false;
    
	mAyVcIsUseAec = isUseAec;
	mAyVcSampleRate = sampleRate;
	
	m_pOpusDataCb = pOpusDataCb;
	m_bThreadStopped = true;

	mbIsPaused = true;
	
	time_t now_time;
	now_time = time(NULL);
	memset(arAudioFileName, 0, sizeof(arAudioFileName));
	sprintf(arAudioFileName, "/mnt/sdcard/b_conf_send_opus_%ld.pcm", now_time);
}

CAudioCapChanle::~CAudioCapChanle()
{

}

int CAudioCapChanle::Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam)
{
	mSampleRate = nSampleRate;
	mBitRate = nBitrate;
	mChannels = nChannel;

	//encoder
	int err = 0;  
	opus_int32 skip = 0;  

	m_OpusEncoder = opus_encoder_create(mSampleRate, nChannel, OPUS_APPLICATION_VOIP, &err);  
	if (err != OPUS_OK) 
	{  
		LOGE("cannnot create opus encoder: %s\n", opus_strerror(err));  
		m_OpusEncoder = NULL;  
		return -1;  
	}  

	nBitrate = nSampleRate;
	LOGE("opus create param:%d,%d,%d", nSampleRate, nBitrate, nChannel);  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_BITRATE(nSampleRate));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_VBR(1));
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_COMPLEXITY(1));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_INBAND_FEC(0));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_FORCE_CHANNELS(OPUS_AUTO));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_DTX(0));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_PACKET_LOSS_PERC(0));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_GET_LOOKAHEAD(&skip));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_LSB_DEPTH(16));  
	opus_encoder_ctl(m_OpusEncoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	LOGE("opus create param end:%d,%d,%d", nSampleRate, nBitrate, nChannel);
	return 0;
}

int CAudioCapChanle::encode2Opus(char* input, int inlen, unsigned char* output, int& outlen)
{
	opus_int16 *frame = (opus_int16 *) input;  
	int nbytes = opus_encode(m_OpusEncoder, frame, inlen/(sizeof(opus_int16)*mChannels), output, outlen);	
	return nbytes;
}

void CAudioCapChanle::ReleaseConnections()
{

}

int CAudioCapChanle::Start()
{
	m_bThreadStopped = false;
	if (pthread_create(&m_thread,NULL,InitThreadProc,(void*)this)!=0)
	{
		LOGE("start thread failed"); 
		return false;
	}

	mbIsPaused = false;
    return 0;
}

void CAudioCapChanle::Stop()
{
	mbIsPaused = true;
	m_bThreadStopped=true;
	pthread_join(m_thread,NULL);
}

void CAudioCapChanle::setPause(bool isPause)
{
	LOGE("CAudioCapChanle::setPause:%d", isPause);  
	mbIsPaused = isPause;
}

/** 
      * ��������ʱ��ļ�����õ�ʱ��� 
      * @param struct timeval* resule ���ؼ��������ʱ�� 
      * @param struct timeval* x ��Ҫ�����ǰһ��ʱ�� 
      * @param struct timeval* y ��Ҫ����ĺ�һ��ʱ�� 
      * return -1 failure ,0 success 
  **/ 
  #include <sys/time.h> 
  static int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y) 
  { 
        int nsec; 
    
        if ( x->tv_sec>y->tv_sec ) 
                  return -1; 
    
        if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) ) 
                  return -1; 
    
        result->tv_sec = ( y->tv_sec-x->tv_sec ); 
        result->tv_usec = ( y->tv_usec-x->tv_usec ); 
    
        if (result->tv_usec<0) 
        { 
                  result->tv_sec--; 
                  result->tv_usec+=1000000; 
        } 
    
        return 0; 
  }


void CAudioCapChanle::ThreadProcMain()
{
	int iDstLen = 4096;
	unsigned char acBuf[4096];	

	LOGE("CAudioCapChanle ThreadProcMain in");
	PPACKET pPacket = NULL;
	while(!m_bThreadStopped) 
	{
		if (m_OpusEncoder == NULL) 
		{
			LOGE("ERROR Audio Decoder is NULL.");
			break;
		}

		if (m_ListPcmData.size() > 0 )
		{
			KAutoLock l(m_kCritSec);
			pPacket=(PPACKET)m_ListPcmData.front();
			m_ListPcmData.pop_front();
		}
		else
		{
			usleep(500);
			continue;
		}

        	//��������
		if(mVolueEnable && mpVolumeNtf)
		{
			long tmCurrent = getCurrentTm();
			if(tmCurrent - mLastVolumeTm >= mIntervalVolumeTm)
			{
				int volume = getVolueByPcm((unsigned char*)pPacket->m_pPacketData, pPacket->m_nPacketSize);
				mpVolumeNtf->OnAudioVolumeOutput(volume);
				mLastVolumeTm = tmCurrent;
			}
		}

		struct timeval start,stop,diff; 
		gettimeofday(&start,0); 

#if 1
		memset(acBuf, 0, sizeof(acBuf));
		int sizelen = sizeof(acBuf);
		int iRet = encode2Opus((char*)pPacket->m_pPacketData, pPacket->m_nPacketSize, acBuf, sizelen);

		gettimeofday(&stop,0); 
		timeval_subtract(&diff,&start,&stop); 
		//LOGI("encode, list size:%ld, tm:(%ld)(%ld), data len:%d, encode len:%d", m_ListPcmData.size(), diff.tv_usec, diff.tv_usec/1000, pPacket->m_nPacketSize, iRet); 

		if(iRet > 0 && m_pOpusDataCb)
		{
			m_pOpusDataCb->OnAudioOpusNotifyOutput(acBuf, iRet);
		}
		else
		{
			LOGE("opus encode failed.");
		}
#else

		if(m_pOpusDataCb)
		{
			saveData2File((char*)pPacket->m_pPacketData, pPacket->m_nPacketSize);
			m_pOpusDataCb->OnAudioOpusNotifyOutput((unsigned char*)pPacket->m_pPacketData, pPacket->m_nPacketSize);
		}
#endif

		delete pPacket;
		pPacket = NULL;
	}
	
	LOGE("CAudioCapChanle ThreadProcMain end");
}

long CAudioCapChanle::getCurrentTm()
{
	long tm = time(0);
	struct timeval tv;
	gettimeofday(&tv,NULL);
	tm = tv.tv_sec*1000 + tv.tv_usec/1000;
	return tm;
}

int CAudioCapChanle::getVolueByPcm(const unsigned char* ptr, int size)
{
	int ndb = 0;
	short int value;

	int i;
	long v = 0;
	for(i=0; i<size; i+=2)
	{
		memcpy((char*)&value, ptr+i, 1);
		memcpy((char*)&value+1, ptr+i+1, 1);
		v += abs(value);
	}

	v = v/(size/2);

	if(v != 0) {
		ndb = (int)(20.0*log10((double)v / 65535.0 ));
	}
	else {
		ndb = -96;
	}

	return ndb;
}


