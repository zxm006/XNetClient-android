//#include "StdAfx.h"
#include "CLinkMicManage.h"
#include "OpenRemoteUser.h"
#include "ConnectServer.h"
#include "AutoLock.h"

CLinkMicManage::CLinkMicManage()
:m_uUserID(0),
m_strAddress(""),
m_serverip(""),
m_OpenLocalUser(NULL),
m_userGroupId(""),
m_iscapscreen(NO),
m_isAnchor(NO),
m_bIsLinkUseVideo(false)
{
    XNetSetting::SetAudioProtocolType(XNetSetting::PT_UDP);
    XNetSetting::SetMCUOnly(1);
    m_pINetWorkCallback=NULL;
  
    m_UserInfoList.clear();
    setupSession();
    mlocalView = NULL;
    m_peertouchMoveViews.clear();

	m_pAudioCap = NULL;
	//m_pAudioPlay = NULL;
	m_pVoiceRpm = NULL;
}

CLinkMicManage::~CLinkMicManage(void)
{
    m_peertouchMoveViews.clear();
}


void CLinkMicManage::setupSession()
{

}
 
int CLinkMicManage::SendData(KCmdPacketEx& pPacket)
{
    return 0;
}



int CLinkMicManage::ConnectServer(const char* szIP,unsigned int nPort,bool isEncrypt)
{
    m_serverip = szIP;
    return m_ConnectServer.Start(szIP,nPort,isEncrypt,this);
}

void CLinkMicManage::OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen)
{
	//LOGI("CVoiceManage in, on auido opus notify, len:%d", nLen);
	this->SendAudioData((char*)pPcm, nLen);
}

void CLinkMicManage::setAudioCapParam(int iChannels, int iSampleRate, int iBitRate)
{
	LOGI("set audio cap param,channel,samp,bit:%d,%d,%d", iChannels, iSampleRate, iBitRate);
	mChannels	= iChannels;
	mSampleRate = iSampleRate;
	mBitRate	= iBitRate;
}

void CLinkMicManage::disconnect()
{
    m_ConnectServer.Stop();
}

void CLinkMicManage::On_SessionConnectStatus(LINKMIC_STATUS cs)
{
    
    if(cs == CS_LOGINED||cs == CS_RECONNECTED)
    {
        LOGI("login success");
        this->getUserList();
/*		
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);
*/
    }
    else if(cs == CS_DISCONNECTED||cs == CS_LOGINFAILED)
    {
        LOGI("login failed");
       
        this->closeMediaSender();
        this->StopAllRemoteMedia();
    }
    else if(cs == CS_CONNECTED||cs == CS_RECONNECTED)
    {
        this->LoginServer( m_username.c_str() ,"" );
    }
    
    if(m_pINetWorkCallback)
    {
        m_pINetWorkCallback->IConnectStatusCallback(cs);
    }
}

void CLinkMicManage::setNetWorkCallback(ILinkMicCallback*netWorkCallback)
{
    m_pINetWorkCallback   =netWorkCallback;
}

