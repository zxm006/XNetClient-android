//
//  OpenLocalUser.cpp
//#include "stdafx.h"
#include "OpenLocalUser.h"
#include <AUDEC_Header.h>
#include <AUDEC_CodecID.h>
#include "VIDEC_Header.h"
#include<stdlib.h>
#include <string.h>
#include<sys/time.h>
#ifdef WIN32
#include <MMSystem.h>
#endif

#include "log.h"


OpenLocalUser::OpenLocalUser()
: m_nPort(0)
, m_ulLocalAudioID(0)
, m_pMediaSender(NULL)
, m_bGotKeyFrameMain(false)
, m_pVideoPacket(NULL)
, m_nFrameBufferLength(0)
, m_pAudioPacket(NULL)
, m_nAudioFrameBufferLength(0)
, m_usAudioSequence(0)
, m_ulLastFrameTimestamp(0)
, m_ulLastPacketTimestamp(0)
, m_MediaServerIp("")
//, m_pVideoCapChan()
, m_nWidth(320)
, m_nHeight(240)
, m_nBitRate(128)
, m_nFrameRate(15)
{
	gettimeofday(&m_tmInit,NULL);
}

OpenLocalUser::~OpenLocalUser()
{

}

bool OpenLocalUser::ConnectMediaServer(std::string strServerIp, unsigned short nPort)
{
	m_MediaServerIp = strServerIp;
	m_nPort = nPort;

	if(!CreateSender())
		return false;

	return true;
}

void OpenLocalUser::ReleaseMediaSever()
{
    ReleaseSender();
}

bool OpenLocalUser::OpenAudioSend(unsigned int ulUserAudioId)
{
    if( ulUserAudioId == 0)
        return false;
    
    m_ulLocalAudioID = ulUserAudioId;

	if (0!=m_pMediaSender->StartAudio(m_ulLocalAudioID))
	{
		return false;
	} 
 
    return true;
}


