//#include "StdAfx.h"

//#import <Foundation/Foundation.h>
//#import "AudioUnitTool.h"
#include "OpenRemoteUser.h"
#include "ConnectServer.h"
#include "log.h"
#include "CVoiceManage.h"


#define USE_IOS 0

#ifndef YES
#define YES 1
#define NO  0
#endif

#define LOGIN_FUNC_NAME	"MultiVoiceLogin"
#define LOGIN_CODE_SUCC "SUCCESS"
#define LOGIN_CODE_OK "OK"

#define LOGIN_CODE_FAIL "FAILE"

#define SPEAK_FUNC_NAME	"MultiVoiceStatus"
#define SPEAK_CODE_YES	"1"
#define SPEAK_CODE_NO	"0"

//for test video
#define TEST_DISABLE_AUDIO 0

#define TEST_DISABLE_VIDEO 0


CVoiceManage::CVoiceManage(AIYOUVOICECALLBACK callback)
:m_uUserID(0),
m_strAddress(""),
m_serverip(""),
m_OpenLocalUser(NULL),
m_userGroupId(""),
m_isVideoCalling(false),
mlocalView(NULL),
mpeerView (NULL),
mhasVideo(true)
{
    XNetSetting::SetAudioProtocolType(XNetSetting::PT_UDP);
    XNetSetting::SetMCUOnly(0);
    m_pINetWorkCallback=NULL;
    m_callback = callback;
    m_UserInfoList.clear();
    m_ROOMaudioStatus = AUDIO_SEND_ENABLE;
    m_GROUPaudioStatus = AUDIO_SEND_ENABLE;
    m_OpenLocalUser =NULL;

	m_pAudioCap = NULL;
	m_pAudioPlay = NULL;
	m_pVoiceRpm = NULL;
}

CVoiceManage::~CVoiceManage(void)
{

}

int CVoiceManage::SendData(KCmdPacketEx& pPacket)
{
    return 0;
}

AUDIO_SEND_STATUS CVoiceManage::getAudioStatus()
{
    return m_ROOMaudioStatus;
}

void CVoiceManage::enableUpVolume(BOOL enable)
{
	LOGI("enable up volume flag:%d", enable);
	m_isEnableVolue = enable;
	if(m_pAudioCap)
	{
		m_pAudioCap->setVolumeParam(enable, 300, this);
	}
}


void CVoiceManage::startSendAudio(BOOL isspeakinroom)
{
	LOGI("start send audio, room flag:%d", isspeakinroom);
	int iRet = 0;
	if(m_isVideoCalling)
	{
		LOGE("this is video calling, status error");
		return;
	}

    if (isspeakinroom)
    {
        if(!mlistenInRoom)
        {
        	LOGI("this is not listen in room, cannot speak in room");
        	return;
        }

        if(m_ROOMaudioStatus==AUDIO_SEND_ENABLE&&m_GROUPaudioStatus!=AUDIO_SEND_SENDING)
        {
			const char *pData = "AUDIO_SEND_DISABLE";
            this->SendVoice(pData, strlen(pData), isspeakinroom);
            m_isSpeakinRoom = isspeakinroom;
        }
        else
        {
        	LOGI("there cannot speak in root, status:%d,%d", m_ROOMaudioStatus, m_GROUPaudioStatus);
            iRet = -1;
        }
    }
    else
    {
        if(m_GROUPaudioStatus==AUDIO_SEND_ENABLE&&m_ROOMaudioStatus!=AUDIO_SEND_SENDING)
        {
			const char *pData = "AUDIO_SEND_DISABLE";
            this->SendVoice(pData, strlen(pData), isspeakinroom);
            m_isSpeakinRoom = isspeakinroom;
        }
        else
        {
            LOGI("there cannot speak in group, status:%d,%d", m_ROOMaudioStatus, m_GROUPaudioStatus);
            iRet = -1;
        }
    }

	if(iRet == 0)
	{
		OpenRemoteUser::setAudioPlayFlag(false);
	}

	return;
}

void CVoiceManage::stopSendAudio()
{
    LOGI("stopSend Audio in");
        if(m_isVideoCalling)
        return;


    if (m_isSpeakinRoom) {
		const char *pData = "AUDIO_SEND_ENABLE";
        this->SendVoice(pData, strlen(pData), YES);
    }
    else
    {
		const char *pData = "AUDIO_SEND_ENABLE";
        this->SendVoice(pData, strlen(pData), NO);
    }

    if(m_ROOMaudioStatus==AUDIO_SEND_SENDING)
    {
        //[[AudioUnitTool shareHandle]stop];


        m_ROOMaudioStatus = AUDIO_SEND_ENABLE;
    }
    else if(m_GROUPaudioStatus==AUDIO_SEND_SENDING)
    {
        //[[AudioUnitTool shareHandle]stop];


        m_GROUPaudioStatus = AUDIO_SEND_ENABLE;
    }

	OpenRemoteUser::setAudioPlayFlag(true);
    this->closeMediaSender(m_isVideoCalling);
    LOGI("stopSend Audio end");
}

