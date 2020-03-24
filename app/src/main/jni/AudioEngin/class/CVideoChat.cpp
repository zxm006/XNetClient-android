#include "CVideoChat.h"
//#import <Foundation/Foundation.h>
//#import "AudioUnitTool.h"
//#import "AudioUnitAecTool.h"
#include "OpenRemoteUser.h"
#include "ConnectServer.h"
#include "AutoLock.h"
//#import "TouchMoveView.h"
//#import <dispatch/queue.h>

CVideoChat::CVideoChat()
:m_uUserID(0),
m_strAddress(""),
m_serverip(""),
m_OpenLocalUser(NULL),
m_userGroupId(""),
m_mtgID (""),
m_openCamera(NO),
mVWidth(0),
mVHeight(0),
mVBitRate(0),
mVFrameRate(0),
m_headUrl(""),
m_nickName(""),
m_latitude(0),
m_longitude(0),
m_mtgType(MTG_Common),
m_pINetWorkCallback(NULL)
{
	XNetSetting::SetAudioProtocolType(XNetSetting::PT_UDP);
	XNetSetting::SetMCUOnly(1);
	m_pINetWorkCallback=NULL;

	m_UserInfoList.clear();

	//    setupSession();

	m_pAudioCap = NULL;
	m_pVoiceRpm = NULL;

	m_touchMoveView = NULL;
}

CVideoChat::~CVideoChat(void)
{
}


void CVideoChat::setAudioSession()
{
    
}

int CVideoChat::SendData(KCmdPacketEx& pPacket)
{
    return 0;
}


int CVideoChat::ConnectServer(const char* szIP,unsigned int nPort,bool isEncrypt)
{
	int iRet = m_ConnectServer.checkIsDomain(szIP);
    	if(iRet == 0)
	{	
		char addr[IP_SIZE] = {0};
		iRet = m_ConnectServer.getIpByDomain(szIP, addr);
		LOGI("%s:is domain, ip is:%s", szIP, iRet==0 ? addr : "get ip failed");
        	m_serverip = addr;
	}
	else if(iRet == 1)
	{
		LOGI("%s:is ip addr", szIP);
        	m_serverip = szIP;
	}
	else
	{
		LOGI("%s:unknow this is what, maybe it is empty", szIP);
        	m_serverip = szIP;
	}
    
	return m_ConnectServer.Start(m_serverip.c_str(),nPort,isEncrypt,this);
}

void CVideoChat::disconnect()
{
    m_ConnectServer.Stop();
}

void CVideoChat::On_SessionConnectStatus(CONNECT_STATUS cs)
{
	if(cs == CS_LOGINED||cs == CS_RECONNECTED)
	{
		LOGI("login ok");
		this->getUserList();
		if(m_pINetWorkCallback)
		m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);
	}
	else if(cs == CS_DISCONNECTED||cs == CS_RESTARTED)
	{
		LOGI("disconnet or restarted");
		KAutoLock lock(m_mKCritSec);
		closeMediaSender();
		StopAllRemoteMedia();
	}
	else if(cs == CS_LOGINFAILED)
	{
		LOGI("login failed");
	}
	else if(cs == CS_CONNECTED||cs == CS_RECONNECTED)
	{
		LOGI("will login server, status is:%d", cs);
		this->sendloginServer( m_username.c_str() ,"",  m_pAudioCap != NULL);
	}

	if(m_pINetWorkCallback)
	{
		m_pINetWorkCallback->IConnectStatusCallback(cs);
	}
}

void CVideoChat::setNetWorkCallback(IVideoChatCallback*netWorkCallback)
{
    m_pINetWorkCallback   =netWorkCallback;
}

void CVideoChat::MtgInfoReg(MTG_CMD mtgCmd,bool result,std::string &info)
{
    if (m_pINetWorkCallback) {
        m_pINetWorkCallback->MtgInfoReg(mtgCmd, result, info);
    }
}

void CVideoChat::MtgInfo(MTG_CMD mtgCmd,std::string username ,std::string &info)
{
    if (m_pINetWorkCallback) {
        m_pINetWorkCallback->MtgInfo(mtgCmd, username, info);
    }
}

