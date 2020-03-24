#include "util_audio_jni_helper.h"

#include "log.h"

extern JavaVM*           g_androidJVM;

JNIEnv *JniUtilHelperAudio::getEnv() {
	JNIEnv *pEnv;
	g_androidJVM->GetEnv((void**)&pEnv, JNI_VERSION_1_6);
	return pEnv;
}

CAutoGetEnvAudio::CAutoGetEnvAudio(JNIEnv** env)
{
	if (g_androidJVM != 0)
	{
		isAttached = false;
		if (g_androidJVM->GetEnv((void**) env, JNI_VERSION_1_6) != JNI_OK) {
			jint res = g_androidJVM->AttachCurrentThread(env, 0);

			// Get the JNI env for this thread
			if ((res < 0) || !*env) {
				LOGE("%s: Could not attach thread to JVM (%d, %p)",	__FUNCTION__, res, *env);
				//return -1;
			}
			isAttached = true;
		}
	}
}

CAutoGetEnvAudio::~CAutoGetEnvAudio()
{
	// Detach this thread if it was attached
	if (g_androidJVM != 0)
	{
		if (isAttached) {
			if (g_androidJVM->DetachCurrentThread() < 0) {
				LOGD("%s: Could not detach thread from JVM", __FUNCTION__);
			}
		}
	}
}