int CVoiceManage::ConnectServer(const char* szIP,unsigned int nPort,bool isEncrypt)
{
	int iRet = m_ConnectServer.checkIsDomain(szIP);
    	if(iRet == 0)
	{
		char addr[IP_SIZE] = {0};
		iRet = m_ConnectServer.getIpByDomain(szIP, addr);
		LOGI("ptt, %s:is domain, ip is:%s", szIP, iRet==0 ? addr : "get ip failed");
        	m_serverip = addr;
	}
	else if(iRet == 1)
	{
		LOGI("ptt, %s:is ip addr", szIP);
        	m_serverip = szIP;
	}
	else
	{
		LOGI("ptt, %s:unknow this is what, maybe it is empty", szIP);
        	m_serverip = szIP;
	}

	return m_ConnectServer.Start(m_serverip.c_str(),nPort,isEncrypt,this);
}


void CVoiceManage::disconnect()
{
    m_ConnectServer.Stop();
}

void CVoiceManage::On_SessionConnectStatus(CONNECT_STATUS cs)
{
	LOGI("CVoice Manage, On SessionConnectStatus in, conn status:%d", cs);

    if(cs == CS_LOGINED||cs == CS_RECONNECTED)
    {

		cb2UiCallback(0,YES, LOGIN_FUNC_NAME, LOGIN_CODE_OK, "", "");

        this->getUserList();
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);


    }
    else if(cs == CS_DISCONNECTED||cs == CS_LOGINFAILED)
    {
        cb2UiCallback(-1,NO, LOGIN_FUNC_NAME, LOGIN_CODE_FAIL, "", "");
		LOGI("will call this.StopLocal Media");

        this->closeMediaSender(m_isVideoCalling);
        this->StopAllRemoteMedia(m_isVideoCalling);
    }
    else if(cs == CS_CONNECTED||cs == CS_RECONNECTED)
    {
        this->LoginServer( m_username.c_str() ,"");
    }

    if(m_pINetWorkCallback)
    {
        m_pINetWorkCallback->IConnectStatusCallback(cs);
    }
}

void CVoiceManage::setNetWorkCallback(INetWorkCallback*netWorkCallback)
{
    m_pINetWorkCallback   =netWorkCallback;
}