void CVideoChat::OnDispatchMtgCmdReg(KCmdPacketEx& pPacket)
{
	MTG_CMD mtgCmd   = (MTG_CMD)pPacket.GetAttrib("MTGCMDREQ").AsUnsignedLong();
	bool result = pPacket.GetAttrib("RESULT").AsBoolean();
	std::string strinfo = pPacket.GetAttrib("INFO").AsString();
	switch (mtgCmd)
	{
		case  MTG_CREAT:
			{
				int iStatus = MS_CREAT_OK;
				if(result)
				{
					LOGI("MTG_CREAT succee");
					openLocalMedia(NO);
				}
				else
				{
					LOGI("MTG_CREAT fail:%d", result);
					closeMediaSender();
					iStatus = MS_CREAT_FAILED;
				}

				if(m_pINetWorkCallback)
					m_pINetWorkCallback->MtgStatuse(iStatus, m_username.c_str(), "", "", "", "", 0);
			}
			break;
        
		case  MTG_JOIN:
			{
	            		int iStatus = MS_JOIN_OK;
				if(result)
				{
					LOGI("MTG_ JOIN success");

					//�Ȼ�ȡ�����Ա�б����ϱ�Ӧ�ò�                    		
					CMD_ITEM_LST lstItems = pPacket.GetItemList();
                			CLIENTUSERINFOLIST_MAP     userInfoList;
					for(CMD_ITEM_LST::const_iterator it=lstItems.begin();it!=lstItems.end();it++)
					{
			                    KCmdItem item((std::string)*it);
			                    CLIENTUSERINFOLIST userinfo;
			                    userinfo.ulMtgTime     = item.GetAttrib("MTGTIME").AsString();
			                    userinfo.strUserName   =  item.GetAttrib("USERNAME").AsString();
			                    userinfo.ulUserId      =  item.GetAttrib("USERID").AsUnsignedLong();
			                    userinfo.ulUserAudioID =  item.GetAttrib("USERAUDIOID").AsUnsignedLong();
			                    userinfo.strHeadUrl    =  item.GetAttrib("HEADURL").AsString();
			                    userinfo.strNickName   =  item.GetAttrib("NICKNAME").AsString();
			                    userinfo.ulLatitude    =  item.GetAttrib("LATITUDE").AsDouble();
			                    userinfo.ulLongitude   =  item.GetAttrib("LONGITUDE").AsDouble();
			                    
			                    userInfoList[userinfo.ulUserId ] =userinfo;
					}
					if(m_pINetWorkCallback)
						m_pINetWorkCallback->MtgUserList(userInfoList);

					openLocalMedia(NO);
					//���ϱ��Լ���״̬
					if(m_pINetWorkCallback)
					{
						m_pINetWorkCallback->MtgStatuse(iStatus, m_username.c_str(), "", "", "", "", 0);
					}

					//���ϱ�ÿ����Ա��״̬
					{
						KAutoLock lock(m_mKCritSec);
						CLIENTUSERINFOLIST_MAP::iterator iter =userInfoList.begin();
						while (iter!=userInfoList.end())
						{
							CLIENTUSERINFOLIST userinfo=(*iter).second;
							OpenRemoteUser* m_OpenRemoteUser = new OpenRemoteUser(false);
							m_OpenRemoteUser->ConnectMediaServer(m_serverip, REMOTE_MCUPort);
							m_OpenRemoteUser->OpenAudioReceive(userinfo.ulUserAudioID );
							m_pOpenRemoteUser_map[userinfo.ulUserId] = m_OpenRemoteUser;
                            
							LOGI("MTG_ JOIN success, user name:%s, audio id:%d, time:%ld", userinfo.strUserName.c_str(), userinfo.ulUserAudioID, userinfo.ulMtgTime.c_str());
							OpenRemoteUserAudio(userinfo.strUserName.c_str());

							if(m_pINetWorkCallback)
								m_pINetWorkCallback->UserVideoView(userinfo.strUserName.c_str());

							char acJingdu[64] = {0};
							sprintf(acJingdu, "%lf", userinfo.ulLatitude);
							char acWeidu[64] = {0};
							sprintf(acWeidu, "%lf", userinfo.ulLongitude);
							if(m_pINetWorkCallback)
								m_pINetWorkCallback->MtgStatuse(iStatus, userinfo.strUserName.c_str(), userinfo.strHeadUrl.c_str(),
																userinfo.strNickName.c_str(), acJingdu, acWeidu, userinfo.ulMtgTime.c_str());

							iter++;
						}
	                    	}
				}
				else
				{
					LOGI("MTG_ JOIN fail");
	                		iStatus = MS_JOIN_FAILED;
					closeMediaSender();
					if(m_pINetWorkCallback)
						m_pINetWorkCallback->MtgStatuse(iStatus, m_username.c_str(), "", "", "", "", 0);
				}
		            
			}
			break;
        
		case  MTG_EXIT:
			{
				LOGI("MTG_exit param:%s", pPacket.GetString().c_str());
				closeMediaSender();
				StopAllRemoteMedia();

				m_mtgID ="";
			}
			break;
        
		case  MTG_DESTROY:
			{
	            		LOGI("MTG_destroy param:%s", pPacket.GetString().c_str());
	            		closeMediaSender();
				StopAllRemoteMedia();

	            		closeLocalVideo();
				m_mtgID ="";

	            		if(m_pINetWorkCallback)
					m_pINetWorkCallback->MtgStatuse(MS_DESTROYED_OK, m_username.c_str(), "", "", "", "", 0);
			}
			break;
        
		default:
			break;
	}
    
    MtgInfoReg(mtgCmd, result, strinfo);
    
}