void CLinkMicManage::OnDispatchCmd(KCmdPacketEx& pPacket)
{
    std::string strCMD = pPacket.GetCMD();
    LOGI("strCMD ==>%s", strCMD.c_str());
   
    if(strCMD=="LOGINSERVERED")
    {
        m_uUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        unsigned long ulErrorType = pPacket.GetAttrib("ERRORTYPE").AsUnsignedLong();
        if(ulErrorType == 0)
        {
        	if(m_pVoiceRpm == NULL)
		{
			m_pVoiceRpm = AiyouVoiceRpm::getInstance();
			//LOGI("get voice rtp rlt:%x", m_ pVoiceRpm);
			m_pVoiceRpm->Init();
		}

		//LOGI("login success 1, voice rtp:%x", m_ pVoiceRpm);
		if (!m_isAnchor)
		{
			this->openMediaSender(m_iscapscreen);
		}

		this->getUserList();	

		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);
		}
        }
        else
        {
		LOGI("login failed 1");
		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINFAILED);
		}
        }
    }
    
    else if(strCMD=="REMOTEUSERLOGIN")
    {
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        unsigned long ulUseraudioID = pPacket.GetAttrib("USERAUDIOID").AsUnsignedLong();
        
        CLIENTUSERINFOLIST usrinfo;
        usrinfo.strUserName = strUserName;
        usrinfo.ulUserId = ulUserID;
        usrinfo.ulUserAudioID = ulUseraudioID;
        m_UserInfoList[ulUserID] = usrinfo;

	LOGI("remote user longin up begin:%d", ulUserID);
	/*����û��ֱ���������ڣ����ϱ���java�㣬���ڴ�����֮���ڵ��õײ����Ƶ��ʾ*/
        if(m_pINetWorkCallback)
        {
            m_pINetWorkCallback->INetReceiveUserLogin(ulUserID, strUserName,ulUseraudioID);
        }

	LOGI("remote user longin up end:%d", ulUserID);
        
    }
    else if(strCMD=="REMOTEUSERQUIT")
    {
		KAutoLock lock(m_mKCritSec);

		unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
		std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
		std::string strExpand = pPacket.GetAttrib("EXPAND").AsString();

		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->INetReceiveUserLogOut(ulUserID, strUserName);
		}
		
		CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.find(ulUserID);
		if(iter != m_UserInfoList.end())
		{
			m_UserInfoList.erase(iter);
		} 

		std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(ulUserID);
		if (it!=m_pOpenRemoteUser_map.end())
		{
			OpenRemoteUser *openRemoteUser=(*it).second;
			openRemoteUser->CloseAudioReceive();
			openRemoteUser->CloseVideoReceive();

			stopReceiveMedia(openRemoteUser);

			openRemoteUser->ReleaseMediaSever();
			delete openRemoteUser;
			m_pOpenRemoteUser_map.erase(it);
		}
		stopAudioDecoderIfNeed();
    }
    else if(strCMD=="UPDATEUSERLIST")
    {
    	LOGI("updata user list begin");
        KAutoLock lock(m_mKCritSec);
        CMD_ITEM_LST lstItems = pPacket.GetItemList();
        for(CMD_ITEM_LST::const_iterator it=lstItems.begin();it!=lstItems.end();it++)
        {
            KCmdItem item((std::string)*it);
            CLIENTUSERINFOLIST usrinfo;
            usrinfo.strUserName = item.GetAttrib("USERNAME").AsString();
            usrinfo.ulUserId =  item.GetAttrib("USERID").AsUnsignedLong();
            usrinfo.ulUserAudioID = item.GetAttrib("USERAUDIOID").AsUnsignedLong();
            
		/*�Ȳ���map�����Ƿ��Ѿ���������Ƶ�ˣ�
		����Ѿ������˾Ͳ������ˣ������ϱ�Ӧ�ò㣬Ȼ���Ӧ�ò��ڵ��ײ�������Ƶ�ӿ�*/

		if(m_isAnchor)
		{
			map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(usrinfo.ulUserId);
			if (it ==m_pOpenRemoteUser_map.end())
			{
				//�ϱ�����
				 if(m_pINetWorkCallback)
			        {
			            m_pINetWorkCallback->INetReceiveUserLogin(usrinfo.ulUserId, usrinfo.strUserName,usrinfo.ulUserAudioID);
			        }
			}
		}
		m_UserInfoList[usrinfo.ulUserId] = usrinfo;
        }
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->INetReceiveUserList(m_UserInfoList);

	LOGI("updata user list end");
    }
    else if(strCMD=="SIGNALINGTRANSFER")
    {
        bool isbroadcast = pPacket.GetAttrib("BROADCAST").AsBoolean();
        std::string strData = pPacket.GetAttrib("DATA").AsString();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        printf("strData =%s\n\r",strData.c_str());
        if(isbroadcast)
        {
            unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetBroadcastData(ulUserID, strData.c_str(), strData.size());
        }
        else
        {
            unsigned long ulUserID = pPacket.GetAttrib("PEERUSERID").AsUnsignedLong();
            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetReceiveData(ulUserID,strUserName, strData.c_str(), strData.size());
        }
    }
}

int CLinkMicManage::LoginServer(const char* szName, const char* szPassword)
{
    if(!szName || !szPassword)
        return -1;
 
    m_username = szName;
    srand((unsigned)time(NULL));
    long timeCurr=time(NULL)+rand()%1000000;
    this->m_audioID=timeCurr-1400000000 ;
    
    std::string strLogin = "LoginServer";
    KCmdPacketEx rPacket(strLogin.c_str(),(int)strLogin.length()+1);
    std::string strCMD = "LOGINSERVER";
    rPacket.SetCMD(strCMD);
    std::string strname = "USERNAME";
    rPacket.SetAttrib(strname,szName);
    std::string strPass = "USERPASS";
    rPacket.SetAttrib(strPass,szPassword);
    std::string strAddress = "ADDRESS";
    rPacket.SetAttrib(strAddress,m_strAddress);
    std::string straudioID = "USERAUDIOID";
    rPacket.SetAttribUL(straudioID, this->m_audioID);
    std::string groupid = "GROUPID";
    rPacket.SetAttrib(groupid, m_userGroupId);
    std::string expand = "EXPAND";
    rPacket.SetAttrib(expand, m_szexpand);
    return m_ConnectServer.SendData(rPacket);
}