void CVoiceManage::OnDispatchCmd(KCmdPacketEx& pPacket)
{
	std::string strCMD = pPacket.GetCMD();
	LOGI("ondispatch cmd in, cmd:%s", strCMD.c_str());
    if(strCMD=="LOGINSERVERED")
    {
		m_uUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
		unsigned long ulErrorType = pPacket.GetAttrib("ERRORTYPE").AsUnsignedLong();
		if(ulErrorType == 0)
		{
			cb2UiCallback(0,YES, LOGIN_FUNC_NAME, LOGIN_CODE_OK, "", "");
			this->getUserList();

			if(m_pVoiceRpm == NULL)
			{
				m_pVoiceRpm = AiyouVoiceRpm::getInstance();
				m_pVoiceRpm->Init();
			}

			if(m_pINetWorkCallback)
				m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);
		}
		else
		{
			cb2UiCallback(-1,NO, LOGIN_FUNC_NAME, LOGIN_CODE_FAIL, "", "");

			if(m_pINetWorkCallback)
				m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINFAILED);
		}
    }
    else if(strCMD=="RELOGIN")
    {
        unsigned long  userid= pPacket.GetAttrib("USERID").AsUnsignedLong();
        LOGI("ptt your number logined from other device");
        if(m_pINetWorkCallback)
                m_pINetWorkCallback->IConnectStatusCallback(CS_RELOGIN);
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
	usrinfo.strHeadUrl    =  pPacket.GetAttrib("HEADURL").AsString();
	usrinfo.strNickName   =  pPacket.GetAttrib("NICKNAME").AsString();
	usrinfo.ulLatitude    =  pPacket.GetAttrib("LATITUDE").AsDouble();
	usrinfo.ulLongitude   =  pPacket.GetAttrib("LONGITUDE").AsDouble();
        m_UserInfoList[ulUserID] = usrinfo;
        if(m_pINetWorkCallback)
        {
            m_pINetWorkCallback->INetReceiveUserLogin(&usrinfo);
        }

    }
    else if(strCMD=="REMOTEUSERQUIT")
    {

        KAutoLock lock(m_mKCritSec);

        unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        std::string strExpand = pPacket.GetAttrib("EXPAND").AsString();

        CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.find(ulUserID);

	if(iter != m_UserInfoList.end())
	{
		if(m_pINetWorkCallback)
		{
			CLIENTUSERINFOLIST userinfo =iter->second;
			m_pINetWorkCallback->INetReceiveUserLogOut(&userinfo);
		}
		m_UserInfoList.erase(iter);
	}

        if(strUserName.compare(m_RoomSpeakerName)==0)
        {
            this->m_ROOMaudioStatus = AUDIO_SEND_ENABLE;
            cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_YES, strUserName.c_str(), "true");
            m_RoomSpeakerName = "";


            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetAudioStatus(ulUserID,YES,AUDIO_SEND_ENABLE,strUserName);
        }

        if(strUserName.compare(m_GroupSpeakerName)==0)
        {
            this->m_GROUPaudioStatus = AUDIO_SEND_ENABLE;
			cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_YES, strUserName.c_str(), "false");

            m_GroupSpeakerName = "";


            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetAudioStatus(ulUserID,NO,AUDIO_SEND_ENABLE,strUserName);
        }

        if (m_videoCallUserinfo.ulUserId == ulUserID)
        {
            closeMediaSender(true);
                std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(m_videoCallUserinfo.ulUserId);
            if (it!=m_pOpenRemoteUser_map.end())
            {
                OpenRemoteUser *openRemoteUser=(*it).second;
                openRemoteUser->CloseAudioReceive();

                openRemoteUser->ReleaseMediaSever();
                delete openRemoteUser;
                m_pOpenRemoteUser_map.erase(it);
            }
            m_videoCallUserinfo.ulUserId = 0;
        }
    }

    else if(strCMD=="UPDATEUSERLIST")
    {
        KAutoLock lock(m_mKCritSec);
        CMD_ITEM_LST lstItems = pPacket.GetItemList();
        for(CMD_ITEM_LST::const_iterator it=lstItems.begin();it!=lstItems.end();it++)
        {
            KCmdItem item((std::string)*it);
            CLIENTUSERINFOLIST usrinfo;
            usrinfo.strUserName = item.GetAttrib("USERNAME").AsString();
            usrinfo.ulUserId =  item.GetAttrib("USERID").AsUnsignedLong();
            usrinfo.ulUserAudioID = item.GetAttrib("USERAUDIOID").AsUnsignedLong();
            usrinfo.strHeadUrl    =  item.GetAttrib("HEADURL").AsString();
            usrinfo.strNickName   =  item.GetAttrib("NICKNAME").AsString();
            usrinfo.ulLatitude    =  item.GetAttrib("LATITUDE").AsDouble();
            usrinfo.ulLongitude   =  item.GetAttrib("LONGITUDE").AsDouble();
            //            printf(" usrinfo.ulUserAudioID = %d", usrinfo.ulUserAudioID);
            m_UserInfoList[usrinfo.ulUserId] = usrinfo;
        }
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->INetReceiveUserList(m_UserInfoList);

    }
    else if(strCMD=="SIGNALINGTRANSFER")
    {
        bool isbroadcast = pPacket.GetAttrib("BROADCAST").AsBoolean();
        std::string strData = pPacket.GetAttrib("DATA").AsString();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();

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
                m_pINetWorkCallback->INetReceiveData(ulUserID, strData.c_str(), strData.size());
        }
    }
    else if(strCMD=="VIDEOCALL")
    {
        std::string strData = pPacket.GetAttrib("DATA").AsString();
        LOGI("VIDEO CALL cmd = %s\n",strData.c_str());
        OnDispatchVideoCall(pPacket, strData);
    }
    else if(strCMD=="SENDVOICE")
    {
    	 LOGI("recv SEND VOICE cmd, this will send voice:%d", m_isVideoCalling);
	if(m_isVideoCalling)
	{
		return;
	}
        KAutoLock lock(m_mKCritSec);
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        std::string strGroupid = pPacket.GetAttrib("GROUPID").AsString();
        bool isinroom = pPacket.GetAttrib("ISINROOM").AsBoolean();
        std::string strAddress = pPacket.GetAttrib("ADDRESS").AsString();
        std::string strData = pPacket.GetAttrib("DATA").AsString();
        std::string strExpand = pPacket.GetAttrib("EXPAND").AsString();
        LOGE("param:%s,%s,%d,%s,%s,%s", strUserName.c_str(), strGroupid.c_str(), isinroom, strAddress.c_str(), strData.c_str(), strExpand.c_str());
	if(isinroom)
	{
		if (m_strAddress.compare(strAddress) !=0)
		{
			LOGE("the addr is same myself");
			return;
		}
	}
	else
	{
		if (m_strAddress.compare(strAddress) !=0||m_userGroupId.compare(strGroupid)!=0 )
		{
			LOGE("the addr is same myself or groupid");
			return;
		}
	}
        unsigned long ulUserID = pPacket.GetAttrib("PEERUSERID").AsUnsignedLong();

        if (strData.compare("AUDIO_SEND_ENABLE") == 0)
        {
        	LOGE("the addr is same myself or groupid");
            if(strUserName.compare(m_username)==0)
            {
                if(isinroom)
                {
                    m_ROOMaudioStatus = AUDIO_SEND_SENDING;
                }
                else
                {
                    m_GROUPaudioStatus = AUDIO_SEND_SENDING;
                }
                this->openMediaSender(m_isVideoCalling);
                cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_NO, strUserName.c_str(), isinroom ? "true" : "false");
                LOGE("the will return");
                return;
            }

		if(isinroom)
		{
			this->m_ROOMaudioStatus = AUDIO_SEND_ENABLE;
			m_RoomSpeakerName="";
			cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_YES, strUserName.c_str(), "true");
		}
		else
		{
			this->m_GROUPaudioStatus = AUDIO_SEND_ENABLE;
			m_GroupSpeakerName="";
			cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_YES, strUserName.c_str(), "false");
		}

            if(m_ROOMaudioStatus !=AUDIO_SEND_DISABLE &&m_GROUPaudioStatus !=AUDIO_SEND_DISABLE&&m_pOpenRemoteUser_map.size()>1)
            {
                StopAllRemoteMedia(m_isVideoCalling);
            }
            else
            {
                unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
                std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(ulUserID);

                if (it!=m_pOpenRemoteUser_map.end())
                {
                    OpenRemoteUser *openRemoteUser=(*it).second;
                    openRemoteUser->CloseAudioReceive();
                    openRemoteUser->ReleaseMediaSever();
                    delete openRemoteUser;
                    m_pOpenRemoteUser_map.erase(it);
                }
            }

            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetAudioStatus(ulUserID,isinroom,AUDIO_SEND_ENABLE,strUserName);
            return;
        }
        else if(strData.compare("AUDIO_SEND_DISABLE") == 0)
        {
            if(strUserName.compare(m_username)==0)
            {

                return;
            }
		OpenRemoteUser::setAudioPlayFlag(true);

            if(isinroom)
            {
                if(!mlistenInRoom)
                    return;

                m_RoomSpeakerName=strUserName;
		   cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_NO, strUserName.c_str(), "true");
                this->m_ROOMaudioStatus = AUDIO_SEND_DISABLE;
            }
            else
            {
                m_GroupSpeakerName=strUserName;
		   cb2UiCallback(0,YES, SPEAK_FUNC_NAME, SPEAK_CODE_NO, strUserName.c_str(),"false");
                this->m_GROUPaudioStatus = AUDIO_SEND_DISABLE;

            }
            std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(ulUserID);

            if (it!=m_pOpenRemoteUser_map.end())
            {
                OpenRemoteUser *openRemoteUser=(*it).second;
                openRemoteUser->CloseAudioReceive();

                openRemoteUser->ReleaseMediaSever();
                delete openRemoteUser;
                m_pOpenRemoteUser_map.erase(it);
            }

            OpenRemoteUser* openRemoteUser = new OpenRemoteUser(false);
            openRemoteUser->ConnectMediaServer(m_serverip, 5566);

            unsigned int ulUseraudioID = pPacket.GetAttrib("USERAUDIOID").AsUnsignedInt();
            openRemoteUser->OpenAudioReceive(ulUseraudioID);
            unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
            m_pOpenRemoteUser_map[ulUserID] = openRemoteUser;

		StartReceiveMedia(openRemoteUser, false); //add by chenzh

            if(m_pINetWorkCallback)
                m_pINetWorkCallback->INetAudioStatus(ulUserID,isinroom,AUDIO_SEND_DISABLE,strUserName);
            return;
        }

        if(m_pINetWorkCallback)
            m_pINetWorkCallback->INetReceiveData(ulUserID, strData.c_str(), strData.size());
    }
}


