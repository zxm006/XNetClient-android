#include "android_opensles_output.h"

#include "Log.h"

#include <stdio.h>
#include <string.h>

#define USE_QUEUE 1
AndroidOpenSLESOutput::AndroidOpenSLESOutput()
{
    sl_gngine_      = NULL;
    sl_streamout_   = NULL;
    sl_player_      = NULL;
    sl_bufferqueue_ = NULL;
    sl_mixer_       = NULL;

    for (int i = 0; i < NumOpenSlBuffers; i++) {
        rec_buf_[i] = NULL;
    }
}

AndroidOpenSLESOutput::~AndroidOpenSLESOutput()
{

}

int AndroidOpenSLESOutput::Connect(int nChannels,
                                   int nSampleRate,
                                   int nSamplesPerFrame,
                                   int nBitrate,
                                   int nParam/*=0*/)
{
    sample_rate_ = nSampleRate;
    channels_ = nChannels;
    sample_bits_ = 16;
    samplesper_frame_ = nSamplesPerFrame;

	LOGI("AndroidOpenSLESOutput Connect in, param:%d,%d,%d,%d", nSampleRate, nChannels, sample_bits_, nSamplesPerFrame);
    SLuint32  nNumSupportedInterfaces = 0;

    SLresult  result = slQueryNumSupportedEngineInterfaces(&nNumSupportedInterfaces);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("[AndroidOpenSLESOutput] slQueryNumSupportedEngineInterfaces Failed.");    
        return -1;
    }

    LOGI("[AndroidOpenSLESOutput]  nNumSupportedInterfaces=%d\n", nNumSupportedInterfaces);

    SLInterfaceID  nInterfaceId;

    for (int i=0; i<nNumSupportedInterfaces; i++) {
        slQuerySupportedEngineInterfaces(i, &nInterfaceId);
    }

    SLboolean        pInterfaceRequired = true;
    result = slCreateEngine((SLObjectItf*)&sl_gngine_, 0, NULL, 1, &SL_IID_OBJECT , &pInterfaceRequired);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("slCreateEngine Failed.");     
        return -1;
    }

    result  = (*sl_gngine_)->Realize(sl_gngine_, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("slEngine Realize Failed.");
        if (sl_gngine_ == NULL) {
            (*sl_gngine_)->Destroy(sl_gngine_);
            sl_gngine_ = NULL;
        }
        return -1;
    }
    
#if USE_QUEUE
    // ......
#else
    for (int i = 0; i < NumOpenSlBuffers; i++) {
        rec_buf_[i] = (uint8_t*)malloc(samplesper_frame_);
    }
#endif

	LOGI("AndroidOpenSLESOutput Connect out");
    return 0;
}

void AndroidOpenSLESOutput::ReleaseConnections(void)
{
    LOGI("[IN ] AndroidOpenSLESOutput::ReleaseConnections ......");
    if (sl_gngine_ == NULL) {
        (*sl_gngine_)->Destroy(sl_gngine_);
        sl_gngine_ = NULL;
    }
    {

#if USE_QUEUE

#else
    for (int i = 0; i < NumOpenSlBuffers; i++) {
        if (rec_buf_[i] != NULL) {
            free(rec_buf_[i]);
            rec_buf_[i] = NULL;
        }
    }
#endif
    }
    LOGI("[OUT] AndroidOpenSLESOutput::ReleaseConnections ......");
}

int AndroidOpenSLESOutput::Start()
{
	LOGI("AndroidOpenSLESOutput::Start in.");
    int nret = OpenAudioDevice();
    if (nret != 0) {
        LOGE("AndroidOpenSLESOutput::Start Failed.");
        return -1;
    }
#if USE_QUEUE

#else
    active_queue_ = 0;
    for (int i = 0; i < NumOpenSlBuffers; i++) {
        (*sl_bufferqueue_)->Enqueue(sl_bufferqueue_, rec_buf_[i],  samplesper_frame_ );
    }
#endif

    mErrTimes = 0;
	LOGI("AndroidOpenSLESOutput::Start out");

    return 0;
}

void AndroidOpenSLESOutput::Stop()
{
    LOGI("AndroidOpenSLESOutput::Stop in");
    int nret = CloseAudioDevice();
	LOGI("AndroidOpenSLESOutput::Stop out");
}

