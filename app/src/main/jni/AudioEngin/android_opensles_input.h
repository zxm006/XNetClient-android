/**
 * Copyright (c) 2015 UUTODO.inc
 * All rights reserved.
 *
 * \file    Android\android_opensles_input.h
 * \brief   Android Audio Capture by OpenSLES.
 * \version 1.0
 * \author  frank zhang.
 * \email   sunfrank2012#qq.com
 * \date    2015.07.10
 */
#ifndef __ANDROID_OPENSLES_INPUT_H__
#define __ANDROID_OPENSLES_INPUT_H__

#include "AudioCapture.h"


#define NumOpenSlBuffers 3

#include <stdint.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES_Android.h>

#include "CritSec.h"
#include "AutoLock.h"

class AndroidOpenSLESInput : public IAudioCapture
{
public:
	AndroidOpenSLESInput(IAudioCaptureNotify& rIAudioCaptureNotify);

	virtual ~AndroidOpenSLESInput();

	virtual int  Connect(int nChannel, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0);

	virtual void ReleaseConnections(void);

	virtual	int  Start();

	virtual void Stop();

	virtual	void SetSampleRate(int nRate);
	
	virtual void SetSamplesPerFrame(int nSamplesPerFrame);
	
	virtual	void SetBitrate(int nBitrate);

	static void   bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

protected:
	void    SL_RecorderCallback(SLAndroidSimpleBufferQueueItf bq);

	int     OpenAudioDevice();

	int     CloseAudioDevice();

private:
	IAudioCaptureNotify& m_IAudioCaptureNotify;

	// 
	SLObjectItf				sl_engine_;
	SLRecordItf				sl_streamin_;
	SLObjectItf				sl_recorder_;
	SLAndroidSimpleBufferQueueItf   sl_recoder_sbq_;
	//
	int						channels_;
	int						samplerate_;
	int						samplebits_;
	int						samplesper_frame_;

	int						active_queue_;

	int						number_overruns_;

	uint8_t					*rec_buf_[NumOpenSlBuffers];

	KCritSec				m_kCritSec;
};

#endif // END __ANDROID_OPENSLES_INPUT_H__