int CVoiceManage::LoginServer(const char* szName, const char* szPassword)
{
    if(!szName || !szPassword)
        return -1;

    m_username = szName;
    m_ROOMaudioStatus=AUDIO_SEND_ENABLE;
    m_GROUPaudioStatus=AUDIO_SEND_ENABLE;
    m_GroupSpeakerName="";
    m_RoomSpeakerName = "";
    srand((unsigned)time(NULL));
    //unsigned int time=[[NSDate date] timeIntervalSince1970]+rand()%1000000;
    //this->m_videoID=time-1400000000 ;
	long timeCurr=time(NULL);
    this->m_audioID=timeCurr-1400000000;
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

    std::string headUrl = "HEADURL";
    rPacket.SetAttrib(headUrl, m_headUrl);

    std::string nickName = "NICKNAME";
    rPacket.SetAttrib(nickName, m_nickName);


    std::string latitude = "LATITUDE";
    rPacket.SetAttribUL(latitude, m_latitude);

    std::string longitude = "LONGITUDE";
    rPacket.SetAttribUL(longitude, m_longitude);


    return m_ConnectServer.SendData(rPacket);
}

int CVoiceManage::logoutServer()
{
	LOGI("will logout");
    this->stopSendAudio();
    this->closeMediaSender(m_isVideoCalling);
    this->StopAllRemoteMedia(m_isVideoCalling);
    m_UserInfoList.clear();
    std::string strGetuserlist= "quitserver";
    KCmdPacketEx rPacket(strGetuserlist.c_str(),(int)strGetuserlist.length()+1);
    std::string strCMD = "QUITSERVER";
    rPacket.SetCMD(strCMD);
    return m_ConnectServer.SendData(rPacket);
}

int CVoiceManage::getUserList()
{
    std::string strGetuserlist= "getuserlist";
    KCmdPacketEx rPacket(strGetuserlist.c_str(),(int)strGetuserlist.length()+1);
    rPacket.SetAttrib("ADDRESS", m_strAddress);
    std::string strCMD = "GETUSERLIST";
    rPacket.SetCMD(strCMD);
    return m_ConnectServer.SendData(rPacket);
}

