#include "android_opensles_input.h"
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>


#include <time.h>

char arAudioFileName[256];
void saveData(char *pData, int len)
{
	FILE *pFile = NULL;
	pFile = fopen(arAudioFileName, "a");
	if(pFile)
	{
		fwrite(pData , 1, len, pFile );    
		fclose(pFile); 
		pFile = NULL;
	}
}

static bool m_bIsRecording = false;

AndroidOpenSLESInput::AndroidOpenSLESInput(IAudioCaptureNotify& rIAudioCaptureNotify) : m_IAudioCaptureNotify(rIAudioCaptureNotify)
{
	sl_engine_		= NULL;
	sl_streamin_	= NULL;
	sl_recorder_	= NULL;
	sl_recoder_sbq_	= NULL;

    active_queue_ = 0;
	for (int i = 0; i < NumOpenSlBuffers; i++) {
		rec_buf_[i] = NULL;
	}

	m_bIsRecording = false;

	time_t now_time;
	now_time = time(NULL);
	memset(arAudioFileName, 0, sizeof(arAudioFileName));
	sprintf(arAudioFileName, "/mnt/sdcard/%ld.pcm", now_time);
}

AndroidOpenSLESInput::~AndroidOpenSLESInput()
{
	// ......
	ReleaseConnections();
}

int AndroidOpenSLESInput::Connect(int nChannel, 
								  int nSampleRate,
								  int nSamplesPerFrame,
								  int nBitrate,
								  int nParam /* = 0 */)
{
	channels_ = nChannel;
	samplerate_ = nSampleRate;
	samplebits_ = 16; // Test 
	samplesper_frame_ = nSamplesPerFrame;

	SLuint32  nNumSupportedInterfaces = 0;
	SLresult  result = slQueryNumSupportedEngineInterfaces(&nNumSupportedInterfaces);
	if (SL_RESULT_SUCCESS != result) {
		LOGE("OpenSles Error:%d",result);
		return -1;
	}
	LOGI("[AndroidOpenSLESInput] OpenSles nNumSupportedInterfaces=%d, nSamplesPerFrame:%d", nNumSupportedInterfaces, nSamplesPerFrame);

	SLInterfaceID  nInterfaceId;

	for (int i = 0; i < nNumSupportedInterfaces; i++) {
		slQuerySupportedEngineInterfaces(i, &nInterfaceId);
	}

	SLboolean        pInterfaceRequired = true;
	result = slCreateEngine((SLObjectItf*)&sl_engine_, 0, NULL, 1, &SL_IID_OBJECT , &pInterfaceRequired);
	if (SL_RESULT_SUCCESS != result) {
		LOGE("OpenSles Error:%d",result);
		return -1;
	}

	result  = (*sl_engine_)->Realize(sl_engine_, SL_BOOLEAN_FALSE);

	if (SL_RESULT_SUCCESS != result) {
		LOGE("OpenSles Error:%d",result);
		return -1;
	}

	// ALLOC Buffer
	active_queue_	 = 0;
	number_overruns_ = 0;

	for (int i = 0; i < NumOpenSlBuffers; i++) {
		rec_buf_[i] = (uint8_t*)malloc(samplesper_frame_);
	}

	return 0;
}

void AndroidOpenSLESInput::ReleaseConnections(void)
{
	LOGD("AndroidOpenSLESInput::ReleaseConnections");
	Stop();

	if (sl_engine_) {
		(*sl_engine_)->Destroy(sl_engine_);
		sl_engine_ = NULL;
	}

	for (int i = 0; i < NumOpenSlBuffers; i++) {
		free(rec_buf_[i]);
		rec_buf_[i] = NULL;
	}
}

int AndroidOpenSLESInput::Start()
{
	int nret = OpenAudioDevice();
	if (nret != 0) {
		LOGE("AndroidOpenSLESInput::Start Failed.");
		return -1;
	}

	for (int i = 0; i < NumOpenSlBuffers; i++) {
		(*sl_recoder_sbq_)->Enqueue(sl_recoder_sbq_, rec_buf_[i],  samplesper_frame_ );
	}
	// ......

	return 0;
}

