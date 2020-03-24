#ifndef __UTIL_AUDIO_JNI_HELPER_H__
#define __UTIL_AUDIO_JNI_HELPER_H__

#include <jni.h>

class JniUtilHelperAudio
{
public:
	static JNIEnv *getEnv();
};

class CAutoGetEnvAudio
{
public:
	CAutoGetEnvAudio(JNIEnv** env);
	~CAutoGetEnvAudio();
private:
	bool isAttached;
};

#endif