void CVoiceManage::loginVoiceServer(const char* szIP,const char* szName ,const char* szGameId,const char* szGameServerId,
    								const char* szRoomId,const char* szGroupId,BOOL listenInRoom,const char* szexpand,bool isencrypt,
    								const char* headUrl,const char* nickName,double latitude,double longitude)
{
    if(!szIP||!szName||!szGameId||!szGameServerId||!szRoomId)
    {
		LOGE("login server, input is null");
		cb2UiCallback(-1,NO, LOGIN_FUNC_NAME, LOGIN_CODE_FAIL, "unknown", "");
		return;
    }

    m_szexpand = szexpand;

    mlistenInRoom = listenInRoom;
    char sAddress[255];
    sprintf(sAddress,"%s-%s-%s",szGameId,szGameServerId,szRoomId);
    m_strAddress = sAddress;
    m_username = szName;
    m_headUrl  = headUrl;
    m_nickName = nickName;
    m_latitude = latitude;
    m_longitude = longitude;

    if(!szGroupId)
    {
        m_userGroupId = "";
    }
    else
    {
        m_userGroupId = szGroupId;
    }

    ConnectServer(szIP,REMOTE_MCUPort,isencrypt);
}

int CVoiceManage::SendVoice(const char* pData, unsigned long nLen,bool isinroom )
{
    if(!pData || nLen == 0)
	{
		LOGI("send voice parm error:%d", nLen);
		return -1;
	}
    //��������ȴ�������Ϣ��
    if (!isinroom&&m_userGroupId.compare("")==0)
    {
    	 LOGI("send voice failed, input parm:%d", isinroom);
        return -1;
    }

    std::string strData(pData, nLen);
    std::string strinfo = "SENDVOICE";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SENDVOICE";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("ISINROOM", isinroom);
    rPacket.SetAttrib("DATA", strData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttrib("USERNAME", m_username);
    rPacket.SetAttrib("ADDRESS", m_strAddress);
    rPacket.SetAttrib("GROUPID", m_userGroupId);
    rPacket.SetAttribUL("USERAUDIOID", m_audioID);
    rPacket.SetAttrib( "EXPAND", m_szexpand);

    return m_ConnectServer.SendData(rPacket);
}

int CVoiceManage::SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen)
{
    if(!pData || nLen == 0)
        return -1;

    std::string strData(pData, nLen);
    std::string strinfo = "Signalingtransfer";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SIGNALINGTRANSFER";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("BROADCAST", false);
    rPacket.SetAttribUL("PEERUSERID", uPeerUserID);
    rPacket.SetAttrib("DATA", strData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttrib("USERNAME", m_username);
    return m_ConnectServer.SendData(rPacket);
}

int CVoiceManage::SendAllUserData(const char* pData, unsigned long nLen)
{
    if(!pData || nLen == 0)
        return -1;

    std::string strData(pData, nLen);
    std::string strinfo = "Signalingtransfer";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "SIGNALINGTRANSFER";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("BROADCAST", true);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttribUL("USERAUDIOID", this->m_audioID);
    rPacket.SetAttrib("DATA", strData);
    return m_ConnectServer.SendData(rPacket);
}

std::string CVoiceManage::GetLocalIP()
{
    return m_ConnectServer.GetLocalIP();
}

std::string CVoiceManage::GetNATIP()
{
    return m_ConnectServer.GetNATIP();
}

void CVoiceManage::SendAudioData(char*pData, int nLen)
{
    if (m_OpenLocalUser&&( m_ROOMaudioStatus==AUDIO_SEND_SENDING||m_GROUPaudioStatus == AUDIO_SEND_SENDING))
    {
        m_OpenLocalUser->SendAudioData(pData, nLen);
    }
    else if(m_OpenLocalUser&&m_isVideoCalling)
    {
        m_OpenLocalUser->SendAudioData(pData, nLen);
    }
}

void CVoiceManage::openMediaSender(bool isVideoCalling)
{
	LOGI("open media sender in:%d", isVideoCalling);
    KAutoLock lock(m_mKCritSec);
    if (!m_OpenLocalUser)
    {
        m_OpenLocalUser = new OpenLocalUser;
        m_OpenLocalUser->ConnectMediaServer(m_serverip, 5566);
        m_OpenLocalUser->OpenAudioSend(this->m_audioID);
        m_OpenLocalUser->OpenVideoSend(this->m_audioID+200);
        if(!isVideoCalling)
            openLocalAudio(isVideoCalling);
    }
}

void CVoiceManage::closeMediaSender(bool isVideoCalling)
{
    KAutoLock lock(m_mKCritSec);
    if (m_OpenLocalUser != NULL)
    {
        closeLocalAudio(isVideoCalling);
        m_OpenLocalUser->CloseAudioSend();
        m_OpenLocalUser->CloseVideoSend();
        m_OpenLocalUser->ReleaseMediaSever();

        if (m_OpenLocalUser)
        {
            m_OpenLocalUser->closeLocalVideo();
        }

        delete m_OpenLocalUser;
        m_OpenLocalUser = NULL;
    }
}

void CVoiceManage::closeLocalAudio(bool isVideoCalling)
{
	if(TEST_DISABLE_AUDIO)
		return;

	LOGI("closeLocal Audio in");
	KAutoLock lock(m_mKCritSec);

	int iD = 0;
	if(m_pAudioCap)
	{
		iD = m_pAudioCap->mAyVcID;
	       if(m_pVoiceRpm)
		{
			m_pVoiceRpm->StopCapture(iD);
		}
		m_pAudioCap->Stop();
		m_pAudioCap->ReleaseConnections();

		delete m_pAudioCap;
		m_pAudioCap = NULL;
	}

	LOGI("closeLocal Audio end");
}

void CVoiceManage::openLocalAudio(bool isVideoCalling)
{
	if(TEST_DISABLE_AUDIO)
		return;

    LOGI("open local audio:%d,%x", isVideoCalling, m_pAudioCap != NULL ? m_pAudioCap : 0);
	if(m_pAudioCap == NULL)
	{
		m_pAudioCap = new CAudioCapChanle(true, mSampleRate, this);
		int iSampleBits = 16;// ��Ƶ�������
		int iInterval = 20;	//20ms����Ƶ��������
		int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
		m_pAudioCap->Connect(mChannels, mSampleRate, iBytes, mBitRate);
		m_pAudioCap->Start();

		if(m_isEnableVolue)
		{
			m_pAudioCap->setVolumeParam(m_isEnableVolue, 300, this);
		}

		//m_pAudioCap->setPause(true);
		if(m_pVoiceRpm)
		{
			m_pVoiceRpm->StartCapture(m_pAudioCap);
		}
	}
}

void CVoiceManage::StartReceiveMedia(OpenRemoteUser* openRemoteUser, bool isVideoCalling)
{
	if(TEST_DISABLE_AUDIO)
		return;

	LOGI("start recv media in");
	//��Ƶ���벥����

	if(m_pAudioPlay)
	{
		LOGI("start recv media, player is not null, release it first");
		m_pAudioPlay->Stop();
		m_pAudioPlay->ReleaseConnections();
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}

	m_pAudioPlay = new CAudioPlayChanle();
	int iSampleBits = 16;// ��Ƶ�������
	int iInterval = 20;	//20ms����Ƶ��������
	int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
	m_pAudioPlay->Connect(mChannels, mSampleRate, iBytes, mBitRate);
	m_pAudioPlay->Start();

	int chanid = -1;

	if(m_pVoiceRpm)
	{
		m_pAudioPlay->setAudioPcmPlayer(m_pVoiceRpm, chanid);
		chanid = m_pVoiceRpm->StartPlayout(mSampleRate);
	}

	if(openRemoteUser)
	{
		openRemoteUser->setAudioPlayer(m_pAudioPlay);
		openRemoteUser->setAudioChanID(chanid);
	}

#if 0
    OpenRemoteUser* m_OpenRemoteUser = new OpenRemoteUser(isVideoCalling);
    m_OpenRemoteUser->ConnectMediaServer(m_serverip, 5566);
    m_OpenRemoteUser->OpenAudioReceive(this->m_audioID);
    
    m_pOpenRemoteUser_map[this->m_uUserID] = m_OpenRemoteUser;
#endif

	//LOGI("ConnectMedia Server ret:%d, OpenAudio Receive ret:%d remote obj:%p", bRet1, bRet2, pRemoteUser);
	LOGI("ConnectMedia Server end");
}

void CVoiceManage::StopReceiveMediaAndroid(OpenRemoteUser* openRemoteUser)
{
	if(TEST_DISABLE_AUDIO)
		return;

	LOGI("stop recv media in");
	//��Ƶ���벥����
	if(m_pAudioPlay)
	{
		LOGI("start recv media, player is not null, release it first");
		m_pAudioPlay->Stop();
		m_pAudioPlay->setAudioPcmPlayer(NULL, -1);
		m_pAudioPlay->ReleaseConnections();
		delete m_pAudioPlay;
		m_pAudioPlay = NULL;
	}

	int iChanID = -1;
	if(openRemoteUser)
	{
		iChanID = openRemoteUser->getAudioChanID();
		openRemoteUser->setAudioPlayer(NULL);
	}

	if(m_pVoiceRpm)
	{
		m_pVoiceRpm->StopPlayout(iChanID);
	}

	LOGI("stop recv media end");
}


void CVoiceManage::StopAllRemoteMedia(bool isVideoCalling)
{
    LOGI("StopAll RemoteMedia in");
    KAutoLock lock(m_mKCritSec);
    std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.begin();
    while (it!=m_pOpenRemoteUser_map.end())
    {
        OpenRemoteUser *openRemoteUser=(*it).second;
        openRemoteUser->CloseAudioReceive();
        openRemoteUser->ReleaseMediaSever();
        delete openRemoteUser;
        m_pOpenRemoteUser_map.erase(it++);
    }
}

void CVoiceManage::OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen)
{
	//LOGI("CVoiceManage in, on auido opus notify, len:%d", nLen);
	this->SendAudioData((char*)pPcm, nLen);
}

