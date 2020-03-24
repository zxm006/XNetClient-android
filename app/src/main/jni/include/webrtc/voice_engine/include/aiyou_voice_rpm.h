/*
 *  Copyright (c) 2016 The gamecloud project authors. All Rights Reserved.
 *
 */

#ifndef AIYOU_VOICE_RECORD_PLAYOUT_MODULE_H
#define AIYOU_VOICE_RECORD_PLAYOUT_MODULE_H

#include "webrtc/voice_engine/include/voe_audio_processing.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/voice_engine/include/voe_hardware.h"
#include "webrtc/voice_engine/include/voe_volume_control.h"
//#include "webrtc/voice_engine/include/aiyou_voice_capplaycb.h"

#ifndef LOG_TAG

#define LOG_TAG "AyVoiceRpm"
#include<android/log.h>

#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__) // 定义LOGD类型
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__) // 定义LOGI类型
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__) // 定义LOGW类型
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__) // 定义LOGE类型
#define LOGF(...)  __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__) // 定义LOGF类型
#endif

class AiyouVoiceRpm
{
private:
	static AiyouVoiceRpm *mpInstance;
	AiyouVoiceRpm();
	static bool mIsInitedVm;
	bool mIsInited;
public:
	static AiyouVoiceRpm* getInstance();
	static int32_t SetAndroidObjects(void* javaVM, void* env, void* context);
	int32_t Init();
	int32_t StartPlayout(int sampleRate);
	int32_t StopPlayout(int chanid);
	int32_t StartCapture(webrtc::CAyVoiceCapCb *pCapDataCb);
	int32_t StopCapture(int id);
	int32_t OnPcmDataIn(void *pData, int len, int chanid, long timestamp);	

public:
	virtual ~AiyouVoiceRpm();

private:
	
    // VoiceEngine
    webrtc::VoiceEngine* mVoiceEngin;
    // Sub-APIs
    webrtc::VoEBase* mVoeBase;
    webrtc::VoEAudioProcessing* mVoeAp;
    webrtc::VoEVolumeControl* mVoeVolume;
};

#endif 