bool OpenLocalUser::OpenVideoSend(unsigned int ulUserVideoId)
{
    if( ulUserVideoId == 0)
        return false;
    
    m_ulLocalVideoID = ulUserVideoId;
    
    if (0!=m_pMediaSender->StartVideo(m_ulLocalVideoID))
    {
        return false;
    }

    return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool OpenLocalUser::CloseVideoSend()
{
    m_ulLocalVideoID = 0;
    if (m_pMediaSender!=NULL)
    {
        m_pMediaSender->StopVideo();
        return true;
    }
    
    return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool OpenLocalUser::CloseAudioSend()
{
	m_ulLocalAudioID = 0;

	if (m_pAudioPacket)
	{
		free(m_pAudioPacket);
		m_pAudioPacket = NULL;
	}

	if (m_pMediaSender!=NULL)
	{
		m_pMediaSender->StopAudio();
		return true;
	}

	return true;
}
void OpenLocalUser::resetLocalVideo()
{
//    [[CameraHelp shareCameraHelp]reSetCamera];
    
}

void OpenLocalUser::switchLocalVideo()
{
//    if(m_pVideoCapChan != NULL)
//       {
//           m_pVideoCapChan->SwitchCamera();
//       }
}


void OpenLocalUser::openLocalVideo(void *localvideo)
{
//
//    LOGI("COpenLocalUser::OpenLocalVideo()1  m_pVideoCapChan = %p",m_pVideoCapChan);
//    if (m_pVideoCapChan == NULL)
//    {
//        m_pVideoCapChan = CreateVideoCapChan();
//     /*1��¼���ɼ���2������ͷ�ɼ�*/
//        m_pVideoCapChan->Init(2);
//    }
//
//    //����������������Ƴ�
//    m_pVideoCapChan->SetPreview((Surface)localvideo);
//    m_pVideoCapChan->RegisterCallback(this);
//
//    LOGI("COpenLocalUser::OpenVideoSend() 2  m_nWidth = %d|m_nHeight = %d",m_nWidth,m_nHeight);
//    m_pVideoCapChan->Open(1, m_nWidth, m_nHeight, m_nBitRate, m_nFrameRate);
//    LOGI("COpenLocalUser::OpenVideoSend() 3 ");
}

void OpenLocalUser::closeLocalVideo()
{
//    if (m_pVideoCapChan != NULL)
//    {
//        m_pVideoCapChan->Close();
//        m_pVideoCapChan->Destory();
//        delete m_pVideoCapChan;
//        m_pVideoCapChan = NULL;
//    }
}

bool OpenLocalUser::SendAudioData(char*pData, int nLen)
{
	ProcessAudioFrame(pData, nLen, TimeGetTimestamp());
	return true;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool OpenLocalUser::CreateSender()
{
	KAutoLock lock(m_mKCritSec);
	m_pMediaSender = XNetMediaSender::Create(*this);
	if (m_pMediaSender!=NULL)
	{
		m_pMediaSender->SetLocalMCUID("MCU001");
		m_pMediaSender->SetLocalMCIIP(m_MediaServerIp.c_str());
		m_pMediaSender->SetLocalMCUPort(m_nPort);
		int m_ConnectStatus = 2;//CS_CONNECTED;
		m_pMediaSender->SetConnectStatus(m_ConnectStatus);

		if ( 0 != m_pMediaSender->Open())
		{
			m_pMediaSender->Close();
			delete m_pMediaSender;
			m_pMediaSender=NULL;
			return false;
		}
		return true;
	}
	return false;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
void OpenLocalUser::ReleaseSender()
{
	KAutoLock lock(m_mKCritSec);
	if (m_pMediaSender!=NULL)
	{
		m_pMediaSender->Close();
		delete m_pMediaSender;
		m_pMediaSender = NULL;
	}
}

bool OpenLocalUser::reopenMediaSender()
{
	if (m_pMediaSender!=NULL)
	{
		m_pMediaSender->StopAudio();
		m_pMediaSender->StopVideo();
	}
	ReleaseSender();
	CreateSender();
	OpenAudioSend(m_ulLocalAudioID);
	OpenVideoSend(m_ulLocalVideoID);
	return true;
}


bool OpenLocalUser::SendVideoData(unsigned char*pData, int nLen, bool bKeyFrame, unsigned long ulTimestamp, int nWidth, int nHeight)
{
    ProcessVideoFrame(pData, nLen,bKeyFrame, TimeGetTimestamp(),nWidth,nHeight);
    return true;
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

void OpenLocalUser::ProcessVideoFrame(unsigned char*pData, int nLen, bool bKeyFrame, unsigned long ulTimestamp, int nWidth, int nHeight)
{
	if (m_nFrameBufferLength<nLen+1024)
	{
		m_nFrameBufferLength=nLen+2048;
		if (m_pVideoPacket)
		{
			free(m_pVideoPacket);
			m_pVideoPacket=NULL;
		}
		m_pVideoPacket=(char*)malloc(m_nFrameBufferLength);
		if (m_pVideoPacket==NULL)
		{
			m_nFrameBufferLength=0;
			return;
		}
	}
    
	if (m_bGotKeyFrameMain==false && bKeyFrame && m_pVideoPacket)
	{
		m_bGotKeyFrameMain=true;
	}
    
	if (m_bGotKeyFrameMain && m_pVideoPacket)
	{
		VIDEC_HEADER_EXT_RESET(m_pVideoPacket);
		VIDEC_HEADER_EXT_SET_CODEC_ID(m_pVideoPacket,VIDEC_CODEC_H264);
		VIDEC_HEADER_EXT_SET_DOUBLE_FIELD(m_pVideoPacket,0);
		VIDEC_HEADER_EXT_SET_KEYFRAME(m_pVideoPacket,(bKeyFrame?1:0));
		VIDEC_HEADER_EXT_SET_SEQUENCE(m_pVideoPacket,m_usVideoSequence++);
		VIDEC_HEADER_EXT_SET_TIMESTAMP(m_pVideoPacket,(unsigned int)ulTimestamp);
		
		if (bKeyFrame)
		{
			VIDEC_HEADER_EXT_SET_ACTUAL_WIDTH(m_pVideoPacket,nWidth);
			VIDEC_HEADER_EXT_SET_ACTUAL_HEIGHT(m_pVideoPacket,nHeight);
			VIDEC_HEADER_EXT_SET_VIRTUAL_WIDTH(m_pVideoPacket,nWidth);
			VIDEC_HEADER_EXT_SET_VIRTUAL_HEIGHT(m_pVideoPacket,nHeight);
		}
        
		int nHeaderLen=VIDEC_HEADER_EXT_GET_LEN(m_pVideoPacket);
		
		memcpy(m_pVideoPacket+nHeaderLen,pData,nLen);
        
		VIDEC_HEADER_EXT_SET_MAIN_FRAME(m_pVideoPacket,1);
		KAutoLock lock(m_mKCritSec);
		if (m_pMediaSender != NULL)
		{
			 int nRet = m_pMediaSender->SendVideo((unsigned char *)m_pVideoPacket, nLen+nHeaderLen);
			//LOGI("send video h264 rlt is:%d ,len:%d,%d", nRet, nLen, nHeaderLen);	
		}
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
void OpenLocalUser::ProcessAudioFrame(char*pData, int nLen, unsigned long ulTimestamp)
{
	if (m_nAudioFrameBufferLength<nLen+1024)
	{
		LOGI("ProcessAudioFrame 1, %d, %d", m_nAudioFrameBufferLength, nLen);
		m_nAudioFrameBufferLength=nLen+2048;
		if (m_pAudioPacket)
		{
			free(m_pAudioPacket);
			m_pAudioPacket=NULL;
		}
		m_pAudioPacket=(char*)malloc(m_nAudioFrameBufferLength);
		if (m_pAudioPacket==NULL)
		{
			m_nAudioFrameBufferLength=0;
			return;
		}
	}
    
	if (m_pAudioPacket)
	{
		unsigned long ulCurTimestamp=ulTimestamp;
		if (ulCurTimestamp-m_ulLastFrameTimestamp<120)
		{
			unsigned long ulDelta1=ulCurTimestamp-m_ulLastPacketTimestamp;
			unsigned long ulDelta2=m_ulLastPacketTimestamp-ulCurTimestamp;
			unsigned long ulDelta=(ulDelta1>ulDelta2?ulDelta2:ulDelta1);
			if (ulDelta==ulDelta2)
			{
				m_ulLastPacketTimestamp+=1;
			}
			else
			{
				m_ulLastPacketTimestamp=ulCurTimestamp;
			}
		}
		else
		{
			m_ulLastPacketTimestamp=ulCurTimestamp;
		}
		m_ulLastFrameTimestamp=ulCurTimestamp;

		int nTmp = m_usAudioSequence++;
		if(m_usAudioSequence > 65535)
		{
			m_usAudioSequence = 0;
		}
		AUDEC_HEADER_RESET(m_pAudioPacket);
		AUDEC_HEADER_SET_SEQUENCE(m_pAudioPacket, nTmp);
		AUDEC_HEADER_SET_TIMESTAMP(m_pAudioPacket, (unsigned int)m_ulLastPacketTimestamp);
		AUDEC_HEADER_SET_CODEC_ID(m_pAudioPacket, X_AUDIO_CODEC_AMR_NB_475);
		int nHeaderSize=AUDEC_HEADER_GET_LEN(m_pAudioPacket);
		memcpy(m_pAudioPacket+nHeaderSize,pData,nLen);
		KAutoLock lock(m_mKCritSec);
		if(m_pMediaSender)
		{
			int iRet = m_pMediaSender->SendAudio((unsigned char*)m_pAudioPacket, nHeaderSize+nLen);
		}
		//LOGI("send audio ret is:%d, len:%d, req:%d, timespan:%d", iRet, nHeaderSize+nLen, nTmp, m_ulLastPacketTimestamp);
//      int nRet =  printf("nRet ==>%d",nRet);
	}
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
unsigned long OpenLocalUser::TimeGetTimestamp()
{
#ifdef WIN32
	return ::timeGetTime();
#else
	struct timeval now;
	gettimeofday(&now,NULL);
	//return now.tv_sec*1000+now.tv_usec/1000;
	return (now.tv_sec - m_tmInit.tv_sec)*1000+now.tv_usec/1000;
#endif
}

void OpenLocalUser::OnVIDEO_CapChanCallback(unsigned char*pData,int nLen,bool& bKeyFrame,int nWidth,int nHeight,int nHeaderLen)
{
    //��Ƶ�����󷵻����˽��Э��ͷ��h264����
    //LOGW("cap h26 encoded data:%x, len:%d,wid:%d,hei:%d,headlen:%d", pData, nLen,  nWidth, nHeight, nHeaderLen);
    SendVideoData(pData,nLen,bKeyFrame,0,nWidth,nHeight);
}

void OpenLocalUser::SetVideoCapParam(int nWidth, int nHeight, int nBitRate, int nFrameRate)
{
    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_nBitRate = nBitRate;
    m_nFrameRate = nFrameRate;
}


