#include "com_aiyou_ptt_AudioOpusJni.h"
#include "AudioCapture.h"
#include "AudioRender.h"
#include "log.h"
#include "CVoiceManage.h"
#include "util_audio_jni_helper.h"
#include "util_audio_register_native_api.h"
#include "CLinkMicManage.h"
#include "CVideoChat.h"
#include "callbackDef.h"
#include "util_audio_register_native_api.h"

#ifndef LOG_TAG
#define LOG_TAG "multVoice"
#endif

#ifndef NULL
#define NULL 0
#endif

using namespace std;

extern "C" int InitJavaObject(JNIEnv* env);
extern "C" int UnInitJavaObject(JNIEnv* pEnv);

JavaVM*               g_utilJVM = NULL;
static jobject        m_javaObj = NULL;
static jmethodID   m_fStatusChanged = NULL;
static jmethodID	m_fNetworkChanged = NULL;
static jmethodID	m_fTalkInviteIn = NULL;
static jmethodID	m_fTalkConnected = NULL;
static jmethodID	m_fTalkDisConn = NULL;
static jclass g_clsAudioOpusJni = NULL;
static jmethodID	m_fPttUserListIn = NULL;
static jmethodID	m_fPttUserStatusChanged = NULL;
static jmethodID	m_fVolumeChanged = NULL;



//????????
static jmethodID	m_fLinkMicNetChanged = NULL;
static jmethodID	m_fLinkMicUserLogin = NULL;
static jmethodID	m_fLinkMicUserLogout = NULL;

//????????
static jmethodID	m_fConfNetChanged = NULL;
static jmethodID	m_fConfUserLogin = NULL;
static jmethodID	m_fConfUserLogout = NULL;
static jmethodID	m_fConfVideoWndNeed = NULL;
static jmethodID	m_fConfUserMediaStatus = NULL;
static jmethodID	m_fConfInviteComein = NULL;
static jmethodID	m_fConfBeKicked = NULL;
static jmethodID	m_fConfStatus = NULL;
static jmethodID	m_fConfUserListIn = NULL;


#ifndef JNI_USER_MAP
typedef std::map<std::string, CLIENTUSERINFOLIST> JNI_USER_MAP;
#endif

JNI_USER_MAP     gMapMembs;


int ExpectionProc(JNIEnv* env)
{
	if(!env)
	{
		return -1;
	}

	jthrowable exc = env->ExceptionOccurred();
	if (exc)
	{
		env->ExceptionDescribe();
		env->ExceptionClear();
		return 0;
	}
	else
	{
		return -2;
	}
}