void CVoiceManage::setAudioCapParam(int iChannels, int iSampleRate, int iBitRate)
{
	LOGI("set audio cap param,channel,samp,bit:%d,%d,%d", iChannels, iSampleRate, iBitRate);
	mChannels	= iChannels;
	mSampleRate = iSampleRate;
	mBitRate	= iBitRate;
}

void CVoiceManage::OnAudioVolumeOutput(int volume)
{
	if(m_pINetWorkCallback)
	{
		m_pINetWorkCallback->INetVolumeChanged(volume);
	}
}


int CVoiceManage::cb2UiCallback(int msgcode, bool issucceed, const char *funtype, const char *status, const char *username, const char * inRoom)
{
	if(m_callback)
	{
		m_callback(msgcode, issucceed, funtype, status, username, inRoom);
	}
    return 0;
}

int CVoiceManage::SetAndroidObjects(void* javaVM, void* env, void* context)
{
	AiyouVoiceRpm::SetAndroidObjects(javaVM, env, context);
	return 0;
}

void CVoiceManage::sendVideoCallMsg(unsigned long userid, const char*msg)
{
    std::string strinfo = "VIDEOCALL";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "VIDEOCALL";
    rPacket.SetCMD(strCMD);
    std::string strData =  msg;
    rPacket.SetAttrib("DATA", strData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttribUL("PEERUSERID", userid);
    m_ConnectServer.SendData(rPacket);
}

void CVoiceManage::inviteVideoCall(unsigned long userid,bool hasVideo)
{
    std::string strinfo = "VIDEOCALL";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "VIDEOCALL";
    rPacket.SetCMD(strCMD);
    std::string strData =  "INVITE";
    rPacket.SetAttribBL("HASVIDEO", hasVideo);
    rPacket.SetAttrib("DATA", strData);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttribUL("PEERUSERID", userid);
    rPacket.SetAttrib("USERNAME", m_username);
    rPacket.SetAttribUL("USERAUDIOID", m_audioID);
    m_isVideoCalling = true;
    mhasVideo = hasVideo;
    m_ConnectServer.SendData(rPacket);
}

void CVoiceManage::OnDispatchVideoCall(KCmdPacketEx& pPacket,string videocmd)
{
	LOGI("ondispatch video call in, cmd:%s", videocmd.c_str());
 if(videocmd == "INVITE")
    {
        if(m_isVideoCalling)
        {
            sendVideoCallMsg(m_videoCallUserinfo.ulUserId,"VIDEOCALLING");
            return;
        }

        if (m_videoCallUserinfo.ulUserId>0)
        {
            return;
        }
        m_videoCallUserinfo.strUserName = pPacket.GetAttrib("USERNAME").AsString();
        m_videoCallUserinfo.ulUserId = pPacket.GetAttrib("USERID").AsUnsignedLong();
        m_videoCallUserinfo.ulUserAudioID = pPacket.GetAttrib("USERAUDIOID").AsUnsignedLong();
        mhasVideo = pPacket.GetAttrib("HASVIDEO").AsBoolean();
        m_isVideoCalling = true;
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->INetVideoCall(m_videoCallUserinfo.ulUserId, mhasVideo,  m_videoCallUserinfo.strUserName);

    }
    else if(videocmd == "ACCEPT")
    {
        unsigned long ulUserId = pPacket.GetAttrib("USERID").AsUnsignedLong();
        if(m_videoCallUserinfo.ulUserId != ulUserId)
        {
            LOGE("ulUserId != caller \n");
            return;
        }

        m_isVideoCalling = true;
        openLocalAudio(m_isVideoCalling);
        OpenRemoteUser* openRemoteUser = new OpenRemoteUser(true);
        openRemoteUser->ConnectMediaServer(m_serverip, 5566);
        unsigned long ulUseraudioID = pPacket.GetAttrib("USERAUDIOID").AsUnsignedLong();
        m_videoCallUserinfo.ulUserAudioID = ulUseraudioID;
        LOGI("ulUseraudioID = %lu",ulUseraudioID);
        openRemoteUser->OpenAudioReceive(ulUseraudioID);
        StartReceiveMedia(openRemoteUser, true); //add by chenzh
        if (mhasVideo )
        {
            if (mlocalView)
            {
                m_OpenLocalUser->openLocalVideo(mlocalView);
            }
            if (mpeerView && !TEST_DISABLE_VIDEO)
            {
                openRemoteUser->OpenVideoReceive(ulUseraudioID+200);

                openRemoteUser->SetVideoWindow(mpeerView);
            }
        }
        unsigned int ulUserID = m_videoCallUserinfo.ulUserId;
        m_pOpenRemoteUser_map[ulUserID] = openRemoteUser;

		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->INetCallConnected();
		}
    }
    else if(videocmd == "VIDEOCALLING")
    {
        closeMediaSender(true);
        m_isVideoCalling = false;
    }
    else if(videocmd == "CANCEL")
    {
        closeMediaSender(true);
        m_isVideoCalling = false;
		m_videoCallUserinfo.ulUserId = 0;
		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->INetCallDisConn(0, "peer cancel");
		}
    }
    else if(videocmd == "REFUSE")
    {
        closeMediaSender(true);
        m_isVideoCalling = false;
		m_videoCallUserinfo.ulUserId = 0;
		if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->INetCallDisConn(1, "peer refuse");
		}
    }
    else if(videocmd == "HANGUP")
    {
        if (m_videoCallUserinfo.ulUserId>0)
        {
            closeMediaSender(true);
            std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(m_videoCallUserinfo.ulUserId);

            if (it!=m_pOpenRemoteUser_map.end())
            {
			LOGI("will close media recver");
			OpenRemoteUser *openRemoteUser=(*it).second;
			openRemoteUser->CloseAudioReceive();

			openRemoteUser->ReleaseMediaSever();
			delete openRemoteUser;

			m_pOpenRemoteUser_map.erase(it);
			StopReceiveMediaAndroid(openRemoteUser); //add by chenzh
            }
        	if(m_pINetWorkCallback)
		{
			m_pINetWorkCallback->INetCallDisConn(2, "peer hangup");
		}
        }
        m_videoCallUserinfo.ulUserId = 0;
        m_isVideoCalling = false;
    }
    else
    {
        printf("video call invalid cmd");
    }
}

