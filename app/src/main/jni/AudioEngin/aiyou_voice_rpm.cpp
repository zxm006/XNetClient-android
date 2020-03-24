#include "webrtc/voice_engine/include/aiyou_voice_rpm.h"

#define enableTrace 1

//AiyouVoiceRpm::mpInstance = NULL;

AiyouVoiceRpm *AiyouVoiceRpm::mpInstance = NULL;
bool AiyouVoiceRpm::mIsInitedVm = false;


AiyouVoiceRpm* AiyouVoiceRpm::getInstance()
{
	if(mpInstance == NULL)
	{
		mpInstance = new AiyouVoiceRpm();
	}
	
       LOGW("aiyou voice instance:%x", mpInstance);
	return mpInstance;
}

int32_t AiyouVoiceRpm::SetAndroidObjects(void* javaVM, void* env, void* context)
{
	if(!mIsInitedVm)
	{
		LOGW("this is will set android javavm");
		webrtc::VoiceEngine::SetAndroidObjects(javaVM, env, context);
		mIsInitedVm = true;
	}
	else
	{
		LOGW("this is not need set android javavm, already seted");
	}/**/
	return 0;
}

int32_t AiyouVoiceRpm::Init()
{
	LOGW("Aiyou VoiceRpm init in");
	if(mIsInited && mVoiceEngin)
	{
		LOGW("there already inited, not need reinit");
		return 0;
	}

	int getOK = 0;

	mVoiceEngin = webrtc::VoiceEngine::Create();
	mVoeBase = webrtc::VoEBase::GetInterface(mVoiceEngin);
	if (!mVoeBase)
	{
		LOGW("get voebase failed");
		getOK = -1;
	}

	// AudioProcessing module
	mVoeAp = webrtc::VoEAudioProcessing::GetInterface(mVoiceEngin);
	if (!mVoeBase)
	{
		LOGW("get voeap failed");
		getOK = -1;
	}

	// AudioProcessing module
	mVoeVolume = webrtc::VoEVolumeControl::GetInterface(mVoiceEngin);
	if (!mVoeVolume)
	{
		LOGW("get voevolume failed");
		getOK = -1;
	}

	if(getOK != 0)
	{
		LOGW("get sub apis failed");
		return -1;
	}

	if (enableTrace)
	{
		if (0 != webrtc::VoiceEngine::SetTraceFile("/sdcard/audiotrace.txt"))
		{
			LOGW("Could not enable trace");
		}
		if (0 != webrtc::VoiceEngine::SetTraceFilter(1))
		{
			LOGW("Could not set trace filter");
		}
	}

	mIsInited = true;
       getOK = mVoeBase->Init();
	LOGI("base init ret is:%d", getOK);
	
	return getOK;
}

int32_t AiyouVoiceRpm::StartPlayout(int sampleRate)
{
	return mVoeBase->StartPlayoutPcm(sampleRate);
}

int32_t AiyouVoiceRpm::StopPlayout(int chanid)
{
	mVoeBase->StopPlayoutPcm(chanid);
	return 0;
}

int32_t AiyouVoiceRpm::StartCapture(webrtc::CAyVoiceCapCb *pCapDataCb)
{
	return mVoeBase->StartCapturePcm(pCapDataCb);
}

int32_t AiyouVoiceRpm::StopCapture(int id)
{
	mVoeBase->StopCapturePcm(id);
	return 0;
}

int32_t AiyouVoiceRpm::OnPcmDataIn(void *pData, int len, int chanid, long timestamp)
{
	mVoeBase->OnPcmDataIn(pData, len, chanid, timestamp);
	return 0;
}


AiyouVoiceRpm::AiyouVoiceRpm()
{
	mVoiceEngin = NULL;
	mVoeBase = NULL;
	mVoeAp = NULL;
	mVoeVolume = NULL;
	mIsInited = false;
}

AiyouVoiceRpm::~AiyouVoiceRpm()
{

}


