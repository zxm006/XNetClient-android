#ifndef _AUDIO_TRACKER_H__
#define _AUDIO_TRACKER_H__

#include "Define.h"
#include "AudioRender.h"

#include <jni.h>


class CAudioTracker
	  : public IAudioRender
{
public:
	CAudioTracker();
	virtual ~CAudioTracker();

public:
	virtual int  Connect(int nChannels, int nSampleRate,int nSamplesPerFrame,int nBitrate,int nParam=0);
	virtual void ReleaseConnections(void);
	virtual int  Start();
	virtual void Stop();
	virtual int PushPacket(unsigned char*pData,int nLen);

protected:
	int m_nSampleRate;	/**< 采样率.*/
	int m_nSampleBits;	/**< 采样大小.*/
	int m_nSamplesPerFrame; 
	int m_nChannels;	/**< 声道.*/

private:
	jobject  m_pJavaClassObj;
	jobject mJavaPlayBuffer;
	void* mpJavaDirectPlayBuffer; // Direct buffer pointer to play buffer
	jmethodID mJavaMethPlay; // Method ID of play in AudioDeviceAndroid
};

//#endif //if Android


#endif
