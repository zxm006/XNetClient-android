#include "OpenRemoteUser.h"
#include <AUDEC_Header.h>
#include <AUDEC_CodecID.h>
#include <VIDEC_Header.h>

#include "log.h"
//#import "AudioUnitTool.h"

OpenRemoteUser::OpenRemoteUser(bool isVideoCalling)
: m_ulPeerAudioID(0)
, m_pMediaReceiver(NULL)
, m_MediaServerIp("")
, m_peer_locport(0)
, m_peer_mcuport(0)
, m_peer_mcuid("MCU001")/*�Է�MCUID*/
,m_isVideoCalling(isVideoCalling)
//,m_pVideoPlayChan(NULL)
,m_chanID(0)
,ms_pAudioPlay(NULL)
,mbPauseAudio(false)
{

    if (m_isVideoCalling)
    {

    }
    else
    {
      
    }
}

OpenRemoteUser::~OpenRemoteUser()
{

}

bool OpenRemoteUser::ConnectMediaServer(std::string strServerIp, unsigned short nPort)
{
    m_MediaServerIp = strServerIp;
    m_nPort        = nPort;
    m_peer_mcuid   = "MCU001";
    m_peer_mcuaddr = strServerIp;
    m_peer_nataddr = strServerIp;
    m_peer_locaddr = strServerIp;
    m_peer_mcuport = nPort;
    m_peer_locport = nPort;
    
    if(!CreateScreenReceiver())
        return false;
    return true;
}

void OpenRemoteUser::ReleaseMediaSever()
{
    ReleaseReceiver();
}

bool OpenRemoteUser::CreateScreenReceiver()
{
    m_pMediaReceiver = XNetMediaReceiver::Create(*this);
    if (m_pMediaReceiver!=NULL)
    {
        m_pMediaReceiver->SetLocalMCUID("MCU001");
        m_pMediaReceiver->SetLocalMCIIP(m_MediaServerIp.c_str());
        m_pMediaReceiver->SetLocalMCUPort(m_nPort);
        
        if (0!=m_pMediaReceiver->Open(m_peer_nodeid.c_str(), m_peer_nataddr.c_str(), m_peer_locaddr.c_str(),
                                      m_peer_locport, m_peer_mcuid.c_str(), m_peer_mcuaddr.c_str(), m_peer_mcuport))
        {
            m_pMediaReceiver->Close();
            delete m_pMediaReceiver;
            m_pMediaReceiver=NULL;
            
            return false;
        }
        
        return true;
    }
    
    return false;
}


bool OpenRemoteUser::ms_bPlayAudio = true;

void OpenRemoteUser::setAudioPlayer(CAudioPlayChanle *pAudioPlay)
{
	ms_pAudioPlay = pAudioPlay;
}

CAudioPlayChanle* OpenRemoteUser::getAudioPlayer()
{
	return ms_pAudioPlay;
}

void OpenRemoteUser::setAudioPlayFlag(bool bPlay)
{
	LOGI("setAudioPlay Flag:%d", bPlay);
	ms_bPlayAudio = bPlay;
}

void OpenRemoteUser::setAudioMute(bool bPause)
{
	LOGI("pause flag:%d", bPause);
	mbPauseAudio = bPause;
}


bool OpenRemoteUser::OpenAudioReceive(unsigned long ulPeerUserAudioId)
{
    m_ulPeerAudioID = ulPeerUserAudioId;
    if (m_pMediaReceiver==NULL || m_ulPeerAudioID==0)
    {
        return false;
    }
    if(0!=m_pMediaReceiver->SetAudioID((unsigned int)m_ulPeerAudioID))
        return false;
    if(0!=m_pMediaReceiver->StartAudio())
        return false;
    
    return true;
}

bool OpenRemoteUser::CloseAudioReceive()
{
	if (m_pMediaReceiver==NULL || m_ulPeerAudioID==0)
	{
		return false;
	}
	LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close 00");
	m_pMediaReceiver->StopAudio();
	LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close 01");

	return true;
}


bool OpenRemoteUser::OpenVideoReceive(unsigned long ulPeerUserVideoId)
{
    m_ulPeerVidoID = ulPeerUserVideoId;
    if (m_pMediaReceiver==NULL || m_ulPeerVidoID==0)
    {
        return false;
    }
    if(0!=m_pMediaReceiver->SetVideoID((unsigned int)m_ulPeerVidoID))
        return false;
    if(0!=m_pMediaReceiver->StartVideo())
        return false;
    
    return true;
}