int AndroidOpenSLESOutput::PushPacket(unsigned char*pData,int nLen)
{
    if (NULL == sl_streamout_ || NULL == pData) {
        LOGE("SL Stream Out Is NULL.");
        return -1;
    }

	SLAndroidSimpleBufferQueueState  queueStae;
    (*sl_bufferqueue_)->GetState(sl_bufferqueue_, &queueStae);

	//LOGW("push packeg aueue state:%d,%d", queueStae.count, queueStae.index);
	
#if USE_QUEUE
	KAutoLock auto_lock(lock_for_queue_);
	if (list_pcm_data_.size() > NumOpenSlBuffers) 
	{
		mErrTimes++;
		LOGW("Push Packet So Fast and Lost It.datalen:%d", nLen);
		if(mErrTimes < 30)
		{
			return -10;
		}
		else
		{
			return -11;
		}
	}
	else
	{
		mErrTimes = 0;
	}
	
	PPACKET pPacket = new PACKET(pData,nLen);
	list_pcm_data_.push_back((void*)pPacket);

	//LOGI("push packeg aueue state:%d,%d, input len:%d", queueStae.count, queueStae.index, nLen);
	(*sl_bufferqueue_)->Enqueue(sl_bufferqueue_, pPacket->m_pPacketData,  pPacket->m_nPacketSize );
	
	//(*sl_bufferqueue_)->Enqueue(sl_bufferqueue_, pData,  nLen);
#else
    uint8_t* audio = rec_buf_[active_queue_];
    memcpy(audio, pData, nLen);
#endif

	return 0;
}

int AndroidOpenSLESOutput::OpenAudioDevice()
{
    SLEngineItf   pFactory;
    SLresult  result;

    LOGI("[AndroidOpenSLESOutput] enter openDevice  \n");
    if (sl_gngine_ == NULL) {
        LOGE("SL Engine is NULL.");
        return -1;
    }

    result =  (*sl_gngine_)->GetInterface(sl_gngine_,  SL_IID_ENGINE, (void*)&pFactory);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed GetInterface SL_IID_ENGINE = %d\n", result);
        return -1;
    }

    const SLInterfaceID id0[1]  = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req0[1]     = {SL_BOOLEAN_TRUE};

    result =  (*pFactory)->CreateOutputMix(pFactory, &sl_mixer_, 1, id0, req0);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed  CreateOutputMix = %d\n", result);
        return -1;
    }

    result = (*sl_mixer_)->Realize(sl_mixer_, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed Mixer Realize = %d\n", result);
        if (sl_mixer_ != NULL) {
            (*sl_mixer_)->Destroy(sl_mixer_);
            sl_mixer_ = NULL;
        }
        return -1;
    }


    SLDataLocator_OutputMix loc_dev = {SL_DATALOCATOR_OUTPUTMIX,  sl_mixer_};

    SLDataSink audioSink = {&loc_dev , NULL}; 

    SLDataLocator_AndroidSimpleBufferQueue recBuffQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, NumOpenSlBuffers};

    SLDataFormat_PCM pcm;
    pcm.formatType      = SL_DATAFORMAT_PCM;
    pcm.numChannels     = channels_;
    switch (sample_rate_) {
    case 8000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_8;
        break;
	case 11025:
        pcm.samplesPerSec = SL_SAMPLINGRATE_11_025;
        break;
	case 12000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_12;
        break;
    case 16000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_16;
        break;
    case 22050:
        pcm.samplesPerSec = SL_SAMPLINGRATE_22_05;
        break;
	case 24000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_24;
        break;
    case 32000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_32;
        break;
    case 44100:
        pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
        break;
    case 48000:
        pcm.samplesPerSec = SL_SAMPLINGRATE_48;
        break;
    default:
        LOGE("Unsupported sample rate: %d",sample_rate_);
        pcm.samplesPerSec = sample_rate_*1000;
        break;
    }
    // pcm.samplesPerSec = sample_rate_*1000; //SL_SAMPLINGRATE_44_1;
    pcm.bitsPerSample   = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize   = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.channelMask     = (channels_ == 1 ) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);
    pcm.endianness      = SL_BYTEORDER_LITTLEENDIAN;

	LOGI("output pcm param: %d,%d,%d, %d,%d,%d, %d", pcm.formatType, pcm.numChannels, pcm.samplesPerSec,
			pcm.bitsPerSample, pcm.containerSize, pcm.channelMask, pcm.endianness);

    SLDataSource audioSrc = {&recBuffQueue,  &pcm}; 


    const SLInterfaceID id1[1]  = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req1[1]     = {SL_BOOLEAN_TRUE};
    result = (*pFactory)->CreateAudioPlayer(pFactory, &sl_player_, &audioSrc,
        &audioSink, 1, id1, req1);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed  CreateAudioPlayer = %d, channels_:%d, sample_rate_=%d\n", result, channels_, sample_rate_);
        goto OPEN_DEV_ERR;
    }

    result  = (*sl_player_)->Realize(sl_player_, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed Player Realize = %d \n", result );
        goto OPEN_DEV_ERR;
    }

    result =  (*sl_player_)->GetInterface(sl_player_, SL_IID_PLAY,(void*)&sl_streamout_);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed  GetInterface SL_IID_RECORD result=%d\n", result);
        goto OPEN_DEV_ERR;
    }

    result =  (*sl_player_)->GetInterface(sl_player_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,(void*)&sl_bufferqueue_);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed  GetInterface SL_IID_ANDROIDSIMPLEBUFFERQUEUE\n" );
        goto OPEN_DEV_ERR;
    }

    result = (*sl_bufferqueue_)->RegisterCallback(sl_bufferqueue_, bqPlayerCallback, this);
    if (result != SL_RESULT_SUCCESS)  {
        LOGE("[AndroidOpenSLESOutput] Failed RegisterCallback." );
        goto OPEN_DEV_ERR;
    }

    result = (*sl_streamout_)->SetPlayState(sl_streamout_, SL_PLAYSTATE_STOPPED);

    result = (*sl_bufferqueue_)->Clear(sl_bufferqueue_);

    SLAndroidSimpleBufferQueueState  queueStae;
    (*sl_bufferqueue_)->GetState(sl_bufferqueue_, &queueStae);	

    LOGI("[AndroidOpenSLESOutput] RecorderCallback count=%d, index=%d\n", queueStae.count,  queueStae.index );   
    // start Play
    result = (*sl_streamout_)->SetPlayState(sl_streamout_, SL_PLAYSTATE_PLAYING);

    LOGI("[AndroidOpenSLESOutput]  leave openDevice  \n");
    return 0;