extern "C" int InitJavaObject(JNIEnv* env)
{
	LOGI("Init Java Object in" );

	int iRet = env->GetJavaVM(&g_utilJVM);
	if (iRet==0)
	{
		LOGI("get javavm success");
	}
	else
	{
		LOGE("get javavm failed");
		g_utilJVM = NULL;
		return  -1;
	}

	jclass j_capture_class = env->FindClass("com/aiyou/ptt/AudioOpusJni");
	if(!j_capture_class)
	{
		LOGE("get audio opus jni obj failed");
		g_utilJVM = NULL;
		return  -1;
	}
	g_clsAudioOpusJni = reinterpret_cast<jclass>(env->NewGlobalRef(j_capture_class));

	//talk
	m_fStatusChanged        = env->GetMethodID(g_clsAudioOpusJni, "onStatusChanged", "(IZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	m_fNetworkChanged     = env->GetMethodID(g_clsAudioOpusJni, "onNetworkChanged", "(I)V");
	m_fTalkInviteIn             = env->GetMethodID(g_clsAudioOpusJni, "onTalkInviteIn", "(IZLjava/lang/String;)V");
	m_fTalkConnected         = env->GetMethodID(g_clsAudioOpusJni, "onTalkConnected", "()V");
	m_fTalkDisConn		   = env->GetMethodID(g_clsAudioOpusJni, "onTalkDisConnected", "(ILjava/lang/String;)V");
	m_fPttUserListIn		= env->GetMethodID(g_clsAudioOpusJni, "onPttUserList", "()V");
    	m_fVolumeChanged	= env->GetMethodID(g_clsAudioOpusJni, "onVolumeChanged", "(I)V");
       m_fPttUserStatusChanged= env->GetMethodID(g_clsAudioOpusJni, "onPttUserStatusChanged", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");

	//link mic
	m_fLinkMicNetChanged	= env->GetMethodID(g_clsAudioOpusJni, "onLinkMicNetworkChanged", "(I)V");
	m_fLinkMicUserLogin		= env->GetMethodID(g_clsAudioOpusJni, "onLinkMicUserLogin", "(ILjava/lang/String;I)V");
	m_fLinkMicUserLogout		= env->GetMethodID(g_clsAudioOpusJni, "onLinkMicUserLogout", "(ILjava/lang/String;)V");

	//conf
	m_fConfNetChanged		= env->GetMethodID(g_clsAudioOpusJni, "onConfNetworkChanged", "(I)V");
	m_fConfUserLogin		= env->GetMethodID(g_clsAudioOpusJni, "onConfUserLogin", "(ILjava/lang/String;I)V");
	m_fConfUserLogout		= env->GetMethodID(g_clsAudioOpusJni, "onConfUserLogout", "(ILjava/lang/String;)V");
	m_fConfVideoWndNeed	= env->GetMethodID(g_clsAudioOpusJni, "onConfVideoWndNeed", "(Ljava/lang/String;)V");
	m_fConfUserMediaStatus	= env->GetMethodID(g_clsAudioOpusJni, "onConfUserMediaStatus", "(Ljava/lang/String;ZZ)V");
	m_fConfInviteComein		= env->GetMethodID(g_clsAudioOpusJni, "onConfInviteIn", "(Ljava/lang/String;Ljava/lang/String;)V");
	m_fConfStatus 			= env->GetMethodID(g_clsAudioOpusJni, "onConfStatus", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    	m_fConfBeKicked			= env->GetMethodID(g_clsAudioOpusJni, "onConfBeKicked", "(Ljava/lang/String;Ljava/lang/String;)V");
       m_fConfUserListIn		= env->GetMethodID(g_clsAudioOpusJni, "onConfUserList", "()V");

	jmethodID construction_id = env->GetMethodID(g_clsAudioOpusJni, "<init>", "()V");
	if (construction_id == 0)
	{
		LOGE("Get MethodID return  null");
	}

	jobject  objTmp = env->NewObject(g_clsAudioOpusJni, construction_id);
	if (objTmp == NULL)
	{
		LOGE("New Object return  null");
	}

	m_javaObj = env->NewGlobalRef(objTmp);
	if (m_javaObj == NULL)
	{
		LOGE("New GlobalRef return  null");
	}

	/*
	jclass classIdMy = env->FindClass("com/uutodo/sdk/WatchLiveUtil");
	if ( classIdMy != NULL)
	{
	LOGI("find class success");
	}
	else
	{
	LOGE("find class failed 1");
	}
	*/

	LOGI("Init Java Object end" );
	return 0;
}


extern "C" int UnInitJavaObject(JNIEnv* pEnv)
{
    LOGI("will delete java object begin");

    if (m_javaObj != NULL)
    {
        pEnv->DeleteGlobalRef(m_javaObj);
        m_javaObj= NULL;
    }

    if (g_clsAudioOpusJni)
    {
        pEnv->DeleteGlobalRef(g_clsAudioOpusJni);
        g_clsAudioOpusJni = NULL;
    }

    LOGI("will delete java object end" );
}

void CNetWork::IConnectStatusCallback(CONNECT_STATUS cs)
{
	int iStatus = cs;
	LOGI("network connect status cb, %d,%d", cs, iStatus);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	LOGI("mult voice conn status, %p, %p,%p", m_javaObj, m_fNetworkChanged, env);

	env->CallVoidMethod(m_javaObj, m_fNetworkChanged, iStatus);

	return;
}

void CNetWork::INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
	LOGI("ptt up user list in");
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	int cnt = 0;
    	gMapMembs.clear();
	CLIENTUSERINFOLIST_MAP::iterator iter =UserInfoList.begin();
	while (iter!=UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
        	gMapMembs[userinfo.strUserName] = userinfo;

		iter++;
	}

	env->CallVoidMethod(m_javaObj, m_fPttUserListIn);
}

void CNetWork::INetReceiveUserLogin(CLIENTUSERINFOLIST *pUserinfo)
{
	LOGI("ptt user login up begin");
	JNIEnv* pEnv = NULL;
	CAutoGetEnvAudio auto_getenv(&pEnv);
	if(pUserinfo == NULL)
	{
		LOGE("input obj is null");
		return;
	}

	jstring strid = pEnv->NewStringUTF(pUserinfo->strUserName.c_str());
	jstring strHead = pEnv->NewStringUTF(pUserinfo->strHeadUrl.c_str());
	jstring strName = pEnv->NewStringUTF(pUserinfo->strNickName.c_str());

	char acBuf[128] = {0};
	char acBuf2[128] = {0};
	memset(acBuf, 0, sizeof(acBuf));
	sprintf(acBuf, "%lf", pUserinfo->ulLatitude);
	jstring strJingdu = pEnv->NewStringUTF(acBuf);
	memset(acBuf2, 0, sizeof(acBuf2));
	sprintf(acBuf2, "%lf", pUserinfo->ulLongitude);
	jstring strWeidu = pEnv->NewStringUTF(acBuf2);
      jstring strTm = pEnv->NewStringUTF(pUserinfo->ulMtgTime.c_str());

	int type = 1;
	LOGI("tm and type:%d",type);

	pEnv->CallVoidMethod(m_javaObj, m_fPttUserStatusChanged, strid, strHead, strName, strJingdu, strWeidu, strTm, (jint)type);
       pEnv->DeleteLocalRef(strid);
       pEnv->DeleteLocalRef(strHead);
       pEnv->DeleteLocalRef(strName);
       pEnv->DeleteLocalRef(strJingdu);
       pEnv->DeleteLocalRef(strWeidu);
       pEnv->DeleteLocalRef(strTm);

	LOGI("ptt user login up end");
}

void CNetWork::INetReceiveUserLogOut(CLIENTUSERINFOLIST *pUserinfo)
{
	LOGI("ptt user logout up begin");
	JNIEnv* pEnv = NULL;
	CAutoGetEnvAudio auto_getenv(&pEnv);

	if(pUserinfo == NULL)
	{
		LOGE("input obj is null");
		return;
	}

	jstring strid = pEnv->NewStringUTF(pUserinfo->strUserName.c_str());
	jstring strHead = pEnv->NewStringUTF(pUserinfo->strHeadUrl.c_str());
	jstring strName = pEnv->NewStringUTF(pUserinfo->strNickName.c_str());

	char acBuf[128] = {0};
	char acBuf2[128] = {0};
	memset(acBuf, 0, sizeof(acBuf));
	sprintf(acBuf, "%lf", pUserinfo->ulLatitude);
	jstring strJingdu = pEnv->NewStringUTF(acBuf);
	memset(acBuf2, 0, sizeof(acBuf2));
	sprintf(acBuf2, "%lf", pUserinfo->ulLongitude);
	jstring strWeidu = pEnv->NewStringUTF(acBuf2);
	jstring strTm = pEnv->NewStringUTF(pUserinfo->ulMtgTime.c_str());

	int type = 0;
	LOGI("tm and type:%d", type);

	pEnv->CallVoidMethod(m_javaObj, m_fPttUserStatusChanged, strid, strHead, strName, strJingdu, strWeidu, strTm, (jint)type);

       pEnv->DeleteLocalRef(strid);
       pEnv->DeleteLocalRef(strHead);
       pEnv->DeleteLocalRef(strName);
       pEnv->DeleteLocalRef(strJingdu);
       pEnv->DeleteLocalRef(strWeidu);
       pEnv->DeleteLocalRef(strTm);

	LOGI("ptt user logout up end");
}

void CNetWork::INetReceiveData(unsigned long uPeerUserID, const char* pData, unsigned long nLen){}
void CNetWork::INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen){}
void CNetWork::INetAudioStatus(unsigned long uPeerUserID, bool isRoom, AUDIO_SEND_STATUS AudioStatus, std::string speaker){}
void CNetWork::INetVideoCall(unsigned long uPeerUserID,bool videocall, std::string strName)
{
	LOGI("aiyou VideoCall in, param:%ld,%d,%s", uPeerUserID, videocall, strName.c_str());
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	LOGI("11");
	jstring jName = env->NewStringUTF(strName.c_str());
	LOGI("12");
	env->CallVoidMethod(m_javaObj, m_fTalkInviteIn, (int)uPeerUserID, videocall, jName);
	LOGI("13");
	env->DeleteLocalRef(jName);
	LOGI("14");
}

void CNetWork::INetCallConnected()
{
	LOGI("aiyou call connected in");
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	env->CallVoidMethod(m_javaObj, m_fTalkConnected);
}

void CNetWork::INetCallDisConn(int iReason, const char*pInfo)
{
	LOGI("aiyou call dis connected in");
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	jstring jInfo = env->NewStringUTF(pInfo);

	env->CallVoidMethod(m_javaObj, m_fTalkDisConn, iReason, jInfo);

	env->DeleteLocalRef(jInfo);
}

void CNetWork::INetVolumeChanged(int volume)
{
	LOGI("ptt volume changed in:", volume);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	env->CallVoidMethod(m_javaObj, m_fVolumeChanged, (jint)volume);
}


static CNetWork *m_pNetWork = NULL;


extern "C" int aiyouVoiceCallback(int msgcode, bool issucceed, const char *funtype,
				const char *status, const char *username, const char* isImRoom)
{
	LOGI("aiyou VoiceCallback in, param:%d,%d,%s,%s,%s,%s", msgcode, issucceed, funtype, status, username, isImRoom);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	LOGI("%p, %p,%p", m_javaObj, m_fStatusChanged, env);

	jstring jFuncType = env->NewStringUTF(funtype);
	jstring jStatus = env->NewStringUTF(status);
	jstring jUser = env->NewStringUTF(username);
	jstring jInRoom = env->NewStringUTF(isImRoom);
	if(!jFuncType || !jStatus || !jUser)
	{
		ExpectionProc(env);
		LOGE("call New StringUTF return NULL");
		return -1;
	}

	env->CallVoidMethod(m_javaObj, m_fStatusChanged, msgcode, issucceed, jFuncType, jStatus, jUser, jInRoom);

	env->DeleteLocalRef(jFuncType);
	env->DeleteLocalRef(jStatus);
	env->DeleteLocalRef(jUser);
	env->DeleteLocalRef(jInRoom);

	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_audioModuleInit
  (JNIEnv * pEnv, jobject pThis, jobject pContext)
{
    LOGD("audio module init in");
    if(!g_utilJVM)
    {
        InitJavaObject(pEnv);
    }
    CVoiceManage::SetAndroidObjects(g_utilJVM,  pEnv, pContext);
    LoadAndroidJavaVM(g_utilJVM);
   LOGD("audio module init end");
    return 0;
}


static CVoiceManage *m_pVoiceMgr = NULL;

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_pttLogin
  (JNIEnv * pEnv, jobject pThis, jstring jip, jstring jUserName, jstring jPssword,
  jstring jGameID, jstring jGameServer, jstring jRoomID, jstring jGroupID,
  jint iChanal, jint sampleRate, jint iBitRate, jboolean jbIsListenInRoom, jstring jExpend, jboolean isEncrypt,
  jstring jHeadUrl, jstring jNickName, jdouble jingdu, jdouble weidu)
{
//	LOGI("jni login begin");
	if(jip == NULL)
	{
		LOGE("start login, but input server ip is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pIP = NULL;
	if(jip)
	{
		pIP = (char*)pEnv->GetStringUTFChars(jip,NULL);
	}

	char *pUserName = NULL;
	if(jUserName)
	{
		pUserName = (char*)pEnv->GetStringUTFChars(jUserName,NULL);
	}

	char *pPwd = NULL;
	if(jPssword)
	{
		pPwd = (char*)pEnv->GetStringUTFChars(jPssword,NULL);
	}

//
	char *pGameId = NULL;
	if(jGameID)
	{
		pGameId = (char*)pEnv->GetStringUTFChars(jGameID,NULL);
	}

	char *pGameServer = NULL;
	if(jGameServer)
	{
		pGameServer = (char*)pEnv->GetStringUTFChars(jGameServer,NULL);
	}

	char *pRoomID = NULL;
	if(jRoomID)
	{
		pRoomID = (char*)pEnv->GetStringUTFChars(jRoomID,NULL);
	}

	char *pGroupID = NULL;
	if(jGroupID)
	{
		pGroupID = (char*)pEnv->GetStringUTFChars(jGroupID,NULL);
	}

	char *pExpend = NULL;
	if(jExpend)
	{
		pExpend = (char*)pEnv->GetStringUTFChars(jExpend,NULL);
	}

	char *pHeadUrl = NULL;
	if(jHeadUrl)
	{
		pHeadUrl = (char*)pEnv->GetStringUTFChars(jHeadUrl,NULL);
	}

    	char *pNickName = NULL;
	if(jNickName)
	{
		pNickName = (char*)pEnv->GetStringUTFChars(jNickName,NULL);
	}

	if(m_pVoiceMgr)
	{
		LOGE("pVoiceMgr is not empty, reset it");
		m_pVoiceMgr->logoutServer();
		usleep(500000);
		m_pVoiceMgr->disconnect();
		delete m_pVoiceMgr;
		m_pVoiceMgr = NULL;
	}

	m_pVoiceMgr = new CVoiceManage(aiyouVoiceCallback);
	m_pNetWork = new CNetWork();
	m_pVoiceMgr->setNetWorkCallback(m_pNetWork);
	m_pVoiceMgr->setAudioCapParam(iChanal, sampleRate, iBitRate);
	m_pVoiceMgr->loginVoiceServer(pIP, pUserName, pGameId, pGameServer,
        							pRoomID, pGroupID, jbIsListenInRoom,
        							pExpend, isEncrypt,
        							pHeadUrl, pNickName, jingdu, weidu);

	if(pIP)pEnv->ReleaseStringUTFChars(jip, pIP);
	if(pUserName)pEnv->ReleaseStringUTFChars(jUserName, pUserName);
	if(pPwd)pEnv->ReleaseStringUTFChars(jPssword, pPwd);
	if(pGameId)pEnv->ReleaseStringUTFChars(jGameID, pGameId);
	if(pGameServer)pEnv->ReleaseStringUTFChars(jGameServer, pGameServer);
	if(pRoomID)pEnv->ReleaseStringUTFChars(jRoomID, pRoomID);
	if(pGroupID)pEnv->ReleaseStringUTFChars(jGroupID, pGroupID);
	if(pExpend)pEnv->ReleaseStringUTFChars(jExpend, pExpend);
	if(pHeadUrl)pEnv->ReleaseStringUTFChars(jHeadUrl, pHeadUrl);
	if(pNickName)pEnv->ReleaseStringUTFChars(jNickName, pNickName);
	LOGI("jni login end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_pttLogout
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni logout begin");

	if(m_pVoiceMgr)
	{
		m_pVoiceMgr->logoutServer();
		usleep(500000);
		m_pVoiceMgr->disconnect();
		m_pVoiceMgr->setNetWorkCallback(NULL);

		delete m_pVoiceMgr;
		m_pVoiceMgr = NULL;
	}

	if(m_pNetWork)
	{
		delete m_pNetWork;
		m_pNetWork = NULL;
	}

	LOGI("jni logout end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_setVolumeEnable
      (JNIEnv * pEnv, jobject pThis, jboolean enable)
{
	LOGI("jni set volume enable begin");

	if(!m_pVoiceMgr)
	{
		LOGE("start send, please login first");
		return -1;
	}

	int iRet = 0;
	m_pVoiceMgr->enableUpVolume(enable);

	LOGI("jni set volume enable end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_startSendAudio
  (JNIEnv * pEnv, jobject pThis, jboolean jbIsInRoom)
{
	//????????
	LOGI("jni start send audio in, room flag:%d", jbIsInRoom);
	if(!m_pVoiceMgr)
	{
		LOGE("start send, please login first");
		return -1;
	}

	int iRet = 0;
	m_pVoiceMgr->startSendAudio(jbIsInRoom);

	LOGI("start Send audio end, ret:%d", iRet);
	return iRet;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_stopSendAudio
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni stop send begin");
	if(!m_pVoiceMgr)
	{
		LOGE("stop send, please login first");
		return -1;
	}

	m_pVoiceMgr->stopSendAudio();

	LOGI("jni stop send end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_startRecvAudio
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni start recv begin");
	if(!m_pVoiceMgr)
	{
		LOGE("start recv, please login first");
		return -1;
	}

	//m_pVoiceMgr->StartReceiveMedia();

	LOGI("jni start recv end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_stopRecvAudio
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni stop recv in");
	if(!m_pVoiceMgr)
	{
		LOGE("stop recv, please login first");
		return -1;
	}

	LOGI("jni stop recv end");
	return 0;
}

JNIEXPORT jobject JNICALL Java_com_aiyou_ptt_AudioOpusJni_pttGetUpUserList
    (JNIEnv * pEnv, jobject pThis)
{
	LOGI("ptt get up user list begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

   	//JNIEnv* env = NULL;
	//CAutoGetEnvAudio auto_getenv(&env);

	//????list
	jclass list_cls = pEnv->FindClass("java/util/ArrayList");//????ArrayList??????
	if(list_cls == NULL)
	{
		LOGE("get array list class failed");
	   	return NULL;
	}

	jmethodID list_costruct = pEnv->GetMethodID(list_cls , "<init>","()V"); //??????????????Id
	if(list_costruct == NULL)
	{
		LOGE("get array init failed");
	   	return NULL;
	}
	jobject list_obj = pEnv->NewObject(list_cls , list_costruct); //????????Arraylist????????
	if(list_obj == NULL)
	{
		LOGE("new array init failed");
	   	return NULL;
	}
	//????Arraylist?????? add()????ID???????????????? boolean add(Object object) ;
	jmethodID list_add  = pEnv->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
    	if(list_add == NULL)
	{
		LOGE("get array add func failed");
	   	return NULL;
	}

	jclass stu_cls = pEnv->FindClass("com/aiyou/ptt/ConfUserInfo");//????Student??????
	if(stu_cls == NULL)
	{
		LOGE("get obj class failed");
	   	return NULL;
	}

	//????????????????????  ???????? <init> ?????????????? void ?? V
	jmethodID stu_costruct = pEnv->GetMethodID(stu_cls , "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    	if(stu_costruct == NULL)
	{
		LOGE("get obj init func failed");
	   	return NULL;
	}

	int cnt = 0;
	char acBuf[128] = {0};
	JNI_USER_MAP::iterator iter =gMapMembs.begin();
	while (iter!=gMapMembs.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
		jstring strid = pEnv->NewStringUTF(userinfo.strUserName.c_str());
		jstring strHead = pEnv->NewStringUTF(userinfo.strHeadUrl.c_str());
		jstring strName = pEnv->NewStringUTF(userinfo.strNickName.c_str());

		memset(acBuf, 0, sizeof(acBuf));
		sprintf(acBuf, "%lf", userinfo.ulLatitude);
		jstring strJingdu = pEnv->NewStringUTF(acBuf);
		memset(acBuf, 0, sizeof(acBuf));
		sprintf(acBuf, "%lf", userinfo.ulLongitude);
		jstring strWeidu = pEnv->NewStringUTF(acBuf);

		jstring strTime = pEnv->NewStringUTF(userinfo.ulMtgTime.c_str());

		//??????????????????????????new ???? Student????
		jobject stu_obj = pEnv->NewObject(stu_cls , stu_costruct , strid, strHead, strName, strJingdu, strWeidu, strTime);  //????????????

		pEnv->CallBooleanMethod(list_obj , list_add , stu_obj); //????Arraylist????????add??????????????stu????

		iter++;
		cnt++;
	}

	LOGI("ptt get up user list end");
	return list_obj;
}




jobject jObjLocalWnd = NULL;
jobject jObjPeerWnd = NULL;


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_talkCallout
  (JNIEnv * pEnv, jobject pThis, jboolean ibIsVideo, jstring jPeerNumb, jobject jLocalVieo, jobject jPeerVideo)
{
	LOGI("jni talk callout in");
	if(!m_pVoiceMgr)
	{
		LOGE("start callout, please login first");
		return -1;
	}

	char *pPeerNum = NULL;
	if(jPeerNumb)
	{
		pPeerNum = (char*)pEnv->GetStringUTFChars(jPeerNumb,NULL);
	}

	if (jObjLocalWnd != NULL)
	{
		pEnv->DeleteGlobalRef(jObjLocalWnd);
	}
	if (jObjPeerWnd != NULL)
	{
		pEnv->DeleteGlobalRef(jObjPeerWnd);
	}

	jObjLocalWnd = pEnv->NewGlobalRef( (jobject)jLocalVieo );
	jObjPeerWnd = pEnv->NewGlobalRef( (jobject)jPeerVideo );

	m_pVoiceMgr->invite(ibIsVideo, pPeerNum, jObjLocalWnd, jObjPeerWnd);

	if(pPeerNum)pEnv->ReleaseStringUTFChars(jPeerNumb, pPeerNum);

	LOGI("jni talk callout end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_talkAccept
  (JNIEnv * pEnv, jobject pThis, jobject jLocalVieo, jobject jPeerVideo)
{
	LOGI("jni talk accept in");
	if(!m_pVoiceMgr)
	{
		LOGE("talk accept, please login first");
		return -1;
	}

	if (jObjLocalWnd != NULL)
	{
		pEnv->DeleteGlobalRef(jObjLocalWnd);
	}
	if (jObjPeerWnd != NULL)
	{
		pEnv->DeleteGlobalRef(jObjPeerWnd);
	}

	jObjLocalWnd = pEnv->NewGlobalRef( (jobject)jLocalVieo );
	jObjPeerWnd = pEnv->NewGlobalRef( (jobject)jPeerVideo );

	m_pVoiceMgr->accept(jObjLocalWnd, jObjPeerWnd);


	LOGI("jni talk accept end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_talkCancel
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni talk cancel in");
	if(!m_pVoiceMgr)
	{
		LOGE("talk cancel, please login first");
		return -1;
	}

	m_pVoiceMgr->cancel();


	LOGI("jni talk cancel end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_talkRefuse
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni talk refuse in");
	if(!m_pVoiceMgr)
	{
		LOGE("talk refuse, please login first");
		return -1;
	}

	m_pVoiceMgr->refuse();

	LOGI("jni talk refuse end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_talkHangup
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni talk hangup in");
	if(!m_pVoiceMgr)
	{
		LOGE("talk hangup, please login first");
		return -1;
	}

	m_pVoiceMgr->hangup();


	LOGI("jni talk hangup end");
	return 0;
}

/************????????*******************/
void CLinkMicCallback::IConnectStatusCallback(LINKMIC_STATUS cs)
{
	int iStatus = cs;
	LOGI("link mic network connect status cb, %d,%d", cs, iStatus);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	LOGI("link mic, %p, %p,%p", m_javaObj, m_fLinkMicNetChanged, env);

	env->CallVoidMethod(m_javaObj, m_fLinkMicNetChanged, iStatus);
	return;
}

void CLinkMicCallback::INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
}

void CLinkMicCallback::INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID)
{
	LOGI("link mic user login in,%ld, %s, %d", uPeerUserID, strName.c_str() != NULL ? strName.c_str() : "NULL", seraudioID);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	//if(strName)
	jstring username = env->NewStringUTF(strName.c_str() != NULL ? strName.c_str() : "");

	LOGI("login up 1");

	env->CallVoidMethod(m_javaObj, m_fLinkMicUserLogin, (int)uPeerUserID,  username, (int)seraudioID);

	LOGI("login up 2");

	env->DeleteLocalRef(username);
	LOGI("login up 3");
}

void CLinkMicCallback::INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName)
{
	LOGI("link mic user logout in,%ld, name:%s", uPeerUserID, strName.c_str());
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	jstring username = env->NewStringUTF(strName.c_str() != NULL ? strName.c_str() : "");
	env->CallVoidMethod(m_javaObj, m_fLinkMicUserLogout, (int)uPeerUserID, username);
	env->DeleteLocalRef(username);
}

void CLinkMicCallback::INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen)
{
}

void CLinkMicCallback::INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen)
{
}


static CLinkMicCallback *m_pLinkMicCb = NULL;
static CLinkMicManage  *m_pLinkMicMgr = NULL;
static map<string,  jobject> m_pMapVideoWnd;
static jobject m_pLocalVideoWnd = NULL;

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicLogin
  (JNIEnv * pEnv, jobject pThis, jstring jip, jstring jUserName, jstring jPssword,
  jstring jGameID, jstring jGameServer, jstring jRoomID, jstring jGroupID,
  jint iChanal, jint sampleRate, jint iBitRate, jboolean isEncrypt, jboolean isAnchor, jstring jExpend)
{
	LOGI("link mic login begin");
	if(jip == NULL)
	{
		LOGE("start login, but input server ip is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pIP = NULL;
	if(jip)
	{
		pIP = (char*)pEnv->GetStringUTFChars(jip,NULL);
	}

	char *pUserName = NULL;
	if(jUserName)
	{
		pUserName = (char*)pEnv->GetStringUTFChars(jUserName,NULL);
	}

	char *pPwd = NULL;
	if(jPssword)
	{
		pPwd = (char*)pEnv->GetStringUTFChars(jPssword,NULL);
	}

	char *pGameId = NULL;
	if(jGameID)
	{
		pGameId = (char*)pEnv->GetStringUTFChars(jGameID,NULL);
	}

	char *pGameServer = NULL;
	if(jGameServer)
	{
		pGameServer = (char*)pEnv->GetStringUTFChars(jGameServer,NULL);
	}

	char *pRoomID = NULL;
	if(jRoomID)
	{
		pRoomID = (char*)pEnv->GetStringUTFChars(jRoomID,NULL);
	}

	char *pGroupID = NULL;
	if(jGroupID)
	{
		pGroupID = (char*)pEnv->GetStringUTFChars(jGroupID,NULL);
	}

	char *pExpend = NULL;
	if(jExpend)
	{
		pExpend = (char*)pEnv->GetStringUTFChars(jExpend,NULL);
	}
//

	if(m_pLinkMicMgr)
	{
		LOGE("LinkMicMgr is not empty, reset it");
		m_pLinkMicMgr->logoutServer();
		usleep(500000);
		m_pLinkMicMgr->disconnect();
		delete m_pLinkMicMgr;
		m_pLinkMicMgr = NULL;
	}

	m_pLinkMicCb = new CLinkMicCallback();
	m_pLinkMicMgr = new CLinkMicManage();
	m_pLinkMicMgr->setNetWorkCallback(m_pLinkMicCb);
	m_pLinkMicMgr->setAudioCapParam(iChanal, sampleRate, iBitRate);
	m_pLinkMicMgr->loginLinkMicServer(
									pIP, pUserName, pGameId,
									pGameServer, pRoomID, pGroupID,
									pExpend, isEncrypt, false, isAnchor);

	if(pIP)pEnv->ReleaseStringUTFChars(jip, pIP);
	if(pUserName)pEnv->ReleaseStringUTFChars(jUserName, pUserName);
	if(pPwd)pEnv->ReleaseStringUTFChars(jPssword, pPwd);
	if(pGameId)pEnv->ReleaseStringUTFChars(jGameID, pGameId);
	if(pGameServer)pEnv->ReleaseStringUTFChars(jGameServer, pGameServer);
	if(pRoomID)pEnv->ReleaseStringUTFChars(jRoomID, pRoomID);
	if(pGroupID)pEnv->ReleaseStringUTFChars(jGroupID, pGroupID);
	if(pExpend)pEnv->ReleaseStringUTFChars(jExpend, pExpend);

	map<string, jobject>::iterator it=m_pMapVideoWnd.begin();
	while (it!=m_pMapVideoWnd.end())
	{
		jobject objtmp = (*it).second;
		if (objtmp != NULL)
		{
			pEnv->DeleteGlobalRef(objtmp);
		}
		m_pMapVideoWnd.erase(it++);
	}

	LOGI("link mic login end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicLogout
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("link mic logout begin");

	if(m_pLinkMicMgr)
	{
		m_pLinkMicMgr->logoutServer();
		usleep(500000);
		m_pLinkMicMgr->disconnect();
		m_pLinkMicMgr->setNetWorkCallback(NULL);

		delete m_pLinkMicMgr;
		m_pLinkMicMgr = NULL;
	}

	if(m_pLinkMicCb)
	{
		delete m_pLinkMicCb;
		m_pLinkMicCb = NULL;
	}

	LOGI("link mic logout end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicOpenRemoteVideo
  (JNIEnv * pEnv, jobject pThis, jstring jPeerNumb, jobject jPeerVideo)
{
	LOGI("jni open remote video start");
	if(!m_pLinkMicMgr || !jPeerNumb)
	{
		LOGE("start remotvideo, but link mic mgr or input peer num is null");
		return -1;
	}

	char *pPeerNum = NULL;
	if(jPeerNumb)
	{
		pPeerNum = (char*)pEnv->GetStringUTFChars(jPeerNumb,NULL);
	}

/*
	jobject jObjTmp = pEnv->NewGlobalRef( (jobject)jPeerVideo );
	m_pMapVideoWnd[string(pPeerNum)] = jObjTmp;*/

	m_pLinkMicMgr->OpenRemoteUserVideo(pPeerNum, jPeerVideo);

	if(pPeerNum)pEnv->ReleaseStringUTFChars(jPeerNumb, pPeerNum);

	LOGI("jni open remote video end");
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicResetRemoteVideo
  (JNIEnv * pEnv, jobject pThis, jstring jPeerNumb, jobject jPeerVideo)
{
	LOGI("jni reset remote video start");
	if(!m_pLinkMicMgr || !jPeerNumb)
	{
		LOGE("reset remotvideo, but link mic mgr or input peer num is null");
		return -1;
	}

	char *pPeerNum = NULL;
	if(jPeerNumb)
	{
		pPeerNum = (char*)pEnv->GetStringUTFChars(jPeerNumb,NULL);
	}

	m_pLinkMicMgr->resetRemoteUserVideo(pPeerNum, jPeerVideo);

	if(pPeerNum)pEnv->ReleaseStringUTFChars(jPeerNumb, pPeerNum);

	LOGI("jni reset remote video end");
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicCloseRemoteVideo
  (JNIEnv * pEnv, jobject pThis, jstring jPeerNumb)
{
	LOGI("jni close remote video start");
	if(!m_pLinkMicMgr || !jPeerNumb)
	{
		LOGE("stop remotvideo, but link mic mgr or input peer num is null");
		return -1;
	}

	char *pPeerNum = NULL;
	if(jPeerNumb)
	{
		pPeerNum = (char*)pEnv->GetStringUTFChars(jPeerNumb,NULL);
	}

	LOGI("jni close remote video 00");

	m_pLinkMicMgr->closeRemoteUserVideo(pPeerNum);
	LOGI("jni close remote video 01");

/*
	map<string,  jobject>::iterator it=m_pMapVideoWnd.find(pPeerNum);
	if (it!=m_pMapVideoWnd.end())
	{
		jobject jObjTmp = (*it).second;
		if (jObjTmp != NULL)
		{
			pEnv->DeleteGlobalRef(jObjTmp);
		}
		LOGI("jni close remote video 2");
		m_pMapVideoWnd.erase(it);
		LOGI("jni close remote video 3");
	}*/

	if(pPeerNum)pEnv->ReleaseStringUTFChars(jPeerNumb, pPeerNum);

	LOGI("jni close remote video end");
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicOpenLocalVideo
  (JNIEnv * pEnv, jobject pThis, jobject jPeerVideo)
{
	LOGI("jni open local video start");
	if(!m_pLinkMicMgr)
	{
		LOGE("start local video, but link mic mgr is null");
		return -1;
	}

	if (m_pLocalVideoWnd != NULL)
	{
		pEnv->DeleteGlobalRef(m_pLocalVideoWnd);
	}

	m_pLocalVideoWnd= pEnv->NewGlobalRef((jobject)jPeerVideo );
	m_pLinkMicMgr->openLocalVideo(m_pLocalVideoWnd);

	LOGI("jni open local video end");
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_LinkMicCloseLocalVideo
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("jni close local video start");
	if(!m_pLinkMicMgr)
	{
		LOGE("close local video, but link mic mgr is null");
		return -1;
	}

	m_pLinkMicMgr->closeLocalVideo();

	if (m_pLocalVideoWnd != NULL)
	{
		pEnv->DeleteGlobalRef(m_pLocalVideoWnd);
		m_pLocalVideoWnd = NULL;
	}

	LOGI("jni close remote video end");
}


/****************????????*******************/
void CConfCallback::MtgStatuse(int status, const char *usename, const char *headurl,
							const char *nickname, const char *jingdu, const char *weidu, const char * mtgTm)
{
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	LOGI("conf up conf status in:%d,%s, %s, %s, %s, %s, %s", status, usename, headurl, nickname, jingdu, weidu, mtgTm);
	jstring jusername = env->NewStringUTF(usename);
	jstring jhead = env->NewStringUTF(headurl);
	jstring jnick = env->NewStringUTF(nickname);
	jstring jjingdu = env->NewStringUTF(jingdu);
	jstring jweidu = env->NewStringUTF(weidu);
    	jstring jTime = env->NewStringUTF(mtgTm);
	env->CallVoidMethod(m_javaObj, m_fConfStatus, status, jusername, jhead, jnick, jjingdu, jweidu, jTime);
	env->DeleteLocalRef(jusername);
	env->DeleteLocalRef(jhead);
	env->DeleteLocalRef(jnick);
	env->DeleteLocalRef(jjingdu);
	env->DeleteLocalRef(jweidu);
	env->DeleteLocalRef(jTime);
	LOGI("conf up conf status out");
}

void CConfCallback::UserVideoView(const char *userid)
{
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	LOGI("conf up video view in");
	jstring jid = env->NewStringUTF(userid);
	env->CallVoidMethod(m_javaObj, m_fConfVideoWndNeed, jid);
	env->DeleteLocalRef(jid);
	LOGI("conf up video view out");
}

void CConfCallback::UserMediaStatus(std::string strName,bool isvideo,bool isopen)
{
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	LOGI("conf up user media status in");
	jstring jusername = env->NewStringUTF(strName.c_str());
	env->CallVoidMethod(m_javaObj, m_fConfUserMediaStatus, jusername, isvideo, isopen);
	env->DeleteLocalRef(jusername);
	LOGI("conf up user media status out");
}

void CConfCallback::MtgInfoReg(MTG_CMD mtgCmd,bool result,std::string &info)
{
}

void CConfCallback::MtgInfo(MTG_CMD mtgCmd,std::string username ,std::string &info)
{
}

void CConfCallback::MtgInvite(std::string username ,std::string mtgId)
{
	LOGI("conf up conf invite in:%s,%s", username.c_str(), mtgId.c_str());
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	jstring jusername = env->NewStringUTF(username.c_str());
	jstring jconfid = env->NewStringUTF(mtgId.c_str());
	env->CallVoidMethod(m_javaObj, m_fConfInviteComein, jusername, jconfid);
    	env->DeleteLocalRef(jusername);
	env->DeleteLocalRef(jconfid);
    	LOGI("conf up conf invite out");
}

void CConfCallback::MtgBeKicked(std::string username ,std::string mtgId)
{
	LOGI("conf up be kicked in:%s,%s", username.c_str(), mtgId.c_str());
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	jstring jusername = env->NewStringUTF(username.c_str());
	jstring jconfid = env->NewStringUTF(mtgId.c_str());
	env->CallVoidMethod(m_javaObj, m_fConfBeKicked, jusername, jconfid);
    	env->DeleteLocalRef(jusername);
	env->DeleteLocalRef(jconfid);
    	LOGI("conf up be kicked out");
}

void  CConfCallback::MtgJoin(CLIENTUSERINFOLIST &userinfo)
{
    LOGI("conf up join in");

}

void  CConfCallback::MtgExit(CLIENTUSERINFOLIST &userinfo)
{
    LOGI("conf up exit in");
}

void  CConfCallback::MtgUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
	LOGI("conf up user list in");
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	int cnt = 0;
    	gMapMembs.clear();
	CLIENTUSERINFOLIST_MAP::iterator iter =UserInfoList.begin();
	while (iter!=UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
        	gMapMembs[userinfo.strUserName] = userinfo;

		iter++;
	}

	env->CallVoidMethod(m_javaObj, m_fConfUserListIn);
}


void CConfCallback::IConnectStatusCallback(LINKMIC_STATUS cs)
{
	int iStatus = cs;
	LOGI("conf network connect status cb, %d,%d", cs, iStatus);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	env->CallVoidMethod(m_javaObj, m_fConfNetChanged, iStatus);
}

void CConfCallback::INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
	LOGI("conf receive user list in");
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	int cnt = 0;
    	gMapMembs.clear();
	CLIENTUSERINFOLIST_MAP::iterator iter =UserInfoList.begin();
	while (iter!=UserInfoList.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
        	gMapMembs[userinfo.strUserName] = userinfo;

		iter++;
        	cnt++;
	}

	env->CallVoidMethod(m_javaObj, m_fConfUserListIn);
}

void CConfCallback::INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long seraudioID)
{
	LOGI("conf up user login in, %d,%s,%d", uPeerUserID, strName.c_str(), seraudioID);
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);
	jstring username = env->NewStringUTF(strName.c_str() != NULL ? strName.c_str() : "");
	env->CallVoidMethod(m_javaObj, m_fConfUserLogin, (int)uPeerUserID,  username, (int)seraudioID);

	env->DeleteLocalRef(username);
	LOGI("conf up user login out");
}

void CConfCallback::INetReceiveUserLogOut(unsigned long uPeerUserID, std::string strName)
{
	LOGI("conf user logout in,%ld, name:%s", uPeerUserID, strName.c_str());
	JNIEnv* env = NULL;
	CAutoGetEnvAudio auto_getenv(&env);

	jstring username = env->NewStringUTF(strName.c_str() != NULL ? strName.c_str() : "");
	env->CallVoidMethod(m_javaObj, m_fConfUserLogout, (int)uPeerUserID, username);
	env->DeleteLocalRef(username);
	LOGI("conf user logout out");
}

void CConfCallback::INetReceiveData(unsigned long uPeerUserID, std::string strName,std::string strData, unsigned long nLen)
{
}

void CConfCallback::INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen)
{
}


static CConfCallback *m_pConfCb = NULL;
static CVideoChat  *m_pConfMgr = NULL;


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLogin
  (JNIEnv * pEnv, jobject pThis, jstring jip, jstring jUserName, jstring jPssword,
  jstring jGameID, jstring jGameServer, jstring jRoomID, jstring jGroupID,
  jint iChanal, jint sampleRate, jint iBitRate, jboolean isEncrypt, jstring jExpend,
  jstring jHeadUrl, jstring jNickName, jdouble jingdu, jdouble weidu)
{
	LOGI("conf login begin");
	if(jip == NULL)
	{
		LOGE("conf login, but input server ip is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pIP = NULL;
	if(jip)
	{
		pIP = (char*)pEnv->GetStringUTFChars(jip,NULL);
	}

	char *pUserName = NULL;
	if(jUserName)
	{
		pUserName = (char*)pEnv->GetStringUTFChars(jUserName,NULL);
	}

	char *pPwd = NULL;
	if(jPssword)
	{
		pPwd = (char*)pEnv->GetStringUTFChars(jPssword,NULL);
	}

	char *pGameId = NULL;
	if(jGameID)
	{
		pGameId = (char*)pEnv->GetStringUTFChars(jGameID,NULL);
	}

	char *pGameServer = NULL;
	if(jGameServer)
	{
		pGameServer = (char*)pEnv->GetStringUTFChars(jGameServer,NULL);
	}

	char *pRoomID = NULL;
	if(jRoomID)
	{
		pRoomID = (char*)pEnv->GetStringUTFChars(jRoomID,NULL);
	}

	char *pGroupID = NULL;
	if(jGroupID)
	{
		pGroupID = (char*)pEnv->GetStringUTFChars(jGroupID,NULL);
	}

	char *pExpend = NULL;
	if(jExpend)
	{
		pExpend = (char*)pEnv->GetStringUTFChars(jExpend,NULL);
	}

    	char *pHeadUrl = NULL;
	if(jHeadUrl)
	{
		pHeadUrl = (char*)pEnv->GetStringUTFChars(jHeadUrl,NULL);
	}

    	char *pNickName = NULL;
	if(jNickName)
	{
		pNickName = (char*)pEnv->GetStringUTFChars(jNickName,NULL);
	}

	if(m_pConfMgr)
	{
		LOGE("conf Mgr is not empty, reset it");
		m_pConfMgr->logoutServer();
		usleep(100000);
		m_pConfMgr->disconnect();
		delete m_pConfMgr;
		m_pConfMgr = NULL;
	}

	m_pConfCb = new CConfCallback();
	m_pConfMgr = new CVideoChat();
	m_pConfMgr->setNetWorkCallback(m_pConfCb);
	m_pConfMgr->setAudioCapParam(iChanal, sampleRate, iBitRate);
	m_pConfMgr->LoginServer(pIP, pUserName, pHeadUrl, pNickName, jingdu, weidu,
							pGameId, pGameServer, pRoomID, pGroupID,
							pExpend, isEncrypt);

	if(pIP)pEnv->ReleaseStringUTFChars(jip, pIP);
	if(pUserName)pEnv->ReleaseStringUTFChars(jUserName, pUserName);
	if(pPwd)pEnv->ReleaseStringUTFChars(jPssword, pPwd);
	if(pGameId)pEnv->ReleaseStringUTFChars(jGameID, pGameId);
	if(pGameServer)pEnv->ReleaseStringUTFChars(jGameServer, pGameServer);
	if(pRoomID)pEnv->ReleaseStringUTFChars(jRoomID, pRoomID);
	if(pGroupID)pEnv->ReleaseStringUTFChars(jGroupID, pGroupID);
	if(pExpend)pEnv->ReleaseStringUTFChars(jExpend, pExpend);
	if(pHeadUrl)pEnv->ReleaseStringUTFChars(jHeadUrl, pHeadUrl);
	if(pNickName)pEnv->ReleaseStringUTFChars(jNickName, pNickName);

	LOGI("conf login end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLogout
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf logout begin");

	if(m_pConfMgr)
	{
		m_pConfMgr->logoutServer();
		usleep(100000);
		m_pConfMgr->disconnect();
		m_pConfMgr->setNetWorkCallback(NULL);

		delete m_pConfMgr;
		m_pConfMgr = NULL;
	}

	if(m_pConfCb)
	{
		delete m_pConfCb;
		m_pConfCb = NULL;
	}

	LOGI("conf logout end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfCreate
  (JNIEnv * pEnv, jobject pThis, jstring jconfid, jboolean isOpenCamera, jint createType)
{
	LOGI("conf create begin");
	if(jconfid == NULL)
	{
		LOGE("conf create, but input confid is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pConfID = NULL;
	if(jconfid)
	{
		pConfID = (char*)pEnv->GetStringUTFChars(jconfid,NULL);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->creatMtg(pConfID, isOpenCamera, (MTG_TYPE)createType);

	if(pConfID)pEnv->ReleaseStringUTFChars(jconfid, pConfID);

	LOGI("conf create end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfJoin
  (JNIEnv * pEnv, jobject pThis, jstring jconfid, jboolean isOpenCamera)
{
	LOGI("conf join begin");
	if(jconfid == NULL)
	{
		LOGE("conf join, but input confid is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pConfID = NULL;
	if(jconfid)
	{
		pConfID = (char*)pEnv->GetStringUTFChars(jconfid,NULL);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->joinMtg(pConfID, isOpenCamera);

	if(pConfID)pEnv->ReleaseStringUTFChars(jconfid, pConfID);

	LOGI("conf join end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfExit
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf exit begin");

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty, cannot exit");
		return -1;
	}

    	m_pConfMgr->exitMtg();
	LOGI("conf logout end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfDestroy
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf destroy begin");

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty, cannot destroy");
		return -1;
	}

    	m_pConfMgr->destroyMtg();
	LOGI("conf destroy end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfInviteMember
  (JNIEnv * pEnv, jobject pThis, jstring jconfid, jstring juserid)
{
	LOGI("conf invite member begin");
	if(jconfid == NULL || juserid == NULL)
	{
		LOGE("conf join, but input confid is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pConfID = NULL;
	if(jconfid)
	{
		pConfID = (char*)pEnv->GetStringUTFChars(jconfid,NULL);
	}

	char *pUserID = NULL;
	if(juserid)
	{
		pUserID = (char*)pEnv->GetStringUTFChars(juserid,NULL);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->mtgInviteUser(pConfID, pUserID);

	if(pConfID)pEnv->ReleaseStringUTFChars(jconfid, pConfID);
	if(pUserID)pEnv->ReleaseStringUTFChars(juserid, pUserID);

	LOGI("conf invite member end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfKickMember
  (JNIEnv * pEnv, jobject pThis, jstring jconfid, jstring juserid)
{
	LOGI("conf kick member begin");
	if(jconfid == NULL || juserid == NULL)
	{
		LOGE("conf kick, but input param is null");
		return -1;
	}

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	char *pConfID = NULL;
	if(jconfid)
	{
		pConfID = (char*)pEnv->GetStringUTFChars(jconfid,NULL);
	}

	char *pUserID = NULL;
	if(juserid)
	{
		pUserID = (char*)pEnv->GetStringUTFChars(juserid,NULL);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	//m_pConfMgr->mtgKickUser(pConfID, pUserID);

	if(pConfID)pEnv->ReleaseStringUTFChars(jconfid, pConfID);
	if(pUserID)pEnv->ReleaseStringUTFChars(juserid, pUserID);

	LOGI("conf kick member end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalVideoSetParam
  (JNIEnv * pEnv, jobject pThis, jint jw, jint jh, jint jbitrate, jint jfamerate)
{
	LOGI("conf set local camera param begin");
	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->SetVideoCapParam(jw, jh, jbitrate, jfamerate);

	LOGI("conf set local camera param end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalVideoOpen
  (JNIEnv * pEnv, jobject pThis, jobject jPeerVideo)
{
	LOGI("conf open local video begin");
	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->openLocalVideo(jPeerVideo);

	LOGI("conf open local video end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalVideoClose
  (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf close local video begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->closeLocalVideo();

	LOGI("conf close local video end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfPeerVideoOpen
  (JNIEnv * pEnv, jobject pThis, jobject jPeerVideo, jstring juserid)
{
	LOGI("conf open peer video begin");
	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

      char *pUserID = NULL;
	if(juserid)
	{
		pUserID = (char*)pEnv->GetStringUTFChars(juserid,NULL);
	}

	m_pConfMgr->OpenRemoteUserVideo(pUserID, jPeerVideo);
	if(pUserID)pEnv->ReleaseStringUTFChars(juserid, pUserID);

	LOGI("conf open peer video end");
	return 0;
}


JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfPeerVideoClose
  (JNIEnv * pEnv, jobject pThis, jstring juserid)
{
	LOGI("conf close peer video begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	char *pUserID = NULL;
	if(juserid)
	{
		pUserID = (char*)pEnv->GetStringUTFChars(juserid,NULL);
	}

	m_pConfMgr->closeRemoteUserVideo(pUserID);
	if(pUserID)pEnv->ReleaseStringUTFChars(juserid, pUserID);

	LOGI("conf close peer video end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalMicMute
	(JNIEnv * pEnv, jobject pThis, jboolean bIsMute)
{
	LOGI("conf mute loca mic begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->localAudioSetPause(bIsMute);

	LOGI("conf mute loca mic end");
	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfPeerMicMute
      (JNIEnv * pEnv, jobject pThis, jstring juserid, jboolean bIsMute)
{
	LOGI("conf mute peer mic begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	char *pUserID = NULL;
	if(juserid)
	{
		pUserID = (char*)pEnv->GetStringUTFChars(juserid,NULL);
	}

	m_pConfMgr->remoteAudioSetPause(pUserID, bIsMute);
	if(pUserID)pEnv->ReleaseStringUTFChars(juserid, pUserID);

	LOGI("conf mute peer mic end");
 	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalVideoSwitch
      (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf switch local camera begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->localCameraSwitch();

	LOGI("conf switch local camera end");
 	return 0;
}

JNIEXPORT jint JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfLocalNetworkChanged
      (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf network switched begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

	if(!m_pConfMgr)
	{
		LOGE("conf Mgr is empty");
		return -1;
	}

	m_pConfMgr->localNetworkSwitched();

	LOGI("conf network switched end");
 	return 0;
}

JNIEXPORT jobject JNICALL Java_com_aiyou_ptt_AudioOpusJni_ConfGetUpUserList
    (JNIEnv * pEnv, jobject pThis)
{
	LOGI("conf get up user list begin");

	if(!g_utilJVM)
	{
		InitJavaObject(pEnv);
	}

   	//JNIEnv* env = NULL;
	//CAutoGetEnvAudio auto_getenv(&env);

	//????list
	jclass list_cls = pEnv->FindClass("java/util/ArrayList");//????ArrayList??????
	if(list_cls == NULL)
	{
		LOGE("get array list class failed");
	   	return NULL;
	}

	jmethodID list_costruct = pEnv->GetMethodID(list_cls , "<init>","()V"); //??????????????Id
	if(list_costruct == NULL)
	{
		LOGE("get array init failed");
	   	return NULL;
	}
	jobject list_obj = pEnv->NewObject(list_cls , list_costruct); //????????Arraylist????????
	if(list_obj == NULL)
	{
		LOGE("new array init failed");
	   	return NULL;
	}
	//????Arraylist?????? add()????ID???????????????? boolean add(Object object) ;
	jmethodID list_add  = pEnv->GetMethodID(list_cls,"add","(Ljava/lang/Object;)Z");
    	if(list_add == NULL)
	{
		LOGE("get array add func failed");
	   	return NULL;
	}

	jclass stu_cls = pEnv->FindClass("com/aiyou/ptt/ConfUserInfo");//????Student??????
	if(stu_cls == NULL)
	{
		LOGE("get obj class failed");
	   	return NULL;
	}

	//????????????????????  ???????? <init> ?????????????? void ?? V
	jmethodID stu_costruct = pEnv->GetMethodID(stu_cls , "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    	if(stu_costruct == NULL)
	{
		LOGE("get obj init func failed");
	   	return NULL;
	}

	int cnt = 0;
	char acBuf[128] = {0};
	JNI_USER_MAP::iterator iter =gMapMembs.begin();
	while (iter!=gMapMembs.end())
	{
		CLIENTUSERINFOLIST userinfo=(*iter).second;
		jstring strid = pEnv->NewStringUTF(userinfo.strUserName.c_str());
		jstring strHead = pEnv->NewStringUTF(userinfo.strHeadUrl.c_str());
		jstring strName = pEnv->NewStringUTF(userinfo.strNickName.c_str());

		memset(acBuf, 0, sizeof(acBuf));
		sprintf(acBuf, "%lf", userinfo.ulLatitude);
		jstring strJingdu = pEnv->NewStringUTF(acBuf);
		memset(acBuf, 0, sizeof(acBuf));
		sprintf(acBuf, "%lf", userinfo.ulLongitude);
		jstring strWeidu = pEnv->NewStringUTF(acBuf);
		jstring strTime = pEnv->NewStringUTF(userinfo.ulMtgTime.c_str());

		//??????????????????????????new ???? Student????
		jobject stu_obj = pEnv->NewObject(stu_cls , stu_costruct , strid, strHead, strName, strJingdu, strWeidu, strTime);  //????????????

		pEnv->CallBooleanMethod(list_obj , list_add , stu_obj); //????Arraylist????????add??????????????stu????

		iter++;
		cnt++;
	}
   
	LOGI("conf get up user list end");
	return list_obj;
}