bool OpenRemoteUser::CloseVideoReceive()
{
	if (m_pMediaReceiver==NULL || m_ulPeerVidoID==0)
	{
		return false;
	}

//    m_pMediaReceiver->StopVideo();

//    if (m_pVideoPlayChan != NULL)
//    {
//        LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close1");
//
//        m_pVideoPlayChan->Close();
//        LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close2");
//
//        m_pVideoPlayChan->Destory();
//        LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close3");
//        delete m_pVideoPlayChan;
//        m_pVideoPlayChan = NULL;
//        LOGI("COpenRemoteUser::CloseRemoteVideo m_pPlayChan Close4");
//    }
	return true;
}

void OpenRemoteUser::ReleaseReceiver()
{
    if (m_pMediaReceiver!=NULL)
    {
        m_pMediaReceiver->Close();
        delete m_pMediaReceiver;
        m_pMediaReceiver=NULL;
    }
}

int  OpenRemoteUser::reopenMediaRecver()
{
	if (m_pMediaReceiver!=NULL)
	{
		m_pMediaReceiver->StopAudio();
		m_pMediaReceiver->StopVideo();
	}
	ReleaseReceiver();
	CreateScreenReceiver();
	OpenAudioReceive(m_ulPeerAudioID);
	OpenVideoReceive(m_ulPeerVidoID);
	return 0;
}


void OpenRemoteUser::getAudioData(unsigned char*  pdata,long len)
{
    if (!m_data||m_length==0)
    {
        return;
    }
    
    pdata = new  unsigned char(m_length);
    memcmp(pdata, m_data, len);
    len = m_length;
    
    if (m_data)
    {
        m_length=0;
        delete m_data;
        m_data =NULL;
    }
}

long getTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	long tmmins  = tv.tv_sec*1000 + tv.tv_usec/1000;
	long tmweim = tv.tv_sec*1000000 + tv.tv_usec;
	return tmmins;
}


void OpenRemoteUser::OnXNetMediaReceiverCallbackAudioPacket(unsigned char*pData,int nLen)
{
	//LOGI("OpenRemote User, AudioPacket recv callback, %p, len:%d", pData, nLen);
	if(pData != NULL && nLen>8 )
	{
		int nHeaderSize = AUDEC_HEADER_GET_LEN(pData);
		unsigned char* pAudio = pData + nHeaderSize;
		int len = nLen - nHeaderSize;

		if(!ms_bPlayAudio)
		{
			//LOGI("this is not play audio");
			return;
		}

        	if(mbPauseAudio)
             {
             		//LOGI("this is paused, not play it");
			return;
             }

		unsigned short seq = AUDEC_HEADER_GET_SEQUENCE(pData);
		unsigned int timestamp = AUDEC_HEADER_GET_TIMESTAMP(pData);
        	//LOGI("recv audio data, seq:%d,timestamp:%d", seq, timestamp);
		if(ms_pAudioPlay)
		{
			//LOGI("will call push packet");
			ms_pAudioPlay->PushPacket(pAudio, len, m_chanID, seq, timestamp);
		}
	}
}

void OpenRemoteUser::SetVideoWindow(void* pVideoWindow)
{
  //[m_liveFFmpegdecode SetLocalVideoWindow:pVideoWindow];
  //  [m_liveFFmpegdecode Beginmpeg_decode_h264];
//    if (m_pVideoPlayChan == NULL)
//    {
// #if 0
//        m_pVideoPlayChan = CreateVideoPlayChan();
//        m_pVideoPlayChan->SetDisplay((Surface)pVideoWindow);
//        m_pVideoPlayChan->Init();
//
//        m_pVideoPlayChan->Open();
//#endif
//    }
}

void OpenRemoteUser::resetVideoWindow(void* pVideoWindow)
{
//    if (m_pVideoPlayChan != NULL)
//    {
//        m_pVideoPlayChan->ResetDisplay((Surface)pVideoWindow);
//    }
//    else
//    {
//        LOGE("reset video wnd, but video play obj is null");
//    }
}

void OpenRemoteUser::setAudioChanID(int id)
{
	m_chanID = id;
}

int  OpenRemoteUser::getAudioChanID()
{
	return m_chanID;
}
	

void OpenRemoteUser::OnXNetMediaReceiverCallbackVideoPacket(unsigned char*pData,int nLen)
{
    if(pData != NULL && nLen>16 )
    {
        int nHeaderLen=VIDEC_HEADER_EXT_GET_LEN(pData);
        unsigned char* pVideo = pData + nHeaderLen;
        int len = nLen - nHeaderLen;
        
        //if(m_liveFFmpegdecode)
        {
            //[m_liveFFmpegdecode ffmpeg_decode_h264:pVideo Len:len iCount:1];
        }

//        if (m_pVideoPlayChan!=NULL)
//        {
//            //LOGI("push data 2 play channel, len:%d", nLen);
//            m_pVideoPlayChan->PushPacket((unsigned char *)pData, nLen);
//        }
        
    }
}