void CVideoChat::OnDispatchMtgCmd( KCmdPacketEx& pPacket)
{
	MTG_CMD mtgCmd   = (MTG_CMD)pPacket.GetAttrib("MTGCMD").AsUnsignedLong();
	long userid = pPacket.GetAttrib("USERID").AsUnsignedLong();
	std::string username = pPacket.GetAttrib("USERNAME").AsString();
      LOGI("OnDispatch Mtg Cmd in:%d,%s,%d", mtgCmd,username.c_str(),userid);
	switch (mtgCmd)
	{
		case  MTG_JOIN:
		{
            		LOGI("recv MTG_ JOIN ed in:%s", username.c_str());
			CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.find(userid);
			if(iter != m_UserInfoList.end())
			{
				CLIENTUSERINFOLIST userinfo=(*iter).second;
                		userinfo.ulMtgTime  = pPacket.GetAttrib("MTGTIME").AsString();
				OpenRemoteUser* m_OpenRemoteUser = new OpenRemoteUser(false);
				m_OpenRemoteUser->ConnectMediaServer(m_serverip, REMOTE_MCUPort);
				m_OpenRemoteUser->OpenAudioReceive(userinfo.ulUserAudioID);
                		m_pOpenRemoteUser_map[userinfo.ulUserId] = m_OpenRemoteUser;

                		OpenRemoteUserAudio(userinfo.strUserName.c_str());
				LOGI("recv MTG_ JOIN ed, user name:%s,%s,%s,%lf,%lf,%s", userinfo.strUserName.c_str(), userinfo.strHeadUrl.c_str(),
                    													userinfo.strNickName.c_str(), userinfo.ulLatitude, userinfo.ulLongitude, 
                    													userinfo.ulMtgTime.c_str());

                		//test begin
                		//char *pTmp = "1510298175247";
                        	//unsigned long tmp = strtoul(pTmp,NULL,10);
                           //LOGI("test data src[%s], dst[%ld],[%x], long len[%d, %d]", pTmp, tmp, tmp, sizeof(unsigned long), sizeof(long));
                		//test end


				if(m_pINetWorkCallback)
					m_pINetWorkCallback->UserVideoView(userinfo.strUserName.c_str());

	                    if(m_pINetWorkCallback)
				{
					char acJingdu[64] = {0};
					sprintf(acJingdu, "%lf", userinfo.ulLatitude);
					char acWeidu[64] = {0};
					sprintf(acWeidu, "%lf", userinfo.ulLongitude);
					m_pINetWorkCallback->MtgStatuse(MS_JOIN_OK, userinfo.strUserName.c_str(), userinfo.strHeadUrl.c_str(),
												userinfo.strNickName.c_str(), acJingdu, acWeidu, userinfo.ulMtgTime.c_str());
                    		m_pINetWorkCallback->MtgJoin(userinfo);
                    
                		}

				//chenzh
				//�ϱ�java�㣬java�㴰�ڴ�����֮���ٵ���m_OpenRemoteUser->SetVideoWindow
				//m_OpenRemoteUser->SetVideoWindow(touchview);
				//m_peertouchMoveViews.add(touchview);

			}
		}
		break;

		case  MTG_EXIT:
		{
            		LOGI("user exist");
                    
			std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(userid);
			if (it!=m_pOpenRemoteUser_map.end())
			{
				LOGI("will close user meida recv");
				OpenRemoteUser *openRemoteUser=(*it).second;
				openRemoteUser->CloseAudioReceive();
				openRemoteUser->CloseVideoReceive();
				usleep(500);
                		openRemoteUser->ReleaseMediaSever();
                        	
                          stopReceiveRemoteAudio(openRemoteUser);

				delete openRemoteUser;
				LOGI("will erase remote use from map 3");
				m_pOpenRemoteUser_map.erase(it);
			}
            
        		LOGI("will close user media");
			CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.find(userid);
			if(iter != m_UserInfoList.end())
			{
				LOGI("will close user audio");
				
                		CLIENTUSERINFOLIST userinfo=(*iter).second;
                		if(m_pINetWorkCallback)
				{
					m_pINetWorkCallback->MtgStatuse(MS_EXIT_OK, userinfo.strUserName.c_str(), userinfo.strHeadUrl.c_str(), 
												userinfo.strNickName.c_str(), "", "", 0);
	                          m_pINetWorkCallback->MtgExit(userinfo);
                    	}
			}
            		
		}
		break;

		case  MTG_DESTROY:
		{
            		LOGI("conf destroyed");
			closeMediaSender();
			StopAllRemoteMedia();

            		closeLocalVideo();

            		if(m_pINetWorkCallback)
				m_pINetWorkCallback->MtgStatuse(MS_DESTROYED_OK, m_username.c_str(), "", "", "", "", 0);
		}
		break;

		default:
			break;
	}
    
}

