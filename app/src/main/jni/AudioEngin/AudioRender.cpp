//#include "StdAfx.h"

#include "AudioRender.h"
#include "android_opensles_output.h"
#include "Log.h"
#include "Define.h"
#include "AudioTracker.h"

#include "XListPtr.h"

IAudioRender *CreateAudioRender()
{
	//return new AndroidOpenSLESOutput();
	//return new CAudioTracker();
	return NULL;
}

static char arAudioFileName[256];
static void saveData2File2(char *pData, int len)
{
	FILE *pFile = NULL;
	pFile = fopen(arAudioFileName, "a+");
	if(pFile)
	{
		fwrite(pData , 1, len, pFile );    
		fclose(pFile); 
		pFile = NULL;
		//LOGE("save 2 file ok:%s", arAudioFileName); 
	}
	else
	{
		//LOGE("open file failed:%s", arAudioFileName); 
	}
}

unsigned long CAudioPlayChanle::TimeGetTimestamp()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return (now.tv_sec - m_tmInit.tv_sec)*1000+now.tv_usec/1000;
}


CAudioPlayChanle::CAudioPlayChanle()
:m_pAudioRender(NULL)
,m_OpusDecoder(NULL)
,mChannelID(-1)
{
	time_t now_time;
	now_time = time(NULL);
	memset(arAudioFileName, 0, sizeof(arAudioFileName));
	sprintf(arAudioFileName, "/mnt/sdcard/b_conf_recv_opus_%ld.pcm", now_time);

	mSampleRate = 0;
	mBitRate = 0;
	mChannel = 0;
	mSamplesPerFrame = 0;
	mParamExt = 0;
	gettimeofday(&m_tmInit,NULL);
    /*
    	memset(mJitterBuffer, 0, sizeof(mJitterBuffer));
       for(int i = 0; i < JB_CNT; i++)
       {
       	mJitterBuffer[i].m_pPacketData = new char[PKG_BUF_LEN];
       }

       mStartSeq = mEndSeq = mRecvedSeq = -1;*/
}


CAudioPlayChanle::~CAudioPlayChanle()
{
	if(m_pAudioRender)
	{
		delete m_pAudioRender;
		m_pAudioRender = NULL;
	}

/*
    	for(int i = 0; i < JB_CNT; i++)
       {
       	if (mJitterBuffer[i].m_pPacketData != NULL) {
			delete[] (char *)mJitterBuffer[i].m_pPacketData;
			mJitterBuffer[i].m_pPacketData = NULL;
		}
       }*/
}


int  CAudioPlayChanle::Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam)
{
	mSampleRate = nSampleRate;
	mBitRate = nBitrate;
	mChannel = nChannel;
	mSamplesPerFrame = nSamplesPerFrame;
	mParamExt = nParam;
	
	//m_pAudioRender->Connect(mChannel,mSampleRate,mSamplesPerFrame,mBitRate,mParamExt);

	LOGE("create opus decoder param:%d,%d", mSampleRate, mChannel); 
	int err = 0; 
	m_OpusDecoder = opus_decoder_create(mSampleRate, mChannel, &err);  
	if (err != OPUS_OK) 
	{  
		LOGE("cannnot decode opus: %d(%s)", err, opus_strerror(err));   
		return -1;  
	}  

	if (m_pAudioRender == NULL) {
		m_pAudioRender = CreateAudioRender();
	}

	if(m_pAudioRender)
	{
		err = m_pAudioRender->Connect(nChannel, nSampleRate, nSamplesPerFrame, nBitrate, 0);
        	if (err != 0)
		{
			delete m_pAudioRender;
			m_pAudioRender = NULL;
			return -1;
		}
       }
	
	return 0;
}

void CAudioPlayChanle::ReleaseConnections(void)
{
	if(m_pAudioRender)
	{
		m_pAudioRender->ReleaseConnections();
		delete m_pAudioRender;
		m_pAudioRender = NULL;
	}
}

int  CAudioPlayChanle::Start()
{
	m_bThreadStopped = false;
	m_thread = 0;
	if (pthread_create(&m_thread,NULL,InitThreadProc,(void*)this)!=0)
	{
		LOGE("start thread failed"); 
		m_bThreadStopped = true;
		return false;
	}

	if(m_pAudioRender)
	{
		m_pAudioRender->Start();
	}
}

void CAudioPlayChanle::Stop()
{
	LOGE("AudioPlayChanle stop in");
	m_bThreadStopped=true;
	if(m_pAudioRender)
	{
		m_pAudioRender->Stop();
	}
	LOGE("AudioPlayChanle stop in 1");

	{
		KAutoLock myLock(m_kCritThread);
		if(m_thread != 0)
		{
			pthread_join(m_thread,NULL);
		}
		m_thread = 0;
	}

	{
		KAutoLock l(m_kCritSec);
		m_ListPcmData.clear();
	}
	LOGE("thread ended"); 
}