int CLinkMicManage::logoutServer()
{    
	std::string strinfo = "quitserver";
	KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
	std::string strCMD = "QUITSERVER";
	rPacket.SetCMD(strCMD);

	closeMediaSender();
	if(m_isAnchor)
	{
		StopAllRemoteMedia();
	}
	m_UserInfoList.clear();
	

	return m_ConnectServer.SendData(rPacket);
}

int CLinkMicManage::getUserList()
{
    std::string strGetuserlist= "getuserlist";
    KCmdPacketEx rPacket(strGetuserlist.c_str(),(int)strGetuserlist.length()+1);
    rPacket.SetAttrib("ADDRESS", m_strAddress);
    std::string strCMD = "GETUSERLIST";
    rPacket.SetCMD(strCMD);
    return m_ConnectServer.SendData(rPacket);
}

void CLinkMicManage::loginLinkMicServer(
	const char* szIP,const char* szName ,const char* szGameId,
	const char* szGameServerId,const char* szRoomId,const char* szGroupId ,
	const char* szexpand,bool isencrypt,bool iscapscreen,bool isAnchor)
{
    if(!szIP||!szName||!szGameId||!szGameServerId||!szRoomId)
    {
          return;
    }
    m_iscapscreen = iscapscreen;
    m_isAnchor = isAnchor;
    m_szexpand = szexpand;
    char sAddress[255] = {0};
    sprintf(sAddress,"%s-%s-%s",szGameId,szGameServerId,szRoomId);
    m_strAddress = sAddress;
    m_username = szName;
    
    if(!szGroupId)
    {
        m_userGroupId = "";
    }
    else
    {
        m_userGroupId = szGroupId;
    }
     ConnectServer(szIP,5566,isencrypt);
}