void CVoiceManage::invite(bool isVideo,const char*username,void* localVideo,void* peerVideo)
{
    LOGI("call out in, name:%s, %d, %x, %x", username, isVideo,localVideo,peerVideo);
    if (m_videoCallUserinfo.ulUserId>0||m_isVideoCalling)
    {
        LOGI("userid already > 0, or callin doing(%d)", m_isVideoCalling);
        return;
    }

    std::string strusername = username;
    if(strusername == m_username)
    {
        LOGE("cannot call myself");
        return;
    }
    CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.begin();

    for (;iter !=m_UserInfoList.end();iter++)
    {
        CLIENTUSERINFOLIST userinfo =iter->second;
        if (userinfo.strUserName ==strusername )
        {
            m_videoCallUserinfo.strUserName = userinfo.strUserName;
            m_videoCallUserinfo.ulUserId =  userinfo.ulUserId;
            m_videoCallUserinfo.ulUserAudioID = userinfo.ulUserAudioID;
            LOGI("callout, ulUseraudioID = %lu",m_videoCallUserinfo.ulUserAudioID);
            mlocalView = localVideo;
            mpeerView = peerVideo;
            openMediaSender(true);
            inviteVideoCall((unsigned long)iter->first,isVideo);
        }
    }
}

void CVoiceManage::accept(void* localVideo,void* peerVideo)
{
    LOGI("accept call, %x, %x", localVideo, peerVideo);
    mlocalView = localVideo;
    mpeerView = peerVideo;

    if (m_videoCallUserinfo.ulUserId>0)
    {
        openMediaSender(true);
        std::string strinfo = "VIDEOCALL";
        KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
        std::string strCMD = "VIDEOCALL";
        rPacket.SetCMD(strCMD);
        std::string strData =  "ACCEPT";
        rPacket.SetAttrib("DATA", strData);
        rPacket.SetAttribUL("USERID", m_uUserID);
        rPacket.SetAttribUL("PEERUSERID", m_videoCallUserinfo.ulUserId);
          rPacket.SetAttribUL("USERAUDIOID", m_audioID);

        m_ConnectServer.SendData(rPacket);
        openLocalAudio(m_isVideoCalling);
        OpenRemoteUser* openRemoteUser = new OpenRemoteUser(true);
        openRemoteUser->ConnectMediaServer(m_serverip, 5566);

        unsigned int ulUseraudioID = m_videoCallUserinfo.ulUserAudioID;
        openRemoteUser->OpenAudioReceive(ulUseraudioID);
 	StartReceiveMedia(openRemoteUser, true);//add by chenzh
        if (mhasVideo )
        {
            if (mlocalView)
            {
                m_OpenLocalUser->openLocalVideo(mlocalView);
            }

            if (mpeerView && !TEST_DISABLE_VIDEO)
            {
                openRemoteUser->SetVideoWindow(mpeerView);
                openRemoteUser->OpenVideoReceive(ulUseraudioID+200);
            }
        }
        unsigned int ulUserID = m_videoCallUserinfo.ulUserId;
        m_pOpenRemoteUser_map[ulUserID] = openRemoteUser;
    }
}