OPEN_DEV_ERR:
    CloseAudioDevice();
    return 0;
}
/**
* 
* Close Order Must Be:
* 1. SLPlayItf: Real Player.
* 2. SLAndroidSimpleBufferQueueItf: Buffer Queue.
* 3. SLObjectItf: Player Obj.
* 4. SLObjectItf: Mixer Obj;
* by frank zhang @ 2015.07.12.
*/
int AndroidOpenSLESOutput::CloseAudioDevice()
{
    LOGD("AndroidOpenSLESOutput::CloseAudioDevice in");
    SLresult  result;
    if (sl_streamout_ != NULL) {
        LOGD("Stop SL Stream Out .....");
        result = (*sl_streamout_)->SetPlayState(sl_streamout_, SL_PLAYSTATE_STOPPED);
        sl_streamout_ = NULL;
    }

    if (sl_bufferqueue_ != NULL) {
        LOGD("Stop SL Clear Buffer Out .....");
        result = (*sl_bufferqueue_)->Clear(sl_bufferqueue_);
        sl_bufferqueue_ = NULL;
    }
   
    if(sl_player_ != NULL) {
        LOGD("Stop SL Destory Player Obj .....");
        (*sl_player_)->Destroy(sl_player_);
        sl_player_	= NULL;	
    }

    if (sl_mixer_ != NULL) {
        LOGD("Stop SL Destory Mixer Obj .....");
        (*sl_mixer_)->Destroy(sl_mixer_);
        sl_mixer_ = NULL;
    }
    LOGD("AndroidOpenSLESOutput::CloseAudioDevice out");
    return 0;
}

SLuint32 AndroidOpenSLESOutput::GetPlayState() const {
    SLuint32 state;
    if (sl_streamout_ != NULL) {
        SLresult err = (*sl_streamout_)->GetPlayState(sl_streamout_, &state);
        if (SL_RESULT_SUCCESS != err) {
            LOGE("GetPlayState failed: %d", err);
        }
    }
    return state;
}

void AndroidOpenSLESOutput::SL_PlayerCallback(SLAndroidSimpleBufferQueueItf bq)
{
    if (GetPlayState() != SL_PLAYSTATE_PLAYING) {
        LOGW("Buffer callback in non-playing state!");
        return;
    }
    // ......
    SLAndroidSimpleBufferQueueState  queueStae;
    (*bq)->GetState(bq, &queueStae);

	//LOGI("SL_PlayerCallback call back aueue state:%d,%d", queueStae.count, queueStae.index);
	
#if USE_QUEUE
	KAutoLock auto_lock(lock_for_queue_);

	PPACKET *pPacket=(PPACKET*)list_pcm_data_.front();
	list_pcm_data_.pop_front();
	delete pPacket;
	pPacket = NULL;
#else
    uint8_t* audio = rec_buf_[active_queue_];

    (*sl_bufferqueue_)->Enqueue(sl_bufferqueue_, rec_buf_[active_queue_],  samplesper_frame_ );

    active_queue_ = (active_queue_ + 1) % NumOpenSlBuffers;
#endif
    //LOGD("SL_PlayerCallback ......");
}