int CLinkMicManage::SendLinkMic(const char* pData, unsigned long nLen,bool isinroom )
{
    if(!pData || nLen == 0)
        return -1;
    
    if (!isinroom&&m_userGroupId.compare("")==0)
    {
        return -1;
    }
    
   std::string strinfo = "SENDLinkMic";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SENDLinkMic";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("ISINROOM", isinroom);
    rPacket.SetAttrib("DATA", pData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttrib("USERNAME", m_username);
    rPacket.SetAttrib("ADDRESS", m_strAddress);
    rPacket.SetAttrib("GROUPID", m_userGroupId);
    rPacket.SetAttribUL("USERAUDIOID", m_audioID);
    rPacket.SetAttrib("EXPAND", m_szexpand);
    
    return m_ConnectServer.SendData(rPacket);
}

int CLinkMicManage::SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen)
{
    if(!pData || nLen == 0)
        return -1;
    std::string strinfo = "Signalingtransfer";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SIGNALINGTRANSFER";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("BROADCAST", false);
    rPacket.SetAttribUL("PEERUSERID", uPeerUserID);
    rPacket.SetAttrib("DATA", pData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttrib("USERNAME", m_username);
    return m_ConnectServer.SendData(rPacket);
}

int CLinkMicManage::SendAllUserData(const char* pData, unsigned long nLen)
{
    if(!pData || nLen == 0)
        return -1;
    
    std::string strinfo = "Signalingtransfer";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SIGNALINGTRANSFER";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("BROADCAST", true);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttribUL("USERAUDIOID", this->m_audioID);
    rPacket.SetAttrib("DATA", pData);
    return m_ConnectServer.SendData(rPacket);
}

std::string CLinkMicManage::GetLocalIP()
{
    return m_ConnectServer.GetLocalIP();
}

std::string CLinkMicManage::GetNATIP()
{
    return m_ConnectServer.GetNATIP();
}

void CLinkMicManage::SendAudioData(char*pData, int nLen)
{
    if (m_OpenLocalUser )
    {
        m_OpenLocalUser->SendAudioData(pData, nLen);
    }
}

void CLinkMicManage::resetLocalVideo()
{
    if (m_OpenLocalUser) {
        m_OpenLocalUser->resetLocalVideo();
    }
}

void CLinkMicManage::openMediaSender(bool isCapScreen)
{
	KAutoLock lock(m_mKCritSec);
	if (!m_OpenLocalUser)
	{
		LOGI("open Media Sender in");
		m_OpenLocalUser = new OpenLocalUser;
		m_OpenLocalUser->ConnectMediaServer(m_serverip, 5566);
		m_OpenLocalUser->OpenAudioSend((unsigned int)this->m_audioID);

		if(m_bIsLinkUseVideo)
		{
			m_OpenLocalUser->OpenVideoSend((unsigned int)this->m_audioID+200);
		}
		openLocalAudio();     
	}
}

void CLinkMicManage::closeMediaSender()
{
	KAutoLock lock(m_mKCritSec);
	if (m_OpenLocalUser != NULL)
	{
		closeLocalAudio();
		m_OpenLocalUser->CloseAudioSend();
		m_OpenLocalUser->ReleaseMediaSever();

		if (m_OpenLocalUser && m_bIsLinkUseVideo)
		{
			m_OpenLocalUser->closeLocalVideo();
			mlocalView = NULL;
		}


		delete m_OpenLocalUser;
		m_OpenLocalUser = NULL;
	}
}

void CLinkMicManage::closeLocalAudio( )
{
	LOGI("closeLocal Audio in");
	KAutoLock lock(m_mKCritSec);

	int iD = 0;
	if(m_pAudioCap)
	{
		iD = m_pAudioCap->mAyVcID;
	       if(m_pVoiceRpm)
		{	
			LOGI("stop cap audio in");
			m_pVoiceRpm->StopCapture(iD);
		}
           
		m_pAudioCap->Stop();
		m_pAudioCap->ReleaseConnections();

		delete m_pAudioCap;
		m_pAudioCap = NULL;
	}
}

void CLinkMicManage::openLocalAudio()
{
	LOGI("open local Audio in, voice rpm:%x", m_pVoiceRpm);
	if(m_pAudioCap == NULL)
	{
		m_pAudioCap = new CAudioCapChanle(true, mSampleRate, this);
		int iSampleBits = 16;// ��Ƶ�������
		int iInterval = 20;	//20ms����Ƶ��������
		int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
		m_pAudioCap->Connect(mChannels, mSampleRate, iBytes, mBitRate);
		m_pAudioCap->Start();
	}

	//LOGI("will call webrtc cap audio 0:%x", m_ pVoiceRpm);
	if(m_pVoiceRpm)
	{	
		LOGI("will call webrtc cap audio 1");
		m_pVoiceRpm->StartCapture(m_pAudioCap);
	}
}

int CLinkMicManage::startReceiveMedia(OpenRemoteUser* remoteUser)
{
	LOGI("start recv media in");
	//��Ƶ���벥����

	LOGI("start recv media, player is not null, release it first");
		
	CAudioPlayChanle *pAudioPlay = new CAudioPlayChanle();
	int iSampleBits = 16;// ��Ƶ�������
	int iInterval = 20; //20ms����Ƶ��������
	int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
	pAudioPlay->Connect(mChannels, mSampleRate, iBytes, mBitRate);
	pAudioPlay->Start();
	
	if(remoteUser)
	{
		remoteUser->setAudioPlayer(pAudioPlay);
	}

	
	int iChanID = -1;
	if(m_pVoiceRpm)
	{
		iChanID = m_pVoiceRpm->StartPlayout(mSampleRate);
	}	
	
	LOGI("ConnectMedia Server end, chanid:%d", iChanID);
	return iChanID;
}

void CLinkMicManage::stopReceiveMedia(OpenRemoteUser* remoteUser)
{
	LOGI("stop recv media in");
	//ֹͣ��Ƶ����ģ��
	if(remoteUser)
	{
		remoteUser->setAudioPlayer(NULL);
	}

	if(m_pVoiceRpm)
	{
		m_pVoiceRpm->StopPlayout(remoteUser->getAudioChanID());
	}

	LOGI("stop recv media end");
}

void  CLinkMicManage::stopAudioDecoderIfNeed()
{
	/*ֹͣ��Ƶ����ģ��
	���ﲻ��Ҫ��map��������Ϊ�ڵ��õĵط����Ѿ����ˣ�
	�ٴμ����ᵼ������
	*/
	/*
	if(m_pOpenRemoteUser_map.size() < 1 && m_pAudioPlay)
	{
		LOGI("this will stop audio decoder");
		m_pAudioPlay->Stop();
		m_pAudioPlay->setAudioPcmPlayer(NULL, -1);
		m_pAudioPlay->ReleaseConnections();
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}*/
}

void CLinkMicManage::StopAllRemoteMedia()
{
	LOGI("StopAll Remote Media begin");
	KAutoLock lock(m_mKCritSec);
	std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.begin();
	while (it!=m_pOpenRemoteUser_map.end())
	{
		LOGI("StopAll Remote Media 11");
		OpenRemoteUser *openRemoteUser=(*it).second;
		openRemoteUser->CloseAudioReceive();
		openRemoteUser->CloseVideoReceive();

		LOGI("StopAll Remote Media 12");
		openRemoteUser->ReleaseMediaSever();
		LOGI("StopAll Remote Media 13");
		stopReceiveMedia(openRemoteUser);

		delete openRemoteUser;
		LOGI("StopAll Remote Media 14");
		m_pOpenRemoteUser_map.erase(it++);
		LOGI("StopAll Remote Media 21");
	}

	stopAudioDecoderIfNeed();

	m_peertouchMoveViews.clear();
	LOGI("StopAll Remote Media end");
}

void CLinkMicManage::OpenRemoteUserVideo(const char* username, void *peerVideoView)
{
	std::string strusername = username;
	OpenRemoteUser* remoteUser = new OpenRemoteUser(false);
	remoteUser->ConnectMediaServer(m_serverip, 5566);

	CLIENTUSERINFOLIST TMPuserinfo;

	CLIENTUSERINFOLIST_MAP::iterator iter =m_UserInfoList.begin();
	while (iter!=m_UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;

		if(userinfo.strUserName  == strusername)
		{
			LOGI("open remote auio video, id:%s", username);
			void  *touchview = peerVideoView;
			remoteUser->OpenAudioReceive(userinfo.ulUserAudioID);
			if(m_bIsLinkUseVideo)
			{
				remoteUser->SetVideoWindow(peerVideoView);
				remoteUser->OpenVideoReceive(userinfo.ulUserAudioID+200);
			}
			
			m_peertouchMoveViews.push_back(peerVideoView);

			if (m_isAnchor)
			{
				int chanid = startReceiveMedia(remoteUser);
				remoteUser->setAudioChanID(chanid);
			}

			m_pOpenRemoteUser_map[userinfo.ulUserId] = remoteUser;
			break;
		}

		iter++;
	}
}

void CLinkMicManage::resetRemoteUserVideo(const char* username, void *peerVideoView)
{
	LOGI("reset Remote User Video in, id:%s", username);
	if(!m_bIsLinkUseVideo)
	{
		LOGI("there has not use video");
		return;
	}
	
	KAutoLock lock(m_mKCritSec);
	std::string strusername = username;

	CLIENTUSERINFOLIST_MAP::iterator iter =m_UserInfoList.begin();
	while (iter!=m_UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
		if(userinfo.strUserName  == strusername)
		{
			std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(userinfo.ulUserId);
			if (it!=m_pOpenRemoteUser_map.end())
			{
				OpenRemoteUser *remoteUser=(*it).second;
				LOGI("reopen remote video, id:%s", username);

				remoteUser->resetVideoWindow(peerVideoView);
				m_pOpenRemoteUser_map[userinfo.ulUserId] = remoteUser;
			}

			break;
		}
		iter++;
	}
	LOGI("reset Remote User Video out");
}


void CLinkMicManage::closeRemoteUserVideo(const char* username)
{
	KAutoLock lock(m_mKCritSec);
	std::string strusername = username;

	CLIENTUSERINFOLIST_MAP::iterator iter =m_UserInfoList.begin();
	while (iter!=m_UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
		if(userinfo.strUserName  == strusername)
		{
			std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(userinfo.ulUserId);
			if (it!=m_pOpenRemoteUser_map.end())
			{
				OpenRemoteUser *openRemoteUser=(*it).second;
				openRemoteUser->CloseAudioReceive();

				if(m_bIsLinkUseVideo)
				{
					openRemoteUser->CloseVideoReceive();
				}

				openRemoteUser->ReleaseMediaSever();
				stopReceiveMedia(openRemoteUser);
				delete openRemoteUser;
				m_pOpenRemoteUser_map.erase(it);
			}

			break;
		}
		iter++;
	}
	
	stopAudioDecoderIfNeed();

}

void CLinkMicManage::openLocalVideo(void *pVideoView)
{
	mlocalView = pVideoView;
	if(m_OpenLocalUser)
	{
		if(m_bIsLinkUseVideo)
		{
			m_OpenLocalUser->openLocalVideo(mlocalView);   
		}
	}
}

void CLinkMicManage::closeLocalVideo()
{
	if (m_OpenLocalUser)
	{
		if(m_bIsLinkUseVideo)
		{
			m_OpenLocalUser->closeLocalVideo();
		}
	}
	mlocalView = NULL;
}