void CVoiceManage:: cancel()
{
    if (m_videoCallUserinfo.ulUserId>0)
    {
        sendVideoCallMsg(m_videoCallUserinfo.ulUserId,"CANCEL");
        closeMediaSender(true);
        m_videoCallUserinfo.ulUserId = 0;
                m_isVideoCalling = false;
    }
}

void CVoiceManage:: refuse()
{
    if (m_videoCallUserinfo.ulUserId>0)
    {
        sendVideoCallMsg(m_videoCallUserinfo.ulUserId,"REFUSE");
        closeMediaSender(true);
        m_videoCallUserinfo.ulUserId = 0;
                m_isVideoCalling = false;
    }
}

void CVoiceManage:: hangup()
{
	if (m_videoCallUserinfo.ulUserId>0)
	{
		closeMediaSender(true);
		sendVideoCallMsg(m_videoCallUserinfo.ulUserId,"HANGUP");
		std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(m_videoCallUserinfo.ulUserId);
		if (it!=m_pOpenRemoteUser_map.end())
		{
			LOGI("hangup, will close media recver");
			OpenRemoteUser *openRemoteUser=(*it).second;
			openRemoteUser->CloseAudioReceive();

			openRemoteUser->ReleaseMediaSever();
			delete openRemoteUser;
			m_pOpenRemoteUser_map.erase(it);

			StopReceiveMediaAndroid(openRemoteUser); //add by chenzh
		}
		m_videoCallUserinfo.ulUserId = 0;
		m_isVideoCalling = false;
	}
}