void CAudioPlayChanle::PushPacket(unsigned char*pData,int nLen, int chanelID, unsigned short seq, unsigned long timestamp)
{
	//LOGE("push opus 2 list in");

	KAutoLock l(m_kCritSec);
	if(m_ListPcmData.size() > 200)
	{
		LOGE("the packeg list much than 200, will clear this list");
		PPACKET pDel = NULL;
		while(m_ListPcmData.size() > 0) 
		{
			pDel=(PPACKET)m_ListPcmData.front();
			m_ListPcmData.pop_front();
			delete pDel;
		}
		return;
	}

	AUDIO_PKG *pPacket = new AUDIO_PKG(pData, nLen, chanelID, timestamp, seq, TimeGetTimestamp());
	XListPtr::iterator item=m_ListPcmData.begin();
	while (m_ListPcmData.size() > 0 && item!=m_ListPcmData.end())
	{
		AUDIO_PKG *pTmp = (AUDIO_PKG*)item.m_pMember;
		if (pTmp->m_nTimeStamp > pPacket->m_nTimeStamp)
		{
			//插入
			LOGE("recv opus data, and out-of-order");
			m_ListPcmData.insert(item, pPacket);
			pPacket = NULL;
			break;
		}
		++item;
	}
    
	if(pPacket)
	{
		m_ListPcmData.push_back((void*)pPacket);
	}
	//LOGE("push opus 2 list end, datalen:%d, list size:%ld", nLen, m_ListPcmData.size());
}

void CAudioPlayChanle::setAudioPcmPlayer(AiyouVoiceRpm *pVoiceRpm, int chanid)
{
	LOGI("set pcm player in, chanid:%d", chanid);
	m_pPcmPlayer = pVoiceRpm;
    	mChannelID = chanid;
}


int CAudioPlayChanle::decode2Pcm(char* input, int inlen, unsigned char* output, int outlen)
{
	if(!m_OpusDecoder)
	{
		LOGE("error, OpusDecoder is null");
		return -101;
	}
	
	int decode_len = opus_decode(m_OpusDecoder, (unsigned char*)input, inlen, (opus_int16 *)output, outlen/(sizeof(opus_int16)), 0);  

	decode_len = decode_len*sizeof(opus_int16);
	//LOGI("decode opus len is:%d, in len:%d, out len:%d", decode_len, inlen, outlen);
	return decode_len;
}

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

#define CACHE_PKG_CNT (20) //列表中缓存的包数

void CAudioPlayChanle::ThreadProcMain()
{
	const int iDstLen = 960*2;
	unsigned char acBuf[iDstLen];

	LOGE("CAudioPlayChanle ThreadProcMain in");

	int interval = 1000/JB_AUDIO_PKG_CNT-1;
    	long lastTime = 0;

	AUDIO_PKG *pPacket = NULL;
	while(!m_bThreadStopped) 
	{
		if (m_OpusDecoder == NULL || m_bThreadStopped) 
		{
			LOGE("big error, Audio Decoder is NULL, or thread stoped");
			break;
		}
        	
		long curTime = TimeGetTimestamp();
        	if((curTime - lastTime < interval) && m_ListPcmData.size() < CACHE_PKG_CNT*2) //2倍的缓存包数以内就等待
             {
             		usleep(500);
			continue;
             }

            	lastTime = curTime;
        	if (m_ListPcmData.size() < CACHE_PKG_CNT)
		{
			usleep(500);
			continue;
		}
		else
		{
			KAutoLock l(m_kCritSec);
			pPacket=(AUDIO_PKG*)m_ListPcmData.front();
			m_ListPcmData.pop_front();
		}


		//KAutoLock myLock(m_kCritThread);

		struct timeval start,stop,diff; 
		
        	gettimeofday(&start,0);
		memset(acBuf, 0, sizeof(acBuf));
		int iRet = decode2Pcm((char*)pPacket->m_pPacketData, pPacket->m_nPacketSize, acBuf, iDstLen);
        
		
		if(iRet > 0 && m_pPcmPlayer /*m_pAudioRender*/)
		{
			//m_pAudioRender->PushPacket(acBuf, iRet);
            		m_pPcmPlayer->OnPcmDataIn(acBuf, iRet, mChannelID, 0);
			//saveData2File2((char*)acBuf, iRet);

			//gettimeofday(&stop,0); 
			//timeval_subtract(&diff,&start,&stop); 
			//LOGI("decode, list size:%ld, tm:(%ld)(%ld), decode len:%d", m_ListPcmData.size(), diff.tv_usec, diff.tv_usec/1000, iRet); 
		}
		else
		{
			LOGE("decode opus failed(%d) or audio render null", iRet);
		}
		delete pPacket;
		pPacket = NULL;
	}

	LOGE("ThreadProcMain end");
}