void CVideoChat::OnDispatchCmd(KCmdPacketEx& pPacket)
{
    std::string strCMD = pPacket.GetCMD();
    LOGI("str CMD ==>%s",strCMD.c_str());
    
    if(strCMD=="LOGINSERVERED")
    {
        m_uUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        unsigned long ulErrorType = pPacket.GetAttrib("ERRORTYPE").AsUnsignedLong();
        if(ulErrorType == 0)
        {
       	 if(m_pVoiceRpm == NULL)
		{
			m_pVoiceRpm = AiyouVoiceRpm::getInstance();
			LOGI("get voice rtp rlt:%x", m_pVoiceRpm);
			m_pVoiceRpm->Init();
		}

        	LOGI("login ok, will get user list");
            this->getUserList();
            if(m_pINetWorkCallback)
                m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINED);
        }
        else
        {
            LOGI("login servered failed");
            if(m_pINetWorkCallback)
                m_pINetWorkCallback->IConnectStatusCallback(CS_LOGINFAILED);
        }
    }
    else if(strCMD=="RELOGIN")
    {
        unsigned long  userid= pPacket.GetAttrib("USERID").AsUnsignedLong();
        LOGI("your number logined from other device");
        if(m_pINetWorkCallback)
                m_pINetWorkCallback->IConnectStatusCallback(CS_RELOGIN);
    }
    else if(strCMD=="REMOTEUSERLOGIN")
    {
    	  KAutoLock lock(m_mKCritSec);
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        unsigned long ulUseraudioID = pPacket.GetAttrib("USERAUDIOID").AsUnsignedLong();
        
        CLIENTUSERINFOLIST usrinfo;
       usrinfo.strUserName   =  pPacket.GetAttrib("USERNAME").AsString();
        usrinfo.ulUserId      =  pPacket.GetAttrib("USERID").AsUnsignedLong();
        usrinfo.ulUserAudioID =  pPacket.GetAttrib("USERAUDIOID").AsUnsignedLong();
        usrinfo.strHeadUrl    =  pPacket.GetAttrib("HEADURL").AsString();
        usrinfo.strNickName   =  pPacket.GetAttrib("NICKNAME").AsString();
        usrinfo.ulLatitude    =  pPacket.GetAttrib("LATITUDE").AsDouble();
        usrinfo.ulLongitude   =  pPacket.GetAttrib("LONGITUDE").AsDouble();
        
        m_UserInfoList[ulUserID] = usrinfo;
        
        if(m_pINetWorkCallback)
        {
            m_pINetWorkCallback->INetReceiveUserLogin(ulUserID, strUserName,ulUseraudioID);
        }
    }
      else if(strCMD=="RELOGIN")
      {
          KAutoLock lock(m_mKCritSec);
         
          closeMediaSender();
        
          
          StopAllRemoteMedia();
          
          if(m_pINetWorkCallback) {
              m_pINetWorkCallback->IConnectStatusCallback(CS_RELOGIN);
          }
      }
    
    else if(strCMD=="REMOTEUSERQUIT")
    {
        
        KAutoLock lock(m_mKCritSec);
        
        unsigned long ulUserID = pPacket.GetAttrib("USERID").AsUnsignedLong();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        std::string strExpand = pPacket.GetAttrib("EXPAND").AsString();
        
        CLIENTUSERINFOLIST_MAP::iterator iter = m_UserInfoList.find(ulUserID);
        
        if(m_pINetWorkCallback)
        {
            m_pINetWorkCallback->INetReceiveUserLogOut(ulUserID, strUserName);
        }
        if(iter != m_UserInfoList.end())
        {
            m_UserInfoList.erase(iter);
        }
        
        std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.find(ulUserID);
        if (it!=m_pOpenRemoteUser_map.end())
        {
            OpenRemoteUser *openRemoteUser=(*it).second;
            if(openRemoteUser)
            {
	            openRemoteUser->CloseAudioReceive();
	            openRemoteUser->CloseVideoReceive();
                usleep(500);
	            openRemoteUser->ReleaseMediaSever();
	            delete openRemoteUser;
	            openRemoteUser = NULL;
            }
            LOGI("will erase remote use from map 1");
            m_pOpenRemoteUser_map.erase(it);
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
            usrinfo.strUserName   =  item.GetAttrib("USERNAME").AsString();
            usrinfo.ulUserId      =  item.GetAttrib("USERID").AsUnsignedLong();
            usrinfo.ulUserAudioID =  item.GetAttrib("USERAUDIOID").AsUnsignedLong();
            usrinfo.strHeadUrl    =  item.GetAttrib("HEADURL").AsString();
            usrinfo.strNickName   =  item.GetAttrib("NICKNAME").AsString();
            usrinfo.ulLatitude    =  item.GetAttrib("LATITUDE").AsDouble();
            usrinfo.ulLongitude   =  item.GetAttrib("LONGITUDE").AsDouble();
            m_UserInfoList[usrinfo.ulUserId] = usrinfo;
        }
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->INetReceiveUserList(m_UserInfoList);
    }
    else if(strCMD=="SIGNALINGTRANSFER")
    {
        KAutoLock lock(m_mKCritSec);
        bool isbroadcast = pPacket.GetAttrib("BROADCAST").AsBoolean();
        std::string strData = pPacket.GetAttrib("DATA").AsString();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        LOGI("strData =%s",strData.c_str());
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
    else if(strCMD=="USERQUIT")
    {
        KAutoLock lock(m_mKCritSec);
        closeMediaSender();
        StopAllRemoteMedia();

        if(m_pINetWorkCallback) {
            m_pINetWorkCallback->IConnectStatusCallback(CS_LOGOUT);
        }
    }
    
    else if(strCMD=="USERMEDIASTATUS")
    {
        KAutoLock lock(m_mKCritSec);
        bool isvideo = pPacket.GetAttrib("ISVIDEO").AsBoolean();
        bool isopen = pPacket.GetAttrib("ISOPEN").AsBoolean();
        std::string strUserName = pPacket.GetAttrib("USERNAME").AsString();
        
        if(m_pINetWorkCallback)
            m_pINetWorkCallback->UserMediaStatus(strUserName, isvideo, isopen);
    }
    else if(strCMD=="MTGCMDREQ")
    {
        KAutoLock lock(m_mKCritSec);
        OnDispatchMtgCmdReg(pPacket);
        
    }
    else if(strCMD=="MTGCMD")
    {
        KAutoLock lock(m_mKCritSec);
        OnDispatchMtgCmd(pPacket);
    }
    else if(strCMD=="INVITEMTGUSER")
    {
        std::string mtgid = pPacket.GetAttrib("MTGID").AsString();
        std::string username = pPacket.GetAttrib("USERNAME").AsString();
     
        if (m_pINetWorkCallback) {
            m_pINetWorkCallback->MtgInvite(username, mtgid);
        }
    }
}

