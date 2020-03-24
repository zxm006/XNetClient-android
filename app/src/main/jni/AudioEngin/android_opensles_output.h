/**
 * Copyright (c) 2015 UUTODO.inc
 * All rights reserved.
 *
 * \file    Android\android_opensles_output.h
 * \brief   Android Audio Play by OpenSLES.
 * \version 1.0
 * \author  frank zhang.
 * \email   sunfrank2012#qq.com
 * \date    2015.07.10
 * 
 *  1. Buffer Cache Size is 3.
 *  
 */
#ifndef __ANDROID_OPENSLES_OUTPUT_H__
#define __ANDROID_OPENSLES_OUTPUT_H__

#include "AudioRender.h"

//#define NumOpenSlBuffers 3
#define NumOpenSlBuffers 10
#include <stdint.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>
#include <SLES/OpenSLES_Android.h>

#include "XListPtr.h"
#include "CritSec.h"
#include "AutoLock.h"
#include "define.h"

class AndroidOpenSLESOutput : public IAudioRender
{
public:
	AndroidOpenSLESOutput();

	virtual ~AndroidOpenSLESOutput();
    
    virtual int  Connect(int nChannels,
        int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0);

    virtual void ReleaseConnections(void);

    virtual	int  Start();

    virtual void Stop();

    virtual int PushPacket(unsigned char*pData,int nLen);

    static void   bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    {
        AndroidOpenSLESOutput *pThis = (AndroidOpenSLESOutput*)context;
        pThis->SL_PlayerCallback(bq);
    }


protected:
    void    SL_PlayerCallback(SLAndroidSimpleBufferQueueItf bq);

    int     OpenAudioDevice();

    int     CloseAudioDevice();

    SLuint32 GetPlayState() const;

private:
    SLObjectItf				sl_gngine_;
    SLPlayItf               sl_streamout_;
    SLObjectItf				sl_player_;
    SLAndroidSimpleBufferQueueItf   sl_bufferqueue_;
    SLObjectItf				sl_mixer_;

    KCritSec                lock_for_queue_;
    uint8_t					*rec_buf_[NumOpenSlBuffers];
    XListPtr			    list_pcm_data_;
    int						active_queue_;

    int                     sample_rate_;
    int                     channels_;
    int                     sample_bits_;
    int                     samplesper_frame_;

	int						mErrTimes;
};

#endif // END __ANDROID_OPENSLES_OUTPUT_H__