void AndroidOpenSLESInput::Stop()
{
	int nret = CloseAudioDevice();
}

void AndroidOpenSLESInput::SetSampleRate(int nRate)
{

}

void AndroidOpenSLESInput::SetSamplesPerFrame(int nSamplesPerFrame)
{

}

void AndroidOpenSLESInput::SetBitrate(int nBitrate)
{

}

void AndroidOpenSLESInput::bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
	if(!m_bIsRecording)
	{
		return;
	}
	
	AndroidOpenSLESInput *pThis = (AndroidOpenSLESInput*)context;
	pThis->SL_RecorderCallback(bq);
}

void AndroidOpenSLESInput::SL_RecorderCallback(SLAndroidSimpleBufferQueueItf bq)
{
	// ......
	if(!sl_recoder_sbq_ || !m_bIsRecording)
	{
		return;
	}
	
	KAutoLock l(m_kCritSec);
	
	SLAndroidSimpleBufferQueueState  queueStae;
	(*bq)->GetState(bq, &queueStae);

	//LOGI("record cb, count:%d, index:%d, activite:%d", queueStae.count,  queueStae.index, active_queue_);
	
	uint8_t* audio = rec_buf_[active_queue_];

	m_IAudioCaptureNotify.OnAudioCaptureNotifyOutput(audio,samplesper_frame_);

	if(!sl_recoder_sbq_ || !m_bIsRecording)
	{
		return;
	}
	(*sl_recoder_sbq_)->Enqueue(sl_recoder_sbq_, rec_buf_[active_queue_],  samplesper_frame_ );

	active_queue_ = (active_queue_ + 1) % NumOpenSlBuffers;

	//saveData((char*)audio, samplesper_frame_);
}