int CVideoChat::sendloginServer(const char* szName, const char* szPassword, bool isConfDoing)
{
    if(!szName || !szPassword)
        return -1;
    
    m_mtgID ="";
    
    m_username = szName;
    srand((unsigned)time(NULL));
    long timeCurr=time(NULL);
    if(!isConfDoing)
    {
	this->m_audioID=timeCurr-1400000000;
    }
    
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
    rPacket.SetAttribDBL(latitude, m_latitude);
    
    std::string longitude = "LONGITUDE";
    rPacket.SetAttribDBL(longitude, m_longitude);
 
    return m_ConnectServer.SendData(rPacket);
}

int CVideoChat::logoutServer()
{   
    std::string strinfo = "quitserver";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "QUITSERVER";
    rPacket.SetCMD(strCMD);
    
    closeMediaSender();
    StopAllRemoteMedia();
    
    KAutoLock lock(m_mKCritSec);
    m_UserInfoList.clear();
    
    m_mtgID ="";
    
    return m_ConnectServer.SendData(rPacket);
}

int CVideoChat::getUserList()
{
    std::string strGetuserlist= "getuserlist";
    KCmdPacketEx rPacket(strGetuserlist.c_str(),(int)strGetuserlist.length()+1);
    rPacket.SetAttrib("ADDRESS", m_strAddress);
    std::string strCMD = "GETUSERLIST";
    rPacket.SetCMD(strCMD);
    return m_ConnectServer.SendData(rPacket);
}

