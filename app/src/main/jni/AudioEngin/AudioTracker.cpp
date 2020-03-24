#include "AudioTracker.h"
#include "Log.h"
#include "util_audio_jni_helper.h"
#include "nativehelper/JNIHelp.h"


static jclass g_AudioTrackClass = NULL;
extern "C" int LoadAudioTrackerAndroidVM(JNIEnv* env)
{
	LOGI("Load AudioTracker AndroidVM in");
	jclass jTrackClass = env->FindClass("com/aiyou/ptt/AudioTracker");
	g_AudioTrackClass = reinterpret_cast<jclass>(env->NewGlobalRef(jTrackClass));
	LOGI("Load AudioTracker AndroidVM out");
	return 0;
}

extern "C" int UnLoadAudioTrackerAndroidVM(JNIEnv* env)
{
	LOGI("unLoad AudioTracker AndroidVM in");
	if (g_AudioTrackClass)
	{
		env->DeleteGlobalRef(g_AudioTrackClass);
		g_AudioTrackClass = NULL;
	}
	return 0;
}

CAudioTracker::CAudioTracker()
: m_pJavaClassObj(NULL)
,mJavaPlayBuffer(NULL)
,mpJavaDirectPlayBuffer(NULL)
,mJavaMethPlay(NULL)
{
	m_nSampleRate = 16000;	/**< 采样率.*/
	m_nSampleBits = 16; /**< 采样大小.*/
	m_nSamplesPerFrame = 4096; 
	m_nChannels = 1;	/**< 声道.*/
}

CAudioTracker::~CAudioTracker()
{
	LOGI("AudioCaptrueAndroid dctor");
	JNIEnv *pEnv = NULL;
	CAutoGetEnvAudio auto_getenv(&pEnv);
	if(!pEnv)
	{
		pEnv = JniUtilHelperAudio::getEnv();
	}
	if (m_pJavaClassObj != NULL) {
		pEnv->DeleteGlobalRef(m_pJavaClassObj);
		m_pJavaClassObj= NULL;
	}
}

int  CAudioTracker::Start()
{
	if (m_pJavaClassObj != NULL )
	{
		LOGE("already started? m_pJavaClassObj:%p",m_pJavaClassObj);
		return -1;
	}

	JNIEnv *pEnv = NULL;
	CAutoGetEnvAudio auto_getenv(&pEnv);
	if(!pEnv)
	{
		pEnv = JniUtilHelperAudio::getEnv();
	}
	LOGE("java obj is:%p,%p", g_AudioTrackClass, pEnv);
	jmethodID ctor = pEnv->GetMethodID(g_AudioTrackClass, "<init>", "()V");

	jlong j_this = reinterpret_cast<intptr_t>(this);

	m_pJavaClassObj = pEnv->NewGlobalRef(pEnv->NewObject(g_AudioTrackClass, ctor, j_this));
	if (m_pJavaClassObj == NULL )
	{
		LOGE("CAudioCaptrueAndroid m_pJavaClassObj get return null:%p",m_pJavaClassObj);
		return -1;
	}

     	// Get play buffer field ID.
	jfieldID fidPlayBuffer = pEnv->GetFieldID(g_AudioTrackClass, "mPlayBuffer", "Ljava/nio/ByteBuffer;");
	if (!fidPlayBuffer)
	{
		LOGE("could not get play buffer fid");
		return -1;
	}

	// Get play buffer object.
	jobject javaPlayBufferLocal = pEnv->GetObjectField(m_pJavaClassObj, fidPlayBuffer);
	if (!javaPlayBufferLocal)
	{
		LOGE("could not get play buffer");
		return -1;
	}

	// Create a global reference to the object (to tell JNI that we are
	// referencing it after this function has returned)
	// NOTE: we are referencing it only through the direct buffer (see below).
	mJavaPlayBuffer = pEnv->NewGlobalRef(javaPlayBufferLocal);
	if (!mJavaPlayBuffer)
	{
		LOGE("could not get play buffer reference");
		return -1;
	}

	// Delete local object ref, we only use the global ref.
	pEnv->DeleteLocalRef(javaPlayBufferLocal);

	// Get direct buffer.
	mpJavaDirectPlayBuffer = pEnv->GetDirectBufferAddress(mJavaPlayBuffer);
	if (!mpJavaDirectPlayBuffer)
	{
		LOGE("could not get direct play buffer");
		return -1;
	}

	// Get the play audio method ID.
	mJavaMethPlay = pEnv->GetMethodID(g_AudioTrackClass, "playAudio", "(I)I");
	if (!mJavaMethPlay)
	{
		LOGE("could not get play audio mid");
		return -1;
	}
      
	jmethodID j_init = pEnv->GetMethodID(g_AudioTrackClass, "initPlayback", "(I)I");
	int iRet = pEnv->CallIntMethod(m_pJavaClassObj, j_init, m_nSampleRate);
	if (iRet != 0)	  
	{
		LOGE("Call init play Failed.");
		return -1;
	}

	jmethodID j_start = pEnv->GetMethodID(g_AudioTrackClass, "startPlayback", "()I");
	iRet = pEnv->CallIntMethod(m_pJavaClassObj, j_start);
	if (iRet != 0)	  
	{
		LOGE("Call start Playback Failed.");
		return -1;
	}
	else
	{
		LOGE("Call start Playback ok.");
		return 0;
	}
}

void  CAudioTracker::Stop()
{
	LOGE("stop play in.");
	JNIEnv *pEnv = NULL;
	CAutoGetEnvAudio auto_getenv(&pEnv);
	if(!pEnv)
	{
		pEnv = JniUtilHelperAudio::getEnv();
	}
	jmethodID j_stop = pEnv->GetMethodID(g_AudioTrackClass, "stopPlayback","()I");
	pEnv->CallIntMethod(m_pJavaClassObj, j_stop);
	LOGE("stop play in 1.");
    
	mpJavaDirectPlayBuffer = NULL;
	pEnv->DeleteGlobalRef(mJavaPlayBuffer);
	mJavaPlayBuffer = 0;
	LOGE("stop play out.");

	return;
}

int  CAudioTracker::Connect(int nChannels, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam)
{
	LOGI("[IN] Connect parma:%d,%d,%d", nChannels, nSampleRate, nSamplesPerFrame);
	
	m_nSampleRate = nSampleRate;	/**< 采样率.*/
	m_nSamplesPerFrame = nSamplesPerFrame; 
	m_nChannels = nChannels;	/**< 声道.*/

	return 0;
}

void CAudioTracker::ReleaseConnections()
{
	
}

int CAudioTracker::PushPacket(unsigned char*pData,int nLen)
{
	if(mpJavaDirectPlayBuffer)
	{
		JNIEnv *pEnv = NULL;
		CAutoGetEnvAudio auto_getenv(&pEnv);
		if(!pEnv)
		{
			pEnv = JniUtilHelperAudio::getEnv();
		}
	       
		memcpy(mpJavaDirectPlayBuffer, pData, nLen);

 		jint res = pEnv->CallIntMethod(m_pJavaClassObj, mJavaMethPlay, nLen);
		if (res < 0)
		{
			LOGI("PlayAudio failed (%d)", res);
		}
		return 0;
	}
    	return -1;
}