int AndroidOpenSLESInput::OpenAudioDevice()
{
	SLEngineItf   pFactory;
	SLresult	  result;

	result =  (*sl_engine_)->GetInterface(sl_engine_,  SL_IID_ENGINE, (void*)&pFactory);
	if (result != SL_RESULT_SUCCESS)  {
		LOGE("[AndroidOpenSLESInput] failed   GetInterface SL_IID_ENGINE = %d\n", result);
		return -1;
	}

	SLDataLocator_IODevice loc_dev = {SL_DATALOCATOR_IODEVICE, SL_IODEVICE_AUDIOINPUT,
		SL_DEFAULTDEVICEID_AUDIOINPUT, NULL};

	SLDataSource audioSrc = {&loc_dev, NULL};

	SLDataLocator_AndroidSimpleBufferQueue recBuffQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 5};

	SLDataFormat_PCM pcm;
	pcm.formatType		= SL_DATAFORMAT_PCM;
	pcm.numChannels		= channels_;				   
	pcm.samplesPerSec	= samplerate_*1000; //SL_SAMPLINGRATE_44_1;
	pcm.bitsPerSample	= SL_PCMSAMPLEFORMAT_FIXED_16;
	pcm.containerSize	= SL_PCMSAMPLEFORMAT_FIXED_16;
	pcm.channelMask		= (channels_ == 1) ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);
	pcm.endianness		= SL_BYTEORDER_LITTLEENDIAN;
	SLDataSink recSink	= {&recBuffQueue, &pcm};

    LOGD("[AndroidOpenSLESInput] channels_:%d samplerate_:%d", channels_, samplerate_);

	LOGI("input pcm param: %d,%d,%d, %d,%d,%d, %d", pcm.formatType, pcm.numChannels, pcm.samplesPerSec,
			pcm.bitsPerSample, pcm.containerSize, pcm.channelMask, pcm.endianness);

	const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req[1]	  = {SL_BOOLEAN_TRUE};
	SLAndroidSimpleBufferQueueState  queueStae;

	result = (*pFactory)->CreateAudioRecorder(pFactory, &sl_recorder_, &audioSrc,
		&recSink, 1, id, req);

	if (result != SL_RESULT_SUCCESS)  {
		LOGE("[AndroidOpenSLESInput] CreateAudioRecorder=%d, channels=%d, samplerate=%d\n", result, channels_, samplerate_);
		goto OPEN_DEV_ERR;
		return -1;
	}

	result  = (*sl_recorder_)->Realize(sl_recorder_, SL_BOOLEAN_FALSE);
	if (result != SL_RESULT_SUCCESS)  {
		LOGE("[AndroidOpenSLESInput] failed  Realize = %d \n", result );
		goto OPEN_DEV_ERR;
		return -1;
	}

	result =  (*sl_recorder_)->GetInterface(sl_recorder_, SL_IID_RECORD,(void*)&sl_streamin_);
	if (result != SL_RESULT_SUCCESS)  {
		LOGE("[AndroidOpenSLESInput] failed  GetInterface SL_IID_RECORD result=%d\n", result);
		goto OPEN_DEV_ERR;
		return -1;
	}

	result =  (*sl_recorder_)->GetInterface(sl_recorder_, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,(void*)&sl_recoder_sbq_);
	if (result != SL_RESULT_SUCCESS)  {
		LOGI("[AndroidOpenSLESInput] failed  GetInterface SL_IID_ANDROIDSIMPLEBUFFERQUEUE\n" );
		goto OPEN_DEV_ERR;
		return -1;
	}

	result = (*sl_recoder_sbq_)->RegisterCallback(sl_recoder_sbq_, bqRecorderCallback,
		this);
	if (result != SL_RESULT_SUCCESS)  {
		goto OPEN_DEV_ERR;
	}

	result = (*sl_streamin_)->SetRecordState(sl_streamin_, SL_RECORDSTATE_STOPPED);
	if (result != SL_RESULT_SUCCESS)  {
		goto OPEN_DEV_ERR;
	}

	result = (*sl_recoder_sbq_)->Clear(sl_recoder_sbq_);
	if (result != SL_RESULT_SUCCESS)  {
		goto OPEN_DEV_ERR;
	}

	(*sl_recoder_sbq_)->GetState(sl_recoder_sbq_, &queueStae);

	LOGI("[AndroidOpenSLESInput] RecorderCallback count=%d, index=%d\n", queueStae.count,  queueStae.index );
	
	result = (*sl_streamin_)->SetRecordState(sl_streamin_, SL_RECORDSTATE_RECORDING);

	m_bIsRecording = true;
	return 0;

OPEN_DEV_ERR:
	
	if(sl_streamin_ != NULL) {
		result = (*sl_streamin_)->SetRecordState(sl_streamin_, SL_RECORDSTATE_STOPPED);
	}

	if (sl_recoder_sbq_ != NULL) {
		result = (*sl_recoder_sbq_)->Clear(sl_recoder_sbq_);
	}

	sl_streamin_    = NULL;
	sl_recoder_sbq_ = NULL;

	if(sl_recorder_ != NULL) {
		(*sl_recorder_)->Destroy(sl_recorder_);
		sl_recorder_	= NULL;	
	}
	return -1;
}

int AndroidOpenSLESInput::CloseAudioDevice()
{
	LOGI("AndroidOpenSLESInput::CloseAudioDevice");
	m_bIsRecording = false;

	KAutoLock l(m_kCritSec);
	
	SLresult	  result;
	if(sl_streamin_ != NULL) {
		result = (*sl_streamin_)->SetRecordState(sl_streamin_, SL_RECORDSTATE_STOPPED);
	}
	
	LOGI("CloseAudioDevice 1");

	if (sl_recoder_sbq_ != NULL) {
		result = (*sl_recoder_sbq_)->Clear(sl_recoder_sbq_);
	}
	LOGI("CloseAudioDevice 2");

	sl_streamin_    = NULL;
	sl_recoder_sbq_ = NULL;

	if(sl_recorder_ != NULL) {
		(*sl_recorder_)->Destroy(sl_recorder_);
		sl_recorder_	= NULL;	
	}
	LOGI("CloseAudioDevice 3");
	return 0;
}