void CVideoChat::LoginServer(const char* szIP,const char* szName, const char* headUrl,
                               const char* nickName,double latitude,double longitude ,const char* szGameId,
                               const char* szGameServerId,const char* szRoomId,const char* szGroupId ,
                               const char* szexpand,bool isencrypt )
{
    if(!szIP||!szName||!szGameId||!szGameServerId||!szRoomId)
    {
        return;
    }
    
    m_szexpand = szexpand;
    char sAddress[255] = {0};
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


int CVideoChat::SendUserMediaStatus(bool isvideo, bool isopen )
{
    if( m_mtgID == "")
    {
        return 0;
    }
    std::string strinfo = "UserMediaStatus";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "USERMEDIASTATUS";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttribBL("ISVIDEO", isvideo);
    rPacket.SetAttribBL("ISOPEN", isopen);
    rPacket.SetAttribUL("USERID", m_uUserID);
    rPacket.SetAttrib("USERNAME", m_username);
    return m_ConnectServer.SendData(rPacket);
}

int CVideoChat::SendMtgCmd(MTG_CMD mtgCmd, std::string mtgid)
{
    
    std::string strinfo = "MTGCMD";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "MTGCMD";
    rPacket.SetCMD(strCMD);
    
    rPacket.SetAttribUL("MTGCMD", mtgCmd);
    rPacket.SetAttrib("MTGID",mtgid);
    if (mtgCmd == MTG_CREAT) {
        rPacket.SetAttribUL("MTGTYPE", m_mtgType);
        
    }
    
    return m_ConnectServer.SendData(rPacket);
}

void CVideoChat::creatMtg(std::string mtgid, bool openCamera,MTG_TYPE mtgType)
{
   if(m_mtgID !="")
        return;
   m_openCamera = openCamera;
    m_mtgType = mtgType;

    m_mtgID = mtgid;
    openMediaSender();
    SendMtgCmd(MTG_CREAT, mtgid);
}

void CVideoChat::joinMtg(std::string mtgid, bool openCamera)
{
    if(m_mtgID !="")
    {
        
        return;
    }
    m_openCamera = openCamera;
    
    m_mtgID = mtgid;
    openMediaSender();
    SendMtgCmd(MTG_JOIN, mtgid);
}

void CVideoChat::exitMtg()
{
    closeMediaSender();
    StopAllRemoteMedia();
    
    SendMtgCmd(MTG_EXIT, m_mtgID);
    m_mtgID ="";
}

void CVideoChat::destroyMtg()
{
    closeMediaSender();
    StopAllRemoteMedia();
    
    SendMtgCmd(MTG_EXIT, m_mtgID);
 m_mtgID ="";
 
}

int CVideoChat::SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen)
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

int CVideoChat::SendAllUserData(const char* pData, unsigned long nLen)
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

int CVideoChat::mtgInviteUser(const char* mtgId, const char* iuserId)
{
    if(!mtgId || !iuserId||m_mtgID ==""  )
        return -1;
    
    std::string strinfo = "INVITEMTGUSER";
    KCmdPacketEx rPacket(strinfo.c_str(),(int)strinfo.length()+1);
    std::string strCMD = "INVITEMTGUSER";
    rPacket.SetCMD(strCMD);
    rPacket.SetAttrib("MTGID", m_mtgID);
    rPacket.SetAttrib("USERNAME", m_username);

	KAutoLock lock(m_mKCritSec);
    CLIENTUSERINFOLIST_MAP::iterator iter =m_UserInfoList.begin();
    while (iter!=m_UserInfoList.end())
    {
        CLIENTUSERINFOLIST userinfo=(*iter).second;
        
        if(userinfo.strUserName  == iuserId)
        {
          rPacket.SetAttribUL("PEERUSERID", userinfo.ulUserId);
           return m_ConnectServer.SendData(rPacket);
          
        }
        iter++;
    }
    return 0;
    
}

std::string CVideoChat::GetLocalIP()
{
    return m_ConnectServer.GetLocalIP();
}

std::string CVideoChat::GetNATIP()
{
    return m_ConnectServer.GetNATIP();
}

void CVideoChat::SendAudioData(char*pData, int nLen)
{
    if (m_OpenLocalUser )
    {
        m_OpenLocalUser->SendAudioData(pData, nLen);
    }
}

void CVideoChat::OnAudioOpusNotifyOutput(unsigned char*pPcm,int nLen)
{
	//LOGI("CStream Media in, on auido opus notify, len:%d", nLen);
	SendAudioData((char*)pPcm, nLen);
}

void CVideoChat::setAudioCapParam(int iChannels, int iSampleRate, int iBitRate)
{
	LOGI("set audio cap param,channel,samp,bit:%d,%d,%d", iChannels, iSampleRate, iBitRate);
	mChannels	= iChannels;
	mSampleRate = iSampleRate;
	mBitRate	= iBitRate;
}

void CVideoChat::SetVideoCapParam(int width, int height, int bitrate, int frameRate)
{
	LOGI("set video cap param:%d,%d,%d,%d", width, height, bitrate, frameRate);
	mVWidth   = width;
	mVHeight = height;
	mVBitRate    = bitrate;
	mVFrameRate =  frameRate;
}

void CVideoChat::resetLocalVideo()
{
    if (m_OpenLocalUser) {
        m_OpenLocalUser->resetLocalVideo();
    }
}

void CVideoChat::openMediaSender()
{
	KAutoLock lock(m_mKCritSec);
	if (!m_OpenLocalUser)
	{
		m_OpenLocalUser = new OpenLocalUser;
		m_OpenLocalUser->ConnectMediaServer(m_serverip, LOCAL_MCUPort);
		m_OpenLocalUser->OpenAudioSend((unsigned int)this->m_audioID);
		m_OpenLocalUser->OpenVideoSend((unsigned int)this->m_audioID+200);
	}
}

void CVideoChat::openLocalMedia(bool isCapScreen)
{
	openLocalAudio();
	if (m_openCamera)
	{
		//chenzh
		//�ϱ�java�㣬java�㴰�ڴ�����֮���ٵ���m_OpenRemoteUser->SetVideoWindow
		
		LOGI("openLocal Media, user name:%s", m_username.c_str());
		if(m_pINetWorkCallback)
			m_pINetWorkCallback->UserVideoView(m_username.c_str());
	}
}

void CVideoChat::closeMediaSender()
{
	KAutoLock lock(m_mKCritSec);
	if (m_OpenLocalUser  )
	{
		closeLocalAudio();
		m_OpenLocalUser->closeLocalVideo();

		m_OpenLocalUser->CloseAudioSend();
		m_OpenLocalUser->CloseVideoSend();
		m_OpenLocalUser->ReleaseMediaSever();

		m_touchMoveView = NULL;

		delete m_OpenLocalUser;
		m_OpenLocalUser = NULL;
	}
}

void CVideoChat::closeLocalAudio( )
{
	SendUserMediaStatus(false, false);

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

void CVideoChat::openLocalAudio()
{
	SendUserMediaStatus(false, true);

	LOGI("open local Audio in, voice rpm:%x", m_pVoiceRpm);
	if(m_pAudioCap == NULL)
	{
		m_pAudioCap = new CAudioCapChanle(true, mSampleRate, this);
		int iSampleBits = 16;// ��Ƶ�������
		int iInterval = 20;	//20ms����Ƶ��������
		int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
		m_pAudioCap->Connect(mChannels, mSampleRate, iBytes, mBitRate);
		m_pAudioCap->Start();

	        if(m_pVoiceRpm)
		{	
			LOGI("will call webrtc cap audio 1");
			m_pVoiceRpm->StartCapture(m_pAudioCap);
		}
	}
}

void CVideoChat::openLocalVideo(void *pWnd)
{
	m_touchMoveView = pWnd;
	if (m_OpenLocalUser) {
		m_OpenLocalUser->openLocalVideo(m_touchMoveView);
		SendUserMediaStatus(true, true);
	}
}

void CVideoChat::closeLocalVideo( )
{
	m_touchMoveView = NULL;
	if (m_OpenLocalUser) {
		m_OpenLocalUser->closeLocalVideo();
		SendUserMediaStatus(true, false);
	}
}

void CVideoChat::StopAllRemoteMedia()
{
	KAutoLock lock(m_mKCritSec);
	std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.begin();
	while (it!=m_pOpenRemoteUser_map.end())
	{
		OpenRemoteUser *openRemoteUser=(*it).second;
        	stopReceiveRemoteAudio(openRemoteUser);
		if(openRemoteUser)
		{
			openRemoteUser->CloseAudioReceive();
			openRemoteUser->CloseVideoReceive();
			openRemoteUser->ReleaseMediaSever();
			delete openRemoteUser;
		}
		LOGI("will erase remote use from map 2");
		m_pOpenRemoteUser_map.erase(it++);
	}

}

void CVideoChat::OpenRemoteUserVideo(const char* username , void* pVideoWnd)
{
	std::string strusername = username;
    
	KAutoLock lock(m_mKCritSec);
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
				openRemoteUser->SetVideoWindow(pVideoWnd);
				openRemoteUser->OpenVideoReceive( userinfo.ulUserAudioID+200);
			}
			break;
		}
		iter++;
	}
}

void CVideoChat::closeRemoteUserVideo(const char* username)
{
	std::string strusername = username;

	KAutoLock lock(m_mKCritSec);
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
				openRemoteUser->CloseVideoReceive();
			}

			break;
		}
		iter++;
	}
}

void CVideoChat::OpenRemoteUserAudio(const char* username)
{
    std::string strusername = username;

    KAutoLock lock(m_mKCritSec);
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
                openRemoteUser->OpenAudioReceive(userinfo.ulUserAudioID);
                startReceiveRemoteAudio(openRemoteUser);
            }
            break;
        }
        iter++;
    }

}

int CVideoChat::startReceiveRemoteAudio(OpenRemoteUser* remoteUser)
{
	//��Ƶ���벥����
	LOGI("start recv media, player is not null, release it first");
		
	CAudioPlayChanle *pAudioPlay = new CAudioPlayChanle();
	int iSampleBits = 16;// ��Ƶ�������
	int iInterval = 20; //20ms����Ƶ��������
	int iBytes = mChannels * mSampleRate * iSampleBits * iInterval/ (8*1000);
	pAudioPlay->Connect(mChannels, mSampleRate, iBytes, mBitRate);
	pAudioPlay->Start();
	
	int iChanID = -1;
	if(m_pVoiceRpm)
	{
		iChanID = m_pVoiceRpm->StartPlayout(mSampleRate);
		pAudioPlay->setAudioPcmPlayer(m_pVoiceRpm, iChanID);
	}
	
	LOGI("ConnectMedia Server end, chanid:%d", iChanID);

	if(remoteUser)
	{
		remoteUser->setAudioPlayer(pAudioPlay);
		remoteUser->setAudioChanID(iChanID);
	}
	
	return 0;
}

void CVideoChat::stopReceiveRemoteAudio(OpenRemoteUser* remoteUser)
{
	LOGI("stop recv media in");
	//ֹͣ��Ƶ����ģ��
	int chanId = -1;
	if(remoteUser)
	{
		chanId = remoteUser->getAudioChanID();
		CAudioPlayChanle *pAudioPlay = remoteUser->getAudioPlayer();
		remoteUser->setAudioPlayer(NULL);
		if(pAudioPlay)
		{
			pAudioPlay->Stop();
			pAudioPlay->ReleaseConnections();
			delete pAudioPlay;
			pAudioPlay = NULL;
		}
	}

	if(m_pVoiceRpm)
	{
		m_pVoiceRpm->StopPlayout(chanId);
	}
    
	LOGI("stop recv media end");
}

int CVideoChat::remoteAudioSetPause(const char* username, bool bPause)
{
	std::string strusername = username;
    	KAutoLock lock(m_mKCritSec);
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
				openRemoteUser->setAudioMute(bPause);
			}
			break;
		}
		iter++;
	}
}


int CVideoChat::localAudioSetPause(bool bPause)
{
	if(m_pAudioCap)
       {
       	m_pAudioCap->setPause(bPause);
       }
	if(bPause)
	{
		SendUserMediaStatus(false, false);
	}
	else
	{
		SendUserMediaStatus(false, true);
	}
}

int CVideoChat::localCameraSwitch()
{
	if(m_OpenLocalUser)
       {
       	m_OpenLocalUser->switchLocalVideo();
       }
}

int CVideoChat::localNetworkSwitched()
{
	LOGI("local network switched begin");
	//self send
	if(m_OpenLocalUser)
       {
       	m_OpenLocalUser->reopenMediaSender();
       }

	LOGI("will reset peer info");
    	//remote recv
    	KAutoLock lock(m_mKCritSec);
	std::map<unsigned long, class OpenRemoteUser*>::iterator it=m_pOpenRemoteUser_map.begin();
	while (it!=m_pOpenRemoteUser_map.end())
	{
		OpenRemoteUser *openRemoteUser=(*it).second;
        	openRemoteUser->reopenMediaRecver();
            	it++;
	}
	LOGI("local network switched end